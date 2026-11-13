"""Crée (ou recrée) /Game/LTVContent/Materials/Styles/PP_Grade : le matériau de post-process
des presets Noir et blanc, Sépia, Duotone (contrat styles v1, section "postprocess").

Lancement, éditeur ouvert :  Output Log > Python :  py "<projet>/Tools/Styles/create_pp_grade.py"
Lancement, éditeur fermé  :  UnrealEditor-Cmd.exe TrailSimulator.uproject -run=pythonscript
                              -script="<projet>/Tools/Styles/create_pp_grade.py" -unattended -nosplash

Point d'insertion : "Scene Color Before DOF" (= avant translucence). Le matériau ne voit que les
objets opaques/masqués (terrain Cesium, ciel UDS, brouillard, tracé, panneaux masqués). Les
objets translucides (cartes M_Master, halos M_Glow, textes M_RawText) sont composés APRÈS lui :
ils gardent leur couleur sans rien faire. Les objets opaques/masqués à protéger (tracé,
panneaux des bornes, pictos) portent CustomStencil = 200 : le matériau les laisse intacts.
"""
import unreal

PACKAGE_PATH = "/Game/LTVContent/Materials/Styles"
ASSET_NAME = "PP_Grade"

# Une seule passe plein écran, ~20 instructions ALU + 2 lectures de texture.
HLSL = r"""
const float3 W = float3(0.2126, 0.7152, 0.0722);
float3 src = Color.rgb;
float lum = max(dot(src, W), 1e-5);
float pivot = max(Pivot, 1e-3);
// Contraste en espace logarithmique autour du gris moyen (couleur de scène HDR pré-exposée)
float l = pivot * pow(lum / pivot, max(Contrast, 0.01)) * Brightness;
// Teintes normalisées en luminance : elles colorent sans éclaircir ni assombrir
float3 ts = ShadowTint.rgb / max(dot(ShadowTint.rgb, W), 1e-3);
float3 th = HighlightTint.rgb / max(dot(HighlightTint.rgb, W), 1e-3);
float t = saturate(l / (l + pivot));
float3 mono = l * lerp(ts, th, t);
float3 graded = lerp(mono, src * (l / lum), saturate(Saturation));
float keep = (ProtectOverlays > 0.5 && abs(Stencil.r - 200.0) < 0.5) ? 1.0 : 0.0;
return lerp(src, graded, saturate(Amount) * (1.0 - keep));
"""

SCALARS = [  # nom, défaut (défauts = N&B neutre)
    ("Amount", 1.0),
    ("Saturation", 0.0),
    ("Contrast", 1.0),
    ("Brightness", 1.0),
    ("Pivot", 0.18),
    ("ProtectOverlays", 1.0),
]
VECTORS = [
    ("ShadowTint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0)),
    ("HighlightTint", unreal.LinearColor(1.0, 1.0, 1.0, 1.0)),
]


def enum_member(enum_cls, *candidates):
    for name in candidates:
        if hasattr(enum_cls, name):
            return getattr(enum_cls, name)
    raise RuntimeError("Aucun de %s dans %s" % (candidates, enum_cls))


def main():
    mel = unreal.MaterialEditingLibrary
    full_path = "%s/%s" % (PACKAGE_PATH, ASSET_NAME)

    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        material = unreal.EditorAssetLibrary.load_asset(full_path)
        mel.delete_all_material_expressions(material)
    else:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            ASSET_NAME, PACKAGE_PATH, unreal.Material, unreal.MaterialFactoryNew())

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    material.set_editor_property("blendable_location", enum_member(
        unreal.BlendableLocation, "BL_SCENE_COLOR_BEFORE_DOF", "SCENE_COLOR_BEFORE_DOF",
        "BL_BEFORE_TRANSLUCENCY", "BEFORE_TRANSLUCENCY"))
    material.set_editor_property("blendable_output_alpha", False)

    def scene_texture(tex_id, y):
        node = mel.create_material_expression(material, unreal.MaterialExpressionSceneTexture, -900, y)
        node.set_editor_property("scene_texture_id", tex_id)
        return node

    color = scene_texture(enum_member(unreal.SceneTextureId, "PPI_POST_PROCESS_INPUT0", "POST_PROCESS_INPUT0"), 0)
    stencil = scene_texture(enum_member(unreal.SceneTextureId, "PPI_CUSTOM_STENCIL", "CUSTOM_STENCIL"), 200)

    custom = mel.create_material_expression(material, unreal.MaterialExpressionCustom, -300, 0)
    custom.set_editor_property("code", HLSL)
    custom.set_editor_property("description", "Grade N&B / Sepia / Duotone")
    custom.set_editor_property("output_type", enum_member(
        unreal.CustomMaterialOutputType, "CMOT_FLOAT3", "FLOAT3"))

    names = ["Color", "Stencil"] + [n for n, _ in SCALARS] + [n for n, _ in VECTORS]
    inputs = []
    for n in names:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", n)
        inputs.append(ci)
    custom.set_editor_property("inputs", inputs)

    mel.connect_material_expressions(color, "Color", custom, "Color")
    mel.connect_material_expressions(stencil, "Color", custom, "Stencil")

    y = 400
    for name, default in SCALARS:
        p = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -900, y)
        p.set_editor_property("parameter_name", name)
        p.set_editor_property("default_value", default)
        mel.connect_material_expressions(p, "", custom, name)
        y += 100
    for name, default in VECTORS:
        p = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -900, y)
        p.set_editor_property("parameter_name", name)
        p.set_editor_property("default_value", default)
        mel.connect_material_expressions(p, "", custom, name)
        y += 150

    mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    unreal.log_warning("[Styles] OK " + full_path)


main()
