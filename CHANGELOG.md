# Changelog

## 0.1.1

- Builds on Tessera 0.4.91 and later (plugin API 0.24), which took away the key and value form of `tessera::action`.
  A key's field now goes as a JSON object, its value as the text typed, as before.
- Selector grid: the twelve keys, the arrows, the page number and the hint are made once when the tile is; a change of
  the options only lays them out again and shows or hides them, instead of deleting and making them anew.
- Selector grid: the options that come are compared with the ones kept as they arrive, so the tile keeps one list of up
  to 64, not two; it declares the 7.2 KB that takes (it said 7 KB and could keep 9.5).

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
