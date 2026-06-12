# SQL Fallback admin console

Minimal HTMX-based operator UI for `Elle.Service.HTTP`'s SQL fallback
queue and poison ledger. Served from a single admin-gated route, uses
the existing JSON endpoints — no new state, no new framework.

## Routes

| route | auth | purpose |
|---|---|---|
| `GET /api/admin/sqlfallback/status` | `AUTH_ADMIN` | All counters: enabled flag, pending/poison file & byte totals, last attempt/success epoch ms, retry counts, last error string |
| `GET /api/admin/sqlfallback/poison?limit=N` | `AUTH_ADMIN` | Existing — lists up to N poison lines |
| `POST /api/admin/sqlfallback/poison/load?limit=N` | `AUTH_ADMIN` | Existing — bulk-replays up to N poison lines back into SQL |
| `GET /api/admin/sqlfallback/console` | `AUTH_ADMIN` | New — serves the HTMX console HTML page |

## Why HTMX, not a SPA

- Zero build step. The HTML is a single string literal in
  `HTTPServer_AdminRoutes.cpp` — no node_modules, no webpack, no
  TypeScript pipeline to maintain alongside a pure-C++ codebase.
- HTMX is a `<script>` tag pulled from a public CDN with subresource
  integrity pinned to a specific version (`1.9.12` with `sha384-…`),
  so air-gapped deployments can mirror the JS to a local file without
  changing the route.
- Pulls live data from the existing JSON endpoints — every refresh of
  the page is just three HTMX requests against routes that already had
  to exist for the service to be observable from a script anyway.
- Total payload: one HTML route handler, no separate static files,
  fits inside the existing routing model without a new TU.

## What the page shows

1. **Status panel** — auto-refreshes every 5s via `hx-trigger="load,
   every 5s, click"`. Renders 14 metrics in a responsive grid with
   color-coded severity:
   - green when healthy (`enabled=true`, no poison files, no last_error)
   - amber when poison files > 0
   - red when `enabled=false` or `last_error` is non-empty
2. **Poison ledger viewer** — on-demand fetch of the latest 100 poison
   lines, rendered as a sortable table with the `Exec/CallProc/QueryParams`
   kind pill-tagged for quick scanning.
3. **Bulk replay button** — gated by `hx-confirm` so operators can't
   one-click 500 INSERTs by accident. Renders a green/red result strip
   below the button on completion.

## Security

The route ships with `AUTH_ADMIN` like every other admin endpoint. Two
operational notes for production:

1. The CDN script tag has SRI but no CSP enforcement at the route level
   — front this with a reverse proxy that adds a `Content-Security-Policy`
   header allowing only `unpkg.com` for `script-src` if you want
   defense-in-depth against a future CDN compromise.
2. The bulk replay endpoint trusts the operator. There is no per-line
   targeting yet; if granular replay matters (e.g. "replay only lines
   where `kind = CallProc`"), extend `ElleSQLFallback::LoadPoisonIntoSql`
   to accept filters and surface those as form fields in the console.

## What this does NOT do (and shouldn't, in this scope)

- No per-line replay (would require `ElleSQLFallback` API surface
  changes; left for a follow-up if operators ask for it).
- No metrics push to Prometheus (use the `ipc_chain_smoke.sh
  --prometheus-textfile` flag for that; the SQL fallback counters
  could get the same treatment if a separate textfile job runs them
  through `/api/admin/sqlfallback/status` on a cron).
- No write-ahead replay log (the existing poison file IS the
  write-ahead log).
- No alarms or paging integration — that's an external concern.
