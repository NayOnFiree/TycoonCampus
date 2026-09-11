"""Add only the selection material; never rewrite the existing work map."""
import unreal
folder = "/Game/TycoonCampus/Blockout/Materials"
path = folder + "/M_Selection"
if not unreal.EditorAssetLibrary.does_asset_exist(path):
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_Selection", folder, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    color = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector)
    color.set_editor_property("constant", unreal.LinearColor(0.05, 0.8, 0.65, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
assert unreal.load_asset(path)
unreal.log("CAMPUS_SELECTION_MATERIAL_OK")
