"""Cree (ou recree) le materiau de post-process du style Cell shading.

    UnrealEditor-Cmd.exe TrailSimulator.uproject -run=pythonscript -script="<chemin>/create_pp_cellshading.py"

Sortie : /Game/LTVContent/Materials/Styles/PP_CellShading
Pilote par le JSON de style de la regie (data/styles/cell_shading.json), section postprocess.
Contrat : Docs/Styles/CONTRAT_STYLES.md (CustomStencil 200 = overlay protege).

Principe (tuiles Cesium photogrammetriques : ombrage cuit dans la photo, normales GBuffer inutiles) :
- aplats : couleur de scene floutee sur 2 anneaux, luminance quantifiee en paliers (Steps) ;
- relief : normale reconstruite depuis la profondeur, ombrage toon relatif au sol plat local
  (verticale du globe SkyAtmosphere) avec le soleil de la scene, en 3 paliers ;
- contours : laplacien de 1/z (silhouettes) + cretes a l'ecart ReliefRadius ;
- ciel intact, fissures entre tuiles bouchees, overlays stencil 200 intacts (ProtectOverlays).
Emplacement : Scene Color After DOF (avant TSR, avant la translucence after-DOF).
Cout mesure : ~0.08 ms GPU en 1920x1080 (RTX 5070 Ti, trace Insights 12/11/2026).
DebugView : 1 contours, 3 stencil, 4 ciel, 5 relief, 7 normale reconstruite, 8 verticale.
"""
import unreal

import os
PKG_DIR = os.environ.get("CS_DIR", "/Game/LTVContent/Materials/Styles")
NAME = os.environ.get("CS_NAME", "PP_CellShading")

# Parametres scalaires : nom -> (defaut, groupe)
SCALARS = [
    ("Intensity", 1.0, "Global"),          # 0 = image d'origine, 1 = cell shading complet
    ("ProtectOverlays", 1.0, "Global"),    # 1 = pixels CustomStencil 200 laisses intacts (convention N&B)
    ("Steps", 4.0, "Paliers"),             # nombre de paliers de lumiere
    ("StepSoftness", 0.15, "Paliers"),     # 0 = marche franche, 1 = degrade
    ("LightPivot", 0.25, "Paliers"),       # luminance du milieu de l'echelle des paliers
    ("Flatten", 0.8, "Couleur"),           # lissage de la photo avant paliers (0..1)
    ("FlattenRadius", 2.0, "Couleur"),     # rayon du lissage en pixels
    ("Detail", 0.25, "Couleur"),           # 0 = aplats purs, 1 = texture d'origine dans chaque palier
    ("ReliefStrength", 0.45, "Relief"),    # 0 = pas d'ombrage toon ; ecart des paliers extremes autour de 1
    ("ReliefSteps", 3.0, "Relief"),        # paliers de l'ombrage (3 = ombre / plat / soleil)
    ("ReliefContrast", 2.0, "Relief"),     # amplification de la pente vis-a-vis du soleil
    ("ReliefRadius", 3.0, "Relief"),
    ("CreaseThreshold", 0.02, "Contours"), # lignes de cretes (laplacien a l'ecart ReliefRadius)
    ("CreaseOpacity", 0.5, "Contours"),       # ecart en pixels pour la normale reconstruite
    ("ReliefFadeDistance", 3000000.0, "Relief"),  # cm
    ("Saturation", 1.15, "Couleur"),       # 1 = inchange
    ("Brightness", 1.0, "Couleur"),        # gain apres paliers
    ("OutlineThickness", 1.0, "Contours"), # en pixels (rendu)
    ("OutlineOpacity", 0.85, "Contours"),
    ("DepthThreshold", 0.04, "Contours"),  # laplacien de 1/profondeur (relatif)
    ("NormalThreshold", 0.35, "Contours"), # 1 - cos(angle)
    ("NormalFadeDistance", 300000.0, "Contours"),  # cm : au-dela, plus de contours de normales
    ("SkyDepth", 1.0e9, "Contours"),       # cm : au-dela = ciel, laisse intact
    ("DebugView", 0.0, "Global"),          # 1 contours, 2 rouge, 3 stencil, 4 ciel, 5 relief, 7 normale reconstruite, 8 verticale
]
VECTORS = [
    ("OutlineColor", unreal.LinearColor(0.02, 0.02, 0.03, 1.0), "Contours"),
    ("FallbackSunDirection", unreal.LinearColor(0.3, 0.2, 0.93, 0.0), "Relief"),  # vers le soleil, si pas de SkyAtmosphere
]

HLSL = r"""
// --- Cell shading (style regie) ---
float3 src = Color.rgb;
if (DebugView > 1.5 && DebugView < 4.5) { return (DebugView > 3.5) ? float3(Depth.r > SkyDepth, 0, 0) : (DebugView > 2.5) ? float3(0, Stencil.r / 255.0, 0) : float3(1, 0, 0); }
float d0 = max(Depth.r, 1.0);
// Overlay protege (trace, bornes km, etiquettes : CustomStencil 200) : couleur d'origine.
// La profondeur custom doit coincider avec celle de la scene : sans r.CustomDepth=3 le stencil
// lu peut contenir n'importe quoi, et un overlay cache derriere le relief n'est pas protege.
if (ProtectOverlays > 0.5 && abs(Stencil.r - 200.0) < 0.5 && CDepth.r <= d0 * 1.001 + 5.0) return src;
const float3 LW = float3(0.2126, 0.7152, 0.0722);

float  t  = max(OutlineThickness, 0.5);
float2 oX = float2(t, 0.0);
float2 oY = float2(0.0, t);

// Contours de profondeur : laplacien de 1/z (nul sur un plan, invariant d'echelle).
// (gardes contre 0 et NaN : certains pixels Cesium ont une normale ou une profondeur nulles)
float dL = max(SceneTextureFetch(PPI_SceneDepth, -oX).r, 1.0);
float dR = max(SceneTextureFetch(PPI_SceneDepth,  oX).r, 1.0);
float dU = max(SceneTextureFetch(PPI_SceneDepth, -oY).r, 1.0);
float dD = max(SceneTextureFetch(PPI_SceneDepth,  oY).r, 1.0);
// Ciel : intact (les degrades du ciel ne supportent pas les paliers). Un pixel « ciel » isole
// au milieu du terrain est une fissure entre tuiles Cesium : on le bouche avec ses voisins.
if (d0 > SkyDepth)
{
    if (max(max(dL, dR), max(dU, dD)) > SkyDepth) return src;
    src = (SceneTextureFetch(PPI_PostProcessInput0, -oX).rgb + SceneTextureFetch(PPI_PostProcessInput0, oX).rgb
         + SceneTextureFetch(PPI_PostProcessInput0, -oY).rgb + SceneTextureFetch(PPI_PostProcessInput0, oY).rgb) * 0.25;
    return src;
}
float lap = abs(d0 / dL + d0 / dR - 2.0) + abs(d0 / dU + d0 / dD - 2.0);
float eDepth = smoothstep(DepthThreshold, DepthThreshold * 2.0, lap);

// Contours de normales, attenues avec la distance (photogrammetrie bruitee au loin).
float3 n0 = Normal.rgb * rsqrt(max(dot(Normal.rgb, Normal.rgb), 1e-8));
float cmin = min(min(dot(n0, SceneTextureFetch(PPI_WorldNormal, -oX).rgb), dot(n0, SceneTextureFetch(PPI_WorldNormal, oX).rgb)),
                 min(dot(n0, SceneTextureFetch(PPI_WorldNormal, -oY).rgb), dot(n0, SceneTextureFetch(PPI_WorldNormal, oY).rgb)));
float nFade = 1.0 - smoothstep(NormalFadeDistance * 0.5, NormalFadeDistance, d0);
float eNormal = smoothstep(NormalThreshold, NormalThreshold * 1.5, 1.0 - cmin) * nFade;
float edge = max(eDepth, eNormal);
edge = (edge == edge) ? saturate(edge) * OutlineOpacity : 0.0;   // NaN -> pas de contour

// Aplat : moyenne de la couleur de scene sur deux anneaux (r et 1.4r en diagonale)
// pour gommer le grain photogrammetrique. L'ombrage du relief est cuit dans la photo :
// on quantifie donc la luminance de la scene (et non un eclairage separe).
float  r  = max(FlattenRadius, 1.0);
float3 cBlur = (src
    + SceneTextureFetch(PPI_PostProcessInput0, float2(-r, 0)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(r, 0)).rgb
    + SceneTextureFetch(PPI_PostProcessInput0, float2(0, -r)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(0, r)).rgb
    + SceneTextureFetch(PPI_PostProcessInput0, float2(-1.4 * r, -1.4 * r)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(1.4 * r, -1.4 * r)).rgb
    + SceneTextureFetch(PPI_PostProcessInput0, float2(-1.4 * r, 1.4 * r)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(1.4 * r, 1.4 * r)).rgb) / 9.0;
float3 c  = lerp(src, cBlur, saturate(Flatten));
float  Lc = max(dot(c, LW), 1e-5);
float  Lb = max(dot(cBlur, LW), 1e-5);

// Paliers sur la luminance floutee (zones stables), espace L/(L+pivot) de la scene HDR pre-exposee ;
// chaque palier a sa valeur centrale, transition douce en fin de palier.
float s  = max(floor(Steps), 1.0);
float sw = max(saturate(StepSoftness), 1e-3);
float pv = max(LightPivot, 1e-3);
float x  = Lb / (Lb + pv) * s;
float fl = floor(x);
float q  = min((fl + 0.5 + smoothstep(1.0 - sw, 1.0, x - fl)) / s, 0.97);
float Lq = pv * q / (1.0 - q) * pow(Lc / Lb, Detail);
float3 styled = c * (Lq / Lc) * Brightness;

// Relief en aplats : eclairage toon N.L avec le soleil de la scene (SkyAtmosphere).
// Les tuiles Cesium n'ecrivent pas de normales utiles dans le GBuffer : la normale est
// reconstruite depuis la profondeur (espace vue), avec un ecart plus large pour lisser.
float3 sun = SunDir.xyz;
if (dot(sun, sun) < 0.25) sun = FallbackSunDirection.xyz;
sun = sun * rsqrt(max(dot(sun, sun), 1e-6));
float3 sunV = mul(sun, (float3x3)ResolvedView.TranslatedWorldToView);
float  rr  = max(ReliefRadius, 1.0);
float2 vuv = GetViewportUV(Parameters);
float2 ndc = (vuv * 2.0 - 1.0) * float2(1.0, -1.0);
float2 stp = 2.0 * rr * ResolvedView.ViewSizeAndInvSize.zw;
float2 pj  = float2(ResolvedView.ViewToClip[0][0], ResolvedView.ViewToClip[1][1]);
float rL = max(SceneTextureFetch(PPI_SceneDepth, float2(-rr, 0)).r, 1.0);
float rR = max(SceneTextureFetch(PPI_SceneDepth, float2( rr, 0)).r, 1.0);
float rU = max(SceneTextureFetch(PPI_SceneDepth, float2(0, -rr)).r, 1.0);
float rD = max(SceneTextureFetch(PPI_SceneDepth, float2(0,  rr)).r, 1.0);
float3 pL = float3((ndc.x - stp.x) * rL / pj.x, ndc.y * rL / pj.y, rL);
float3 pR = float3((ndc.x + stp.x) * rR / pj.x, ndc.y * rR / pj.y, rR);
float3 pU = float3(ndc.x * rU / pj.x, (ndc.y + stp.y) * rU / pj.y, rU);
float3 pD = float3(ndc.x * rD / pj.x, (ndc.y - stp.y) * rD / pj.y, rD);
float3 nV = cross(pR - pL, pU - pD);
nV = nV * rsqrt(max(dot(nV, nV), 1e-12));
float3 p0 = float3(ndc.x * d0 / pj.x, ndc.y * d0 / pj.y, d0);
if (dot(nV, p0) > 0.0) nV = -nV;
float ndl = dot(nV, sunV);
if (DebugView > 6.5 && DebugView < 7.5) return nV * 0.5 + 0.5;
// Ombrage relatif au sol plat local (verticale du globe, centre planete de SkyAtmosphere) :
// un plat garde le palier median, les pentes au soleil s'eclaircissent, les autres s'assombrissent.
float3 pT  = mul(float4(p0, 1.0), ResolvedView.ViewToTranslatedWorld).xyz;
float3 upT = pT - ResolvedView.SkyPlanetTranslatedWorldCenterAndViewHeight.xyz;
upT = (dot(upT, upT) > 1.0) ? normalize(upT) : float3(0, 0, 1);
float3 upV = mul(upT, (float3x3)ResolvedView.TranslatedWorldToView);
if (DebugView > 7.5) return upV * 0.5 + 0.5;
float shade = 0.5 + (ndl - dot(upV, sunV)) * ReliefContrast;
float rs  = max(floor(ReliefSteps), 1.0);
float xr  = saturate(shade) * rs;
float fr  = min(floor(xr), rs - 1.0);
float tr  = saturate((fr + 0.5 + smoothstep(1.0 - sw, 1.0, xr - fr)) / rs);
float relief = 1.0 + (tr - 0.5) * 2.0 * ReliefStrength;
float rFade = 1.0 - smoothstep(ReliefFadeDistance * 0.5, ReliefFadeDistance, d0);
relief = lerp(1.0, relief, rFade);
styled *= relief;

// Lignes de cretes et de talwegs : laplacien de 1/z a l'ecart du relief (plus large que les contours).
float lapR = abs(d0 / rL + d0 / rR - 2.0) + abs(d0 / rU + d0 / rD - 2.0);
float crease = smoothstep(CreaseThreshold, CreaseThreshold * 2.0, lapR) * CreaseOpacity * rFade;
edge = max(edge, crease);
if (DebugView > 4.5) return relief.xxx * 0.5;
if (!(dot(styled, LW) >= 0.0)) styled = src;                      // NaN -> couleur d'origine

if (DebugView > 0.5 && DebugView < 1.5) return lerp(float3(1, 1, 1), OutlineColor.rgb, edge);
// Saturation stylisee.
float Lo = dot(styled, LW);
styled = max(lerp(Lo.xxx, styled, Saturation), 0.0);

// Contours.
styled = lerp(styled, OutlineColor.rgb, edge);
return lerp(src, styled, saturate(Intensity));
"""


def main():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    mel = unreal.MaterialEditingLibrary
    path = f"{PKG_DIR}/{NAME}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mat = unreal.EditorAssetLibrary.load_asset(path)
        mel.delete_all_material_expressions(mat)
    else:
        mat = asset_tools.create_asset(NAME, PKG_DIR, unreal.Material, unreal.MaterialFactoryNew())

    mat.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    # Avant TSR et avant la translucence after-DOF : contours antialiases, etiquettes translucides intactes.
    mat.set_editor_property("blendable_location", unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_DOF)

    def scene_tex(tex_id, y):
        e = mel.create_material_expression(mat, unreal.MaterialExpressionSceneTexture, -900, y)
        e.set_editor_property("scene_texture_id", tex_id)
        return e

    st = unreal.SceneTextureId
    tex = {
        "Color": scene_tex(st.PPI_POST_PROCESS_INPUT0, -300),
        "Depth": scene_tex(st.PPI_SCENE_DEPTH, -150),
        "Normal": scene_tex(st.PPI_WORLD_NORMAL, 0),
        "Stencil": scene_tex(st.PPI_CUSTOM_STENCIL, 150),
        "CDepth": scene_tex(st.PPI_CUSTOM_DEPTH, 300),
    }

    custom = mel.create_material_expression(mat, unreal.MaterialExpressionCustom, -300, 0)
    custom.set_editor_property("code", HLSL)
    custom.set_editor_property("description", "CellShading")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    names = list(tex.keys()) + ["SunDir"] + [n for n, _, _ in SCALARS] + [n for n, _, _ in VECTORS]
    inputs = []
    for n in names:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", n)
        inputs.append(ci)
    custom.set_editor_property("inputs", inputs)

    for n, e in tex.items():
        mel.connect_material_expressions(e, "Color", custom, n)
    sun = mel.create_material_expression(mat, unreal.MaterialExpressionSkyAtmosphereLightDirection, -900, 380)
    mel.connect_material_expressions(sun, "", custom, "SunDir")

    y = 450
    for n, v, g in SCALARS:
        p = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, y)
        p.set_editor_property("parameter_name", n)
        p.set_editor_property("default_value", v)
        p.set_editor_property("group", g)
        mel.connect_material_expressions(p, "", custom, n)
        y += 80
    for n, v, g in VECTORS:
        p = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -900, y)
        p.set_editor_property("parameter_name", n)
        p.set_editor_property("default_value", v)
        p.set_editor_property("group", g)
        mel.connect_material_expressions(p, "", custom, n)
        y += 80

    mel.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    errors = mel.recompile_material(mat)
    for e in errors or []:
        unreal.log_error(f"CELLSHADING_COMPILE_ERROR {e}")
    stats = mel.get_statistics(mat)
    unreal.log(f"CELLSHADING_STATS pixel_instr={stats.num_pixel_shader_instructions} samplers={stats.num_samplers}")
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
    unreal.log(f"CELLSHADING_OK {path} inputs={len(inputs)}")


main()
