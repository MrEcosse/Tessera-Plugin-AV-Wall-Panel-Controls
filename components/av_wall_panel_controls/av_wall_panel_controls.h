#pragma once
#include "esphome/core/component.h"
#include "esphome/components/smart_display/plugin_api.h"

namespace esphome::av_wall_panel_controls {

// The plugin: an ESPHome component and a tessera::Plugin. It registers its five tile types in setup(); each lives in
// a file and a namespace of its own and is made by one of these.
tessera::Tile *make_transport(const tessera::Plugin *plugin);  // transport.cpp: skip, stop, play, pause, eject
tessera::Tile *make_dpad(const tessera::Plugin *plugin);       // dpad.cpp: arrows, OK and the four corners
tessera::Tile *make_volume(const tessera::Plugin *plugin);     // volume.cpp: quieter, mute, louder; or channels
tessera::Tile *make_numpad(const tessera::Plugin *plugin);     // numpad.cpp: the digits, Clear and Enter
tessera::Tile *make_grid(const tessera::Plugin *plugin);       // grid.cpp: a selector's options as keys

class AvWallPanelControls : public Component, public tessera::Plugin {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
};

}  // namespace esphome::av_wall_panel_controls
