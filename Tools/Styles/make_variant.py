"""Cree une instance de materiau de test (parametres scalaires en JSON dans CS_PARAMS)."""
import json, os, unreal
parent = unreal.EditorAssetLibrary.load_asset(os.environ["CS_PARENT"])
d, n = os.environ["CS_DIR"], os.environ["CS_NAME"]
path = f"{d}/{n}"
if unreal.EditorAssetLibrary.does_asset_exist(path):
    unreal.EditorAssetLibrary.delete_asset(path)
mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(n, d, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
unreal.MaterialEditingLibrary.set_material_instance_parent(mi, parent)
for k, v in json.loads(os.environ.get("CS_PARAMS", "{}")).items():
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, k, float(v))
unreal.MaterialEditingLibrary.update_material_instance(mi)
unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
unreal.log(f"CELLSHADING_MI_OK {path}")
