"""Headless map creation for SSXPure.

Usage:
UnrealEditor-Cmd.exe D:\\SSXPure\\SSXPure.uproject -run=pythonscript \
  -script="<abs path to this file>" -unattended -nullrhi -nosplash -nopause -stdout

Writes result JSON to Saved/make_map_result.json. Uses guarded steps so a
partial failure is visible instead of silent.
"""
import json
import os

RESULTS = []


def report(key, ok, detail=""):
    line = "[MYTOOL] %s=%s %s" % (key, "OK" if ok else "FAIL", detail)
    RESULTS.append((key, ok, detail))
    (unreal.log_warning if ok else unreal.log_error)(line)


def step(key):
    def wrap(fn):
        try:
            report(key, True, fn() or "")
        except Exception as exc:
            report(key, False, "%s: %s" % (type(exc).__name__, exc))
    return wrap


@step("CREATE_MAIN_MAP")
def _create_main_map():
    import unreal
    lvl = unreal.LevelEditorSubsystem()
    ok = lvl.new_level("/Game/Maps/Main")
    if not ok:
        raise RuntimeError("new_level returned False")
    return "new_level=/Game/Maps/Main"


@step("ADD_PLAYER_START")
def _add_player_start():
    import unreal
    loc = unreal.Vector(0.0, 0.0, 2000.0)
    rot = unreal.Rotator(0.0, 0.0, 0.0)
    actor = unreal.EditorActorSubsystem().spawn_actor_from_class(
        unreal.PlayerStart, loc, rot)
    if actor is None:
        raise RuntimeError("spawn_actor_from_class returned None")
    return "spawned PlayerStart"


@step("SAVE_MAP")
def _save_map():
    import unreal
    ok = unreal.EditorAssetLibrary.save_asset("/Game/Maps/Main", only_if_is_dirty=True)
    if not ok:
        raise RuntimeError("save_asset returned False")
    return "saved /Game/Maps/Main"


failed = [k for k, ok, d in RESULTS if not ok]
all_ok = len(failed) == 0
out_path = os.path.join(unreal.Paths().project_saved_dir(), "make_map_result.json")
with open(out_path, "w") as f:
    json.dump({"steps": RESULTS, "failed": failed, "all_ok": all_ok}, f, indent=2)
unreal.log_warning("[MYTOOL] RESULT all_ok=%s -> %s" % (all_ok, out_path))
