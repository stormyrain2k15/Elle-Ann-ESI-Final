# Lua Dependency Setup

The `Elle.Lua.Behavioral` service compiles **Lua 5.4 source inline**, so no
prebuilt `lua54.lib` is required. The Lua tarball is **not** checked into
the repo (size + license concerns) — fetch it once with the helper script:

```powershell
cd ElleAnn\Lua
.\Fetch-Lua.ps1           # downloads 5.4.6 by default
# or:
.\Fetch-Lua.ps1 -Version 5.4.7
```

Result: `ElleAnn\Lua\lua54\src\` populated with ~35 `.c` files + headers.
`Elle.Lua.Behavioral.vcxproj` picks them up automatically on next build.

If you prefer **vcpkg** or a **prebuilt lib**, set the `LUA_DIR` environment
variable to the install prefix before opening Visual Studio — see
`BUILD_VS.md` Options B and C.

The CI workflow (`.github/workflows/elleann-build.yml`) runs the same
download step so it builds green without any local setup.
