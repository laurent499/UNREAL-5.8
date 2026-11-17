"""Cree (ou recree) M_PlainRoundedGradient : M_PlainRounded avec un degrade gauche -> droite.

    UnrealEditor-Cmd.exe TrailSimulator.uproject -run=pythonscript -script="<chemin>/create_plain_rounded_gradient.py"

Editeur ferme (ou M_PlainRoundedGradient non ouvert). Sortie :
/Game/LTVContent/Materials/Masters/M_PlainRoundedGradient

Copie de M_PlainRounded (coins arrondis, biseau, memes primitive data 0-9), plus :
- BaseColorB : couleur d'arrivee, primitive data 10-13 (lineaire) ;
- GradientT : position horizontale 0 (gauche de l'ecran) -> 1 (droite), tiree comme le masque
  des bornes locales du composant (les UV des meshes ne couvrent pas toujours le bandeau).
La couleur de depart reste BaseColor (primitive data 0-3). Utilise par le fond du nom des coureurs
GTWS quand le template active "nameGradient" (Runner_GTWS.cpp). M_PlainRounded n'est pas modifie :
relancer ce script apres une retouche de M_PlainRounded pour la reporter ici.
"""
import unreal

SRC = "/Game/LTVContent/Materials/Masters/M_PlainRounded"
DST = "/Game/LTVContent/Materials/Masters/M_PlainRoundedGradient"

GRADIENT_CODE = """// Position horizontale dans le fond : 0 a gauche de l'ecran, 1 a droite (meme repere que RoundedCornerMask).
float3 S = max(BMax - BMin, 1e-4);
float3 N = (LP - BMin) / S;
float2 C;
if (S.x <= S.y && S.x <= S.z)      C = N.yz;
else if (S.y <= S.z)               C = N.xz;
else                               C = N.xy;
float2 dCdx = ddx(C);
float2 dCdy = ddy(C);
float3 dPdx = ddx(WP);
float3 dPdy = ddy(WP);
float det = dCdx.x * dCdy.y - dCdx.y * dCdy.x;
if (abs(det) < 1e-12) return 0.0;
float3 dPdu = (dPdx * dCdy.y - dPdy * dCdx.y) / det;
float3 dPdv = (dPdy * dCdx.x - dPdx * dCdy.x) / det;
// Axe horizontal : celui qui ne suit pas le Z monde, oriente vers la droite de l'ecran
bool VertV = abs(dPdv.z) >= abs(dPdu.z);
float H = VertV ? C.x : C.y;
float Dir = VertV ? (dCdx.x >= 0.0 ? 1.0 : -1.0) : (dCdx.y >= 0.0 ? 1.0 : -1.0);
return saturate(Dir > 0.0 ? H : 1.0 - H);
"""

mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary

if eal.does_asset_exist(DST):
    eal.delete_asset(DST)
mat = eal.duplicate_asset(SRC, DST)
if mat is None:
    raise RuntimeError("Duplication de %s impossible" % SRC)

base = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_BASE_COLOR)
mask = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_OPACITY_MASK)
if base is None or mask is None:
    raise RuntimeError("Graphe de M_PlainRounded inattendu (BaseColor ou masque absent)")

# Noeuds du masque d'arrondi, repris pour le degrade
corner = None
for n in mel.get_inputs_for_material_expression(mat, mask):
    if isinstance(n, unreal.MaterialExpressionCustom):
        corner = n
if corner is None:
    raise RuntimeError("Noeud RoundedCornerMask introuvable")
names = [str(x) for x in mel.get_material_expression_input_names(corner)]
ins = dict(zip(names, mel.get_inputs_for_material_expression(mat, corner)))
local_pos, bounds, world_pos = ins.get("LP"), ins.get("BMin"), ins.get("WP")
if not (local_pos and bounds and world_pos):
    raise RuntimeError("Entrees LP / BMin / WP du masque introuvables : %s" % names)

# Couleur d'arrivee : primitive data 10-13
color_b = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -1040, -220)
color_b.set_editor_property("parameter_name", "BaseColorB")
color_b.set_editor_property("default_value", unreal.LinearColor(1, 1, 1, 1))
color_b.set_editor_property("use_custom_primitive_data", True)
color_b.set_editor_property("primitive_data_index", 10)

grad = mel.create_material_expression(mat, unreal.MaterialExpressionCustom, -1040, -400)
grad.set_editor_property("description", "GradientT")
grad.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
grad.set_editor_property("code", GRADIENT_CODE)
inputs = []
for nm in ["LP", "BMin", "BMax", "WP"]:
    ci = unreal.CustomInput()
    ci.set_editor_property("input_name", nm)
    inputs.append(ci)
grad.set_editor_property("inputs", inputs)
mel.connect_material_expressions(local_pos, "", grad, "LP")
mel.connect_material_expressions(bounds, "Min", grad, "BMin")
mel.connect_material_expressions(bounds, "Max", grad, "BMax")
mel.connect_material_expressions(world_pos, "", grad, "WP")

lerp = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -800, -120)
mel.connect_material_expressions(base, "", lerp, "A")
mel.connect_material_expressions(color_b, "", lerp, "B")
mel.connect_material_expressions(grad, "", lerp, "Alpha")

# Tout ce qui lisait BaseColor lit maintenant le degrade (sortie BaseColor et emissive)
mel.connect_material_property(lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)
emissive = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
stack = [emissive]
seen = set()
while stack:
    n = stack.pop()
    if n is None or n.get_name() in seen or n == lerp:
        continue
    seen.add(n.get_name())
    n_names = [str(x) for x in mel.get_material_expression_input_names(n)]
    for nm, src in zip(n_names, mel.get_inputs_for_material_expression(mat, n)):
        if src == base:
            mel.connect_material_expressions(lerp, "", n, nm)
        else:
            stack.append(src)

mel.recompile_material(mat)
eal.save_loaded_asset(mat)
unreal.log("M_PlainRoundedGradient cree")
