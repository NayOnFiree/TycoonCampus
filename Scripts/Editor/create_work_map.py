"""Jalon 1.1: generate the initial blockout once in a separate editor commandlet.

Refuses to overwrite an existing map. No dependency on this script at runtime.
"""
import json
from pathlib import Path
import unreal

MAP = "/Game/TycoonCampus/Maps/L_Campus_Work"
ROOT = "/Game/TycoonCampus/Blockout"
project = Path(unreal.Paths.project_dir())
report = project / "Saved" / "CampusWorkMapReport.json"
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("Work map already exists; refusing to overwrite user edits.")

world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assets = unreal.AssetToolsHelpers.get_asset_tools()
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
assert world and cube

def material(name, rgb):
    path = ROOT + "/Materials/" + name
    existing = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if existing:
        return existing
    mat = assets.create_asset(name, ROOT + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
    color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector)
    color.set_editor_property("constant", unreal.LinearColor(*rgb, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant)
    rough.set_editor_property("r", 0.85)
    unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    return mat

grass = material("M_Blockout_Grass", (0.20, 0.32, 0.15))
path_mat = material("M_Blockout_Path", (0.45, 0.46, 0.42))
wall = material("M_Blockout_Wall", (0.65, 0.68, 0.65))
court = material("M_Blockout_Court", (0.49, 0.28, 0.12))
line = material("M_Blockout_Line", (0.85, 0.87, 0.78))

def box(label, location, size, mat, folder):
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    mesh = actor.static_mesh_component
    mesh.set_static_mesh(cube)
    mesh.set_material(0, mat)
    actor.set_actor_scale3d(unreal.Vector(*(v / 100.0 for v in size)))
    return actor

# Unreal units are centimetres. Ground top is Z=0; campus is 200 x 200 m.
box("Campus_Ground_200m", (0, 0, -50), (20000, 20000, 100), grass, "01_Terrain")
# West-side entrance, 4 m wide pedestrian approach.
box("Entrance_Plaza", (-8500, 0, 5), (1200, 1600, 10), path_mat, "02_Access")
box("Main_Path_4m", (-4550, 0, 6), (6700, 400, 12), path_mat, "02_Access")
box("Entrance_Pillar_North", (-9000, 700, 150), (100, 100, 300), wall, "02_Access")
box("Entrance_Pillar_South", (-9000, -700, 150), (100, 100, 300), wall, "02_Access")

# Hall 40 x 26 m, open west entrance, roof intentionally absent at this stage.
box("Gym_Floor_40x26m", (700, 0, 10), (4000, 2600, 20), wall, "03_Gym_Placeholder")
box("Gym_Wall_North", (700, 1300, 410), (4100, 40, 800), wall, "03_Gym_Placeholder")
box("Gym_Wall_South", (700, -1300, 410), (4100, 40, 800), wall, "03_Gym_Placeholder")
box("Gym_Wall_East", (2700, 0, 410), (40, 2600, 800), wall, "03_Gym_Placeholder")
box("Gym_West_North", (-1300, 800, 410), (40, 1000, 800), wall, "03_Gym_Placeholder")
box("Gym_West_South", (-1300, -800, 410), (40, 1000, 800), wall, "03_Gym_Placeholder")
box("Basket_Surface_28x15m", (700, 0, 22), (2800, 1500, 4), court, "03_Gym_Placeholder")
for y in (-750, 750):
    box("Court_Sideline_" + str(y), (700, y, 25), (2810, 5, 2), line, "03_Gym_Placeholder")
for x in (-700, 700, 2100):
    box("Court_Line_" + str(x), (x, 0, 25), (5, 1500, 2), line, "03_Gym_Placeholder")

sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-45, -30, 0))
sun.set_actor_label("Sun_WorkLight")
sun.set_folder_path("04_Lighting")
sun.light_component.set_editor_property("intensity", 3.0)
sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1500))
sky.set_actor_label("Sky_WorkLight")
sky.set_folder_path("04_Lighting")
atmosphere = actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
atmosphere.set_folder_path("04_Lighting")
start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-6500, 0, 5500), unreal.Rotator(-40, 0, 0))
start.set_actor_label("PlayerStart_WorkOverview")
start.set_folder_path("05_View")

assert unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
loaded = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
assert loaded, "Saved map did not reload"
labels = {a.get_actor_label() for a in actors.get_all_level_actors()}
required = {"Campus_Ground_200m", "Main_Path_4m", "Gym_Floor_40x26m", "PlayerStart_WorkOverview", "Sun_WorkLight"}
assert required <= labels, "Missing essential actors after reload"
report.write_text(json.dumps({"map": MAP, "saved_and_reloaded": True, "actor_count": len(labels), "required_actors": sorted(required), "parcel_metres": [200, 200], "note": "Structural validation only; no gameplay or FPS claim."}, indent=2), encoding="utf-8")
unreal.log("CAMPUS_WORK_MAP_SUCCESS " + str(report))
