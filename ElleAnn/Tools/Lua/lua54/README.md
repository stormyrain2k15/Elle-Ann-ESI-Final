# Lua 5.4.6 — vendored in-tree

This directory contains the unmodified Lua 5.4.6 source distribution from
https://www.lua.org/ftp/ . It is vendored (checked into git) rather than
downloaded at build time so that:

  • Builds work offline and on firewalled build hosts
  • CI cache misses don't trigger a lua.org download
  • Build reproducibility is locked in — the exact source bytes compiled
    against are the same ones in the commit history
  • A lua.org outage or supply-chain compromise cannot affect our build

## License

Lua is distributed under the MIT License. See `doc/readme.html` for the
upstream license text:

  > Copyright © 1994–2023 Lua.org, PUC-Rio.

## Provenance

  • Version:    5.4.6
  • Source:     https://www.lua.org/ftp/ (archived ZIP as `bjx6fxkw_lua-5.4.6.zip`)
  • Imported:   Feb 2026 by the user from the official distribution
  • Layout:     `src/` (C sources — all 34 files shipped by lua.org) +
                `doc/` (HTML manual + readme + license)

## How the build uses this

`Elle.Lua.Behavioral.vcxproj` compiles exactly 32 of the 34 `.c` files
(all of `src/*.c` EXCEPT `lua.c` and `luac.c` — those are the interpreter
and compiler entry points, not needed when embedding the library).

Each of those 32 files carries a per-file warning-policy opt-out in the
vcxproj: `<TreatWarningAsError>false</TreatWarningAsError>` +
`<WarningLevel>Level3</WarningLevel>`. Upstream Lua is ANSI C with known
implicit-conversion warnings that fire under our repo-wide `/W4` + `/WX`
policy; vendored third-party code is not ours to audit, so we relax the
policy for these files only. See `AUDIT_SCOREBOARD.md` for the rationale.

## Updating

To bump Lua:

  1. Download the new `.zip` or `.tar.gz` from https://www.lua.org/ftp/
  2. Extract; replace the contents of `src/` and `doc/` with the new release
  3. Verify the SHA-256 of the upstream archive and record it in this
     README as the new "Source" line
  4. Update the per-file `<ClCompile>` list in
     `Lua/Elle.Lua.Behavioral/Elle.Lua.Behavioral.vcxproj` if the set of
     `.c` files changed
  5. Run a full build + smoke test; Lua minor releases rarely break API
     but patch-level releases have added headers before

## Do NOT modify these files

If you find yourself wanting to patch a vendored Lua source file, stop.
Either:

  • Wrap or shim the offending behavior on the Elle side (preferred), or
  • Report the bug upstream and upgrade when a fix lands

Local patches to vendored code become maintenance debt the next time we
try to upgrade. The `!bloated-lua` rule is: zero local modifications to
`src/` or `doc/`.
