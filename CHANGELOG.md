# Changelog

## 0.1.0
- First version, as one plugin of five tiles: transport controls, a D-pad, volume and channel controls, a number pad and
  a selector grid. They were four plugins of their own before (transport_controls 0.2.1, d_pad 0.1.2, volume_controls
  0.1.2, number_pad 0.1.1) and a fifth that was never released; one plugin keeps the command parser in flash once and
  gives one release to follow.
- Transport controls: the keys wrap onto two or three rows on a tile too narrow for them side by side (the 0.2.1
  release of the old plugin had gone out without it). Eject shows the word until Tessera's icon set has the symbol,
  instead of a drawing of the plugin's own.
- Volume and channel controls: a Style option, Volume or Channel (two chevron keys, no mute).
- The memory each tile declares counts what it keeps: about 180 bytes a key and 228 for each key's action.
- The manifest names every action the README uses, `webostv.button` included.
