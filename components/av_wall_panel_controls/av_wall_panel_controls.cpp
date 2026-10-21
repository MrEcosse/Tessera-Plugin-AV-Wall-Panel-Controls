#include "av_wall_panel_controls.h"

namespace esphome::av_wall_panel_controls {

// The tile ids are the manifest's (tiles[].id).
void AvWallPanelControls::setup() {
  add_tile("transport", [this]() { return make_transport(this); });
  add_tile("dpad", [this]() { return make_dpad(this); });
  add_tile("volume", [this]() { return make_volume(this); });
  add_tile("numpad", [this]() { return make_numpad(this); });
  add_tile("grid", [this]() { return make_grid(this); });
}

}  // namespace esphome::av_wall_panel_controls
