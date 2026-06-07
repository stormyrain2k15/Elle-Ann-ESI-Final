# 9Data — Fiesta-integration tree

This folder mirrors the directory layout Elle drops into on a live
FiestaOnline server. At deploy time you copy the contents of this
folder on top of the server's `9Data\` (or set
`Installation.root_path` in `ServerInfo.txt` to point the services
at wherever your `9Data` actually lives).

Philosophy: Elle should be **indistinguishable from native Fiesta
services at the file level**. Same folder structure, same config
filenames, same patterns, same dispatcher behaviour. An admin opening
the server directory should see Elle's files sit alongside Zone /
World / Login like they were always there.

## Layout

```
9Data/
└── Hero/
    └── LuaScript/
        └── ElleLua/             ← Elle's entire Lua surface
            ├── settings.lua     ← behaviour / personality / tuning
            ├── creative_synthesis.lua
            ├── dream_processing.lua
            ├── ethical_reasoning.lua
            ├── extended_behaviors.lua
            ├── goal_formation.lua
            ├── inner_life.lua
            ├── intent_scoring.lua
            ├── metacognition.lua
            ├── personality.lua
            ├── reasoning.lua
            ├── self_reflection.lua
            ├── social_modeling.lua
            ├── temporal_reasoning.lua
            ├── thresholds.lua
            └── x_subjective.lua
```

`settings.lua` is the ONE file an operator edits to tune Elle. Every
other file is a behaviour script loaded by `Elle.Lua.Behavioral.exe`.

## Paths at runtime

`ElleConfig::LoadFromServerInfo` resolves the Lua directory by looking
at `Installation.root_path` in `ServerInfo.txt` and appending the
Fiesta-convention suffix `9Data\Hero\LuaScript\ElleLua`. So wherever
your server lives, Elle finds her scripts automatically.

## Dev → prod sync

```
# From repo root, on your build/dev box:
robocopy ElleAnn\9Data  <FiestaRoot>\9Data  /E
```

That's the entire deploy step for the script side. Service binaries
are handled separately via double-click (see
`Shared/ElleServiceBase.cpp::DoubleClickInstall` — Fiesta-style
`<SERVICE UPLOAD ONLY OK>` dialog).
