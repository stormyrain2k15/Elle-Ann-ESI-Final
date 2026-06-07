# Fiesta-Format ServerInfo — Elle Deployment Layout

Elle-Ann's services read their listen sockets and ODBC connections from
text files that use the **exact same grammar Fiesta's original server
tools use**. If you've run `JHR_ServerInfo.txt`, `ZoneServerInfo.txt`,
or `0oneServerInfo.txt` before, this file tree will look familiar.

## Layout

```
9Data\ServerInfo\
├── _ServerInfo.txt                ; master — all SERVER_INFO + ODBC_INFO
├── _HTTPserverinfo.txt            ; one per service, #includes _ServerInfo.txt
├── _Cognitiveserverinfo.txt
├── _Emotionalserverinfo.txt
│   … 18 more
└── _Fiestaserverinfo.txt
```

Each per-service file declares one `MY_SERVER` line (its own name, subdir
label, Server ID, World, Zone) and pulls the master file in via
`#include "./_ServerInfo.txt"`. This mirrors how Fiesta's zone services
(`0oneServerInfo.txt` → `ZoneServerInfo.txt` → `JHR_ServerInfo.txt`) are
chained.

## Server ID map

Elle services live at IDs 100–120 so they never collide with a legacy
Fiesta deploy (0–20 reserved for Account/Login/Zone/OPTOOL/Launcher/
Client). See the header of `_ServerInfo.txt` for the full ID table.

## Runtime

`ElleConfig::LoadFromServerInfo(<path>)` picks the right file by convention:
when the config path has `"ServerInfo"` in it and a `.txt` extension,
`ElleConfig::Load` switches to the Fiesta-grammar loader automatically.
Services pass their own `_<Service>serverinfo.txt` at startup and the
loader follows the `#include` chain to the master.

## Editing

* Keep the master file as the single source of truth for `SERVER_INFO`
  and `ODBC_INFO`.
* Per-service files change only when a service is added, removed, or
  its Server ID is reshuffled.
* Passwords never live in these files on production hosts — swap the
  committed `Trusted_Connection=Yes` for an environment-supplied
  `UID=... PWD=...` pair in your local copy.

## Regenerating

If the service list changes, re-run:

```bash
python3 Deploy/gen_serverinfo_files.py
```

The script is the source of truth for the 21-service enumeration; the
.txt files are derived artifacts.
