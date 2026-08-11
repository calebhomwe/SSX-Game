# SNOWBOARDING SSX — HANDOVER (continue WITHOUT Claude)

You're low on Claude credits. Everything below lets the build continue using the cheap
**Qwen** models directly (no Claude). All work is committed in git.

## The game
- Project: `C:\Users\caleb\skywalker-studio\games\snowboarding` (Godot 4.7)
- Godot exe: `C:\Users\caleb\OneDrive\Desktop\Godot_v4.7-stable_win64.exe`
- Repo: parent `C:\Users\caleb\skywalker-studio` (one git repo; SwellRider + blender-mcp are submodules). **Never `git add -A`** — stage explicit paths.
- Play it: `Godot_v4.7-stable_win64.exe --path C:\Users\caleb\skywalker-studio\games\snowboarding res://Main.tscn`

## What's DONE (this session)
Player fixed (crouched mesh + procedural motion), controller (uber=RB, pause ESC/START),
neon rails + tunnels + giant ice arches, silhouette peaks, fog+bloom+saturation, hang-time
slow-mo, speed lines, camera roll, landing punch, bigger booters, carve spray, TRICKY
screen-warp, speed→chromatic, rival barks, combo→boost meter, branching shortcut, dynamic
music. Music runs **muted** by default (main.gd mutes the Music bus).

## The tools (already built, in `games/snowboarding/tools/`)
- `python tools/vision_judge.py <shot.png>` → Qwen `qwen3-vl-plus` scores a screenshot vs SSX
  (JSON: overall/mountain/ramps/rails/richness/character + fixes). **NOTE: noisy + blind to
  motion; trust your eyes.**
- `python tools/qwen_research.py "<question>"` → Qwen `qwen3.7-max-2026-05-17` returns
  pros/cons + steps. Research + critique lane.
- Take a screenshot: `Godot...exe --path <proj> res://Main.tscn -- --cam "action wide" --shot 12 --out C:/tmp/s.png`

## Working AI models (tested; via DashScope key `DASHSCOPE_API_KEY` in `~/.env`)
- **Code gen (fast/cheap):** `qwen3-coder-480b-a35b-instruct` (1.5s) — USE THIS to write code.
- Research/reasoning: `qwen3.7-max-2026-05-17` (8s).
- Vision: `qwen3-vl-plus`.
- Kimi coder also works: `kimi-k2.7-code` (needs the Moonshot endpoint/key, not DashScope).
- AVOID (403/timeout): `qwen3.7-max` alias, `-06-08`, `-05-20`, `-05-26`, `qwen3.6-max-preview`.

## Remaining backlog (from docs/RESEARCH_LOG.md, newest critique R6)
1. Heavier/tactile carve PHYSICS feel (edge grip, momentum swing).
2. Deeper combo → screen-shattering UBER trick payoff (combo meter exists; add the Uber climax).
3. More branching routes / risk-reward shortcuts (one exists — add 1-2 more).
4. Themed BIOME variety (Tokyo/Alps/Volcano color+prop swaps per World Tour stop).
5. Dynamic music polish (real music tracks, beat-reactive; currently ducking logic only).
6. Rival AI with real overtake/rubber-band behavior + personality.
7. Speed-based FOV already partial — push it + tie chromatic harder.

## HOW TO CONTINUE WITHOUT CLAUDE
### Option A — automated (run the builder)
Double-click `RUN_SNOWBOARD_AUTOBUILD.bat` on the Desktop (or run `python autobuild_snowboard.py`).
It uses `qwen3-coder-480b` to implement the next backlog item as a NEW self-contained `.gd`
file, parse-checks it in Godot, and commits it. Then it prints the one line to paste into
`main.gd`'s `_ready()` to wire it in (the only manual step — automated wiring is risky).
Edit the `BACKLOG` list at the top of `autobuild_snowboard.py` to steer it.

### Option B — manual (you + Qwen)
1. `python tools/qwen_research.py "what should I add next to make X feel like SSX?"` → get a plan.
2. Ask `qwen3-coder-480b` (via DashScope, or your `bl` CLI) to write ONE new `.gd` file for it,
   as a self-contained node that reads the player via `get_tree().get_first_node_in_group("player")`
   and does NOT edit player.gd/main.gd.
3. Save it under `scripts/`, then `Godot...exe --headless --path <proj> --import`, boot Main.tscn,
   look at a `--shot`. If good: `git add games/snowboarding/scripts/<file>` and commit.
4. Wire it: add `add_child(preload("res://scripts/<file>").new())` in `main.gd` `_ready()`.

## Rules that keep it safe (the traps I hit)
- **NEVER edit player.gd or main.gd from an auto-agent** — new features = new self-contained
  nodes that watch the `"player"` group. main.gd wiring is a 1-line manual paste.
- `Engine.time_scale` must ALWAYS restore to 1.0 (hang-time does this).
- New `class_name` script → run `--import` before the game will see it.
- `--check-only --script X.gd` FALSE-flags autoloads (GameSession) + global class_names
  (Characters, CourseRails…) as "not declared" — IGNORE those; a clean **runtime boot** is truth.
- Terrain height lives in TWO copies that must stay identical: `main.gd::_get_height` and
  `player.gd::_get_terrain_height`. Don't change one without the other.
- Big terrain bumps make white walls — build ramps as MESH wedges, not terrain height.
- The vision judge caps ~4-5 because the art is low-poly (it wants photoreal). Don't chase
  its number; build FEEL + spectacle and judge by eye.

## SwellRider (surf game) — parallel, separate submodule repo
`games/SwellRider` — its own git repo. Wave-realism + beach + HUD done & committed there.
Same tools/models apply; commit inside that folder.
