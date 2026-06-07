# Repo-Local Configuration Surfaces — Audit Note

Feb 2026 audit item #128 asked for an accounting of any unusual
configuration / tool surfaces living inside the repository itself, so
an operator auditing the source tree can tell what is project code,
what is platform metadata, and what (if anything) is consumed at
runtime.

## `.git/`
Standard Git metadata. No special handling.

## `.github/workflows/elleann-build.yml`
CI pipeline. Runs 8 static-audit jobs on every push to `main` or PR:
SLN integrity, C++ balance audit, SQL delta idempotency, SQL schema
end-to-end coherence, PowerShell syntax, service-name manifest
consistency, cppcheck (C++ static analysis), and the Windows MSBuild
job that compiles the full solution against Lua 5.4 with MD5-verified
download. All jobs must pass for a green ✓.

## `.emergent/emergent.yml`
**Platform metadata**, written by the Emergent development platform
(the cloud IDE / forked-agent runtime some contributors use). Records
the base container image, a per-run `job_id`, and a creation
timestamp. **Not consumed by Elle-Ann at runtime.** The file is safe
to ignore on a local clone; it is equally safe to leave in place. It
does not contain any secrets.

## `.gitconfig`
**Platform metadata**, also written by the Emergent platform. Sets
the Git `user.email` and `user.name` used when the cloud agent
commits on your behalf. Local clones that commit from a developer
workstation will override this with their own `~/.gitconfig`; the
file's presence has no effect on runtime behaviour.

## `frontend/`
**Optional dev-time control surface.** Not part of the shipped
product. See `frontend/README.md` for details. The Elle-Ann service
bundle does not depend on this directory.

## `backend/`
**Optional dev-time scaffold.** Not part of the shipped product. The
canonical backend is the 20-service C++ bundle under `ElleAnn/`.

## `ElleAnn/Deploy/video_worker/`
External Python worker for Wav2Lip / ffmpeg. Runs as a separate
process (typically on a GPU box) that polls the C++ HTTP service for
video jobs. See `ElleAnn/Deploy/video_worker/README.md` for the
deployment contract. The C++ core runs to full functionality without
it; the video endpoint simply returns a "worker offline" status when
no worker is polling.

## `memory/`
Notes written by cloud fork agents (PRD, test credentials, handoff
summaries). **Not consumed at runtime.** Safe to delete or ignore on
any local clone.

## `.gitignore` / `.dockerignore` (if present)
Standard ignore lists. No special handling.

---

## Summary

Everything in the repo falls into one of four buckets:

1. **Product** — `ElleAnn/` (C++ / MASM / Lua), the installer, the SQL
   deltas, the CI workflow that gates merges.
2. **Optional dev surfaces** — `frontend/`, `backend/`,
   `ElleAnn/Deploy/video_worker/`.
3. **Platform metadata** — `.emergent/`, `.gitconfig`, `memory/`.
   Not consumed by Elle-Ann, not required for local development.
4. **Git plumbing** — `.git/`, `.github/`.

No hidden runtime dependencies, no unexpected third-party tool
configurations, no auto-updating vendor directories. If it isn't in
the above list, it's project source.
