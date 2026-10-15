# Claude Mod Menu

A [Geode](https://geode-sdk.org) mod menu for Geometry Dash 2.2081 (Geode 5.11.1) with level, editor and general hacks. See [about.md](about.md) for the feature list.

## Getting the mod

Every push runs the **Build Geode Mod** workflow. When it finishes, open the run in the Actions tab and download the **Build Output** artifact. It contains `randonaughtica.claude-mod-menu.geode` built for Windows, macOS, iOS and Android. Put that file in your Geode `mods` folder.

## Layout

```
src/
  Hacks.hpp/.cpp      hack list, state and Safe Mode logic (state is stored as Geode settings)
  MenuPopup.hpp/.cpp  the tabbed menu and the round menu button
  main.cpp            loads settings and registers the open-menu keybind
  hooks/
    Level.cpp         PlayLayer, GJBaseGameLayer, PlayerObject and CCScheduler hooks
    Editor.cpp        EditLevelLayer, EditorUI and LevelInfoLayer hooks
    Other.cpp         icon unlocks, practice music, menu transitions
    Buttons.cpp       menu buttons in the main menu, pause menu and editor pause menu
scripts/
  validate-mod.mjs    checks mod.json against the setting keys the code uses
```

## Adding a hack

1. Add a `bool` setting to `mod.json`. Its name and description show up in the menu.
2. Add a value to the `Hack` enum in `src/Hacks.hpp` and a row with the same key to `HACKS` in `src/Hacks.cpp`, in the same position.
3. Write the hook in the matching file under `src/hooks` and check `hacks::isOn(Hack::YourHack)`.

`scripts/validate-mod.mjs` runs first in CI and fails if a key from step 2 is missing from `mod.json`.

## CI

`.github/workflows/build.yml` uses the official `geode-sdk/build-geode-mod` actions. All JavaScript actions run on Node.js 24:

- the actions it uses (`checkout@v7`, `setup-node@v6`, `upload-artifact@v7` and the ones inside the Geode actions) all declare `node24`
- `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24` is set for the whole workflow, so nothing falls back to Node.js 20
- the validation job runs its script with `node-version: 24`

## Building locally

Install the [Geode CLI and SDK](https://docs.geode-sdk.org/getting-started/), then:

```bash
geode build
```
