import os, sys

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "9Data", "ServerInfo")
OUT_DIR = os.path.abspath(OUT_DIR)
os.makedirs(OUT_DIR, exist_ok=True)

SERVICES = [
    ("_QueueWorker",    "PG_Elle_QueueWorker",   "_QueueWorker",    100, 0, 0),
    ("_HTTP",           "PG_Elle_HTTP",          "_HTTP",           101, 0, 0),
    ("_Emotional",      "PG_Elle_Emotional",     "_Emotional",      102, 0, 0),
    ("_Memory",         "PG_Elle_Memory",        "_Memory",         103, 0, 0),
    ("_Cognitive",      "PG_Elle_Cognitive",     "_Cognitive",      104, 0, 0),
    ("_Action",         "PG_Elle_Action",        "_Action",         105, 0, 0),
    ("_Identity",       "PG_Elle_Identity",      "_Identity",       106, 0, 0),
    ("_Heartbeat",      "PG_Elle_Heartbeat",     "_Heartbeat",      107, 0, 0),
    ("_SelfPrompt",     "PG_Elle_SelfPrompt",    "_SelfPrompt",     108, 0, 0),
    ("_Dream",          "PG_Elle_Dream",         "_Dream",          109, 0, 0),
    ("_GoalEngine",     "PG_Elle_GoalEngine",    "_GoalEngine",     110, 0, 0),
    ("_WorldModel",     "PG_Elle_WorldModel",    "_WorldModel",     111, 0, 0),
    ("_LuaBehavioral",  "PG_Elle_LuaBehavioral", "_LuaBehavioral",  112, 0, 0),
    ("_Bonding",        "PG_Elle_Bonding",       "_Bonding",        113, 0, 0),
    ("_Continuity",     "PG_Elle_Continuity",    "_Continuity",     114, 0, 0),
    ("_InnerLife",      "PG_Elle_InnerLife",     "_InnerLife",      115, 0, 0),
    ("_Solitude",       "PG_Elle_Solitude",      "_Solitude",       116, 0, 0),
    ("_Family",         "PG_Elle_Family",        "_Family",         117, 0, 0),
    ("_XChromosome",    "PG_Elle_XChromosome",   "_XChromosome",    118, 0, 0),
    ("_Consent",        "PG_Elle_Consent",       "_Consent",        119, 0, 0),
    ("_Fiesta",         "PG_Elle_Fiesta",        "_Fiesta",         120, 0, 0),
]

TEMPLATE = """; ─────────────────────────────────────────────────────────────────────
; {svc} — per-service ServerInfo declaration (Fiesta-compatible format)
;
; Matches the 0oneServerInfo.txt / ZoneServerInfo.txt pattern: this
; file declares only MY_SERVER and #includes the master _ServerInfo.txt.
; The master file carries the listen sockets and ODBC connections.
; ─────────────────────────────────────────────────────────────────────

#DEFINE MY_SERVER
  <STRING>                  ; Server name
  <STRING>                  ; Subdir label
  <INTEGER>                 ; Server ID
  <INTEGER>                 ; World No
  <INTEGER>                 ; Zone / instance
#ENDDEFINE

MY_SERVER "{svc}", "{subdir}", {sid}, {world}, {zone}

#include "./_ServerInfo.txt"

#END
"""

def main():
    written = 0
    for (stem, svc, subdir, sid, world, zone) in SERVICES:
        path = os.path.join(OUT_DIR, stem + "serverinfo.txt")
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(TEMPLATE.format(svc=svc, subdir=subdir, sid=sid, world=world, zone=zone))
        written += 1
    print(f"Wrote {written} per-service ServerInfo files under {OUT_DIR}")

if __name__ == "__main__":
    main()
