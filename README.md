# SFML Game Demo

## Example

![Game](https://github.com/nerealnaya-zomba/SFML-Game-Demo/blob/main/screenshot.png)

## How to try

Open terminal and enter this:
``` bash
cmake -E make_directory build
cmake -E chdir build cmake ..
cmake -E chdir build cmake .. --build
```
Executable located at ./build/bin

***If something not working, try to open folder in VS Code and configure&build using CMake extension.***

## Keybindings

- Left/Right/Up/Down Arrows - Moving
- Z - Jump
- X - Shot
- C - Dash
- R - Open portal
- F1 - Open developer overlay

## Integrated Libraries

- `TGUI` - main menu, pause menu and popup UI
- `imgui-sfml` - in-game developer overlay for quick runtime inspection and actions
- `SelbaWard` - enhanced objective HUD visuals via animated ring accents and a dedicated progress bar
- persistent launch settings - remembered `VSync` and menu ash density between launches

## Overlay Features

Press `F1` in-game to open the new `Ritual Console` overlay. While it is open, the world simulation is paused and you can:

- inspect FPS, frame time, level info and camera data
- inspect player health, energy, gold and movement stats
- restore vitals, add gold, restart the current level, respawn at checkpoint or return to base
- inspect ability cooldowns and the active campaign objective
- quick-travel between unlocked levels without leaving gameplay
- toggle `VSync` and menu ash density from the in-game session tools
- see live toast notifications for settings changes, quick actions and major run events

## Saved Preferences

The menu settings now persist in `data/launchSettings.json`.
At the moment the game remembers:

- `VSync`
- menu ash density (`Low`, `Medium`, `High`)
- ImGui overlay layout in `data/imgui.ini`

## Progress Reset

Open `Settings` from the main menu or pause menu to find `Reset saved progress`.
The confirmation flow clears saved gold, relics, weapons and unlocked gates, then returns the run to the starting state.

## Compatibility Notes

This project now targets `SFML 3`, so not every library from the classic SFML ecosystem can be dropped in safely.
Libraries such as `imgui-sfml` and `SelbaWard` were integrated directly because they already support `SFML 3`.
Some others from older SFML lists still target `SFML 2.x`, so they were intentionally left out for now to avoid breaking the build.
