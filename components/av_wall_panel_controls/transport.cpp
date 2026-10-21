#include "av_wall_panel_controls.h"
#include "panel_command.h"
#include "esphome/core/hal.h"
#include <algorithm>
#include <cstring>
#include <string>

namespace esphome::av_wall_panel_controls {
namespace transport {

using tessera::Font;
using tessera::Tone;
namespace ui = tessera::ui;

// The keys in the order they always stand in, by their option in the manifest. Their icons are Material Design Icons
// of Tessera's set. Eject is not in it yet, so its key says the word instead, on an ordinary label; when the set has
// mdi-eject (0xF01EA), its glyph goes here and the word goes.
struct KeyKind {
  const char *option;
  uint32_t glyph;   // 0: the word (eject_word)
};
static constexpr KeyKind KINDS[] = {
    {"previous", 0xF04AE},    // skip-previous
    {"stop", 0xF04DB},        // stop
    {"play", 0xF040A},        // play
    {"pause", 0xF03E4},       // pause
    {"play_pause", 0xF040E},  // play-pause
    {"next", 0xF04AD},        // skip-next
    {"eject", 0},             // eject
};
static constexpr int KIND_COUNT = sizeof(KINDS) / sizeof(KINDS[0]);
enum Kind { PREVIOUS, STOP, PLAY, PAUSE, PLAY_PAUSE, NEXT, EJECT };

// How long a key stays lit after a tap: at least this long, and until the next tick after it.
static constexpr uint32_t FLASH_MS = 300;

// The tile: the keys that have an action, side by side in the middle of the card.
class KeysTile : public tessera::Tile {
 public:
  explicit KeysTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    const char *entity = c.entity ? c.entity : "";
    detail::copy(entity_, sizeof entity_, entity, std::strlen(entity));
    player_ = std::strncmp(entity_, "media_player.", 13) == 0;

    for (int k = 0; k < KIND_COUNT; ++k) {
      Key &key = keys_[count_];
      const Parsed parsed = parse(c.options[KINDS[k].option] | "", key.command);
      if (parsed == Parsed::EMPTY) continue;
      key.kind = static_cast<Kind>(k);
      key.valid = parsed == Parsed::OK;
      ++count_;
    }

    if (count_ == 0) {
      // Nothing to press yet: say what to do, in the middle.
      hint_ = ui::label(c.parent, Font::BODY, theme::MUTED);
      lv_obj_set_width(hint_, c.width);
      lv_obj_set_style_text_align(hint_, LV_TEXT_ALIGN_CENTER, 0);
      ui::set_text(hint_, plugin_->text("no_keys"));
      lv_obj_set_pos(hint_, 0, (c.height - ui::line_height(Font::BODY)) / 2);
      return;
    }

    // The keys in one row where each is at least a finger wide (7 mm), else in two or three rows, the last row holding
    // what is left; where nothing reaches 7 mm, the rows that give the largest keys. A key is never wider than half again
    // its height, nor higher than 12 mm: a finger's key on a small screen, not a slab on a large one. The gap narrows
    // before the keys do.
    int best = -1;
    for (int r = 1; r <= 3 && r <= count_; ++r) {
      const int per = (count_ + r - 1) / r;
      int gap = ui::px(8);
      int slot = (c.width - gap * (per - 1)) / per;
      int high = (c.height - gap * (r - 1)) / r;
      if ((slot < ui::mm(9) || high < ui::mm(9)) && (per > 1 || r > 1)) {
        gap = ui::px(3);
        slot = (c.width - gap * (per - 1)) / per;
        high = (c.height - gap * (r - 1)) / r;
      }
      const int kh = std::max(1, std::min(high, ui::mm(12)));
      const int kw = std::max(1, std::min(slot, kh * 3 / 2));
      const int size = std::min(kw, kh);
      const bool fits = size >= ui::mm(7);
      if (best < 0 || fits || size > best) {
        best = size;
        rows_ = r;
        per_ = per;
        gap_ = gap;
        kw_ = kw;
        kh_ = kh;
      }
      if (fits) break;
    }
    top_ = (c.height - (rows_ * kh_ + (rows_ - 1) * gap_)) / 2;
    for (int r = 0; r < rows_; ++r) {
      const int in_row = std::min(per_, count_ - r * per_);
      row_left_[r] = (c.width - (in_row * kw_ + (in_row - 1) * gap_)) / 2;
    }
    const int kh = kh_;
    const int radius = std::min({ui::px(ui::large() ? 12 : 8), kw_ / 2, kh / 2});

    // The largest icon that leaves the key a margin.
    const int room = std::min(kw_, kh) * 3 / 4;
    const Font face = ui::text_width(ui::icon(0xF040A), Font::ICON) <= room ? Font::ICON : Font::ICON_SMALL;
    const int glyph = std::min(ui::text_width(ui::icon(0xF040A), face), std::min(kw_, kh));
    const int line = ui::line_height(face);

    for (int i = 0; i < count_; ++i) {
      Key &key = keys_[i];
      key.block = ui::block(c.parent, theme::BUTTON);
      lv_obj_set_size(key.block, kw_, kh);
      lv_obj_set_pos(key.block, row_left_[i / per_] + (i % per_) * (kw_ + gap_), top_ + (i / per_) * (kh_ + gap_));
      lv_obj_set_style_radius(key.block, radius, 0);
      if (KINDS[key.kind].glyph) {
        key.icon = ui::label(key.block, face, theme::INK);
        lv_obj_set_width(key.icon, kw_);
        lv_obj_set_style_text_align(key.icon, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(key.icon, 0, (kh - line) / 2);
        ui::set_text(key.icon, ui::icon(KINDS[key.kind].glyph));
      } else {
        // The word, in the larger face where it fits the key and the smaller where it does not; cut on a tiny key.
        const std::string word = plugin_->text("eject_word");
        const int pad = ui::px(4);
        // A margin of its own: the measured width of a word is a few pixels short of what the label draws.
        const int room = kw_ - 2 * pad - ui::px(6);
        const Font words = ui::text_width(word, Font::BODY_LARGE) <= room ? Font::BODY_LARGE : Font::BODY;
        // On a key too narrow for the word with a margin, it takes the key's whole width.
        const int side = words == Font::BODY && ui::text_width(word, Font::BODY) > room ? 0 : pad;
        key.icon = ui::label(key.block, words, theme::INK);
        lv_obj_set_width(key.icon, kw_ - 2 * side);
        lv_label_set_long_mode(key.icon, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(key.icon, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(key.icon, side, (kh - ui::line_height(words)) / 2);
        ui::set_text(key.icon, word);
      }
    }
    paint();
  }

  // The entity's state: a media player lights the key of what it does now.
  void on_state(JsonObjectConst data) override {
    const char *state = data["state"] | "";
    if (std::strcmp(state, state_) == 0) return;
    detail::copy(state_, sizeof state_, state, std::strlen(state));
    paint();
  }

  void on_tick(uint32_t) override {
    if (flash_ >= 0 && esphome::millis() - flash_at_ >= FLASH_MS) {
      flash_ = -1;
      paint();
    }
  }

  void on_theme() override {
    if (hint_) ui::set_color(hint_, theme::MUTED);
    paint();
  }

  void on_tap() override {
    const int i = key_at(tessera::tap_point());
    if (i < 0) return;
    const Key &key = keys_[i];
    const char *target = key.command.target[0] ? key.command.target : entity_;
    bool sent = false;
    if (key.valid && target[0])
      sent = tessera::action(key.command.action, target, key.command.key[0] ? key.command.key : nullptr,
                             key.command.value);
    flash_ = i;
    flash_ok_ = sent;
    flash_at_ = esphome::millis();
    paint();
  }

 private:
  struct Key {
    Kind kind = PREVIOUS;
    bool valid = false;
    Command command;
    lv_obj_t *block = nullptr, *icon = nullptr;
  };

  // The key under the finger: its own size and half the gap around it, so a tap between two keys still counts. A tap
  // above or below the rows counts for the nearest row.
  int key_at(tessera::Point p) const {
    if (count_ == 0) return -1;
    if (p.x < 0 || p.y < 0) return count_ == 1 ? 0 : -1;
    const int row = std::max(0, std::min(rows_ - 1, (p.y - top_ + gap_ / 2) / (kh_ + gap_)));
    const int x = p.x - row_left_[row] + gap_ / 2;
    if (x < 0) return -1;
    const int col = x / (kw_ + gap_);
    const int i = row * per_ + col;
    return col < per_ && i < count_ ? i : -1;
  }

  // Whether the player's state is what this key does: playing lights play and play/pause, paused lights pause.
  bool lit(Kind kind) const {
    if (!player_) return false;
    if (std::strcmp(state_, "playing") == 0) return kind == PLAY || kind == PLAY_PAUSE;
    if (std::strcmp(state_, "paused") == 0) return kind == PAUSE;
    return false;
  }

  void paint() {
    const bool away = std::strcmp(state_, "unavailable") == 0;
    for (int i = 0; i < count_; ++i) {
      Key &key = keys_[i];
      // A tap: the pressed blue while it goes out, red when it could not be sent.
      if (i == flash_) {
        if (flash_ok_) ui::set_fill(key.block, Tone::NORMAL, theme::ACCENT_PRESSED);
        else ui::set_fill(key.block, Tone::ALERT, theme::BUTTON);
        ui::set_color(key.icon, theme::ON_ACCENT);
        continue;
      }
      // What the player does now: a blue key, as a chosen segment is.
      if (key.valid && !away && lit(key.kind)) {
        ui::set_fill(key.block, Tone::NORMAL, theme::ACCENT);
        ui::set_color(key.icon, theme::ON_ACCENT);
        continue;
      }
      ui::set_fill(key.block, Tone::NORMAL, theme::BUTTON);
      if (!key.valid) ui::set_tone(key.icon, Tone::ALERT, theme::INK);
      else if (away) ui::set_color(key.icon, theme::SUBTLE);
      else ui::set_color(key.icon, theme::INK);
    }
  }

  const tessera::Plugin *plugin_;
  Key keys_[KIND_COUNT];
  int count_ = 0;
  int rows_ = 1, per_ = 1, top_ = 0, kw_ = 1, kh_ = 1, gap_ = 0;
  int row_left_[3] = {0, 0, 0};
  char entity_[128] = "";
  char state_[32] = "";
  bool player_ = false;
  int flash_ = -1;
  bool flash_ok_ = false;
  uint32_t flash_at_ = 0;
  lv_obj_t *hint_ = nullptr;
};

}  // namespace transport

tessera::Tile *make_transport(const tessera::Plugin *plugin) { return new transport::KeysTile(plugin); }

}  // namespace esphome::av_wall_panel_controls
