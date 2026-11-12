"""Cree (ou recree) le materiau de post-process du style Toon simple (dessin anime peu detaille).

    UnrealEditor-Cmd.exe TrailSimulator.uproject -run=pythonscript -script="<chemin>/create_pp_toonsimple.py"

Sortie : /Game/LTVContent/Materials/Styles/PP_ToonSimple
Pilote par le JSON de style de la regie (data/styles/toon_simple.json), section postprocess.
Contrat : Docs/Styles/CONTRAT_STYLES.md (CustomStencil 200 = overlay protege).

Derive de PP_CellShading (create_pp_cellshading.py), en plus simple et plus graphique :
- details supprimes : filtre de Kuwahara (4 quadrants, 24 lectures) qui gomme texture, herbe,
  flore et grain photogrammetrique en gardant des bords nets entre les zones ;
- couleurs : peu de paliers de luminance (2 par defaut), marches franches, saturation poussee ;
- relief : 2 paliers (ombre / soleil) depuis la normale reconstruite par la profondeur ;
- contours : laplacien de 1/z sur 4 axes (8 voisins) a OutlineThickness pixels : traits epais,
  ronds et nets ; pas de contours de normales (bruites sur Cesium) ;
- rayons et epaisseurs en pixels a 1080 lignes, mis a l'echelle de la resolution de rendu ;
- ciel intact, fissures entre tuiles bouchees, overlays stencil 200 intacts (ProtectOverlays).
Emplacement : Scene Color After DOF (avant TSR, avant la translucence after-DOF).
DebugView : 1 contours, 3 stencil, 4 ciel, 5 relief, 6 aplats sans contours.
"""
import os

import unreal

PKG_DIR = os.environ.get("TS_DIR", "/Game/LTVContent/Materials/Styles")
NAME = os.environ.get("TS_NAME", "PP_ToonSimple")

# Parametres scalaires : nom -> (defaut, groupe)
SCALARS = [
    ("Intensity", 1.0, "Global"),          # 0 = image d'origine, 1 = toon complet
    ("ProtectOverlays", 1.0, "Global"),    # 1 = pixels CustomStencil 200 laisses intacts
    ("SmoothRadius", 6.0, "Aplats"),       # rayon du Kuwahara en pixels (a 1080 lignes)
    ("Steps", 2.0, "Aplats"),              # paliers de luminance
    ("StepSoftness", 0.04, "Aplats"),      # 0 = marche franche, 1 = degrade
    ("LightPivot", 0.3, "Aplats"),         # luminance du milieu de l'echelle des paliers
    ("Saturation", 1.45, "Couleur"),       # 1 = inchange
    ("Brightness", 1.0, "Couleur"),        # gain apres paliers
    ("ReliefStrength", 0.3, "Relief"),     # ecart ombre / soleil autour de 1 (0 = pas d'ombrage)
    ("ReliefContrast", 2.0, "Relief"),     # amplification de la pente vis-a-vis du soleil
    ("ReliefRadius", 5.0, "Relief"),       # ecart en pixels pour la normale reconstruite
    ("ReliefFadeDistance", 3000000.0, "Relief"),  # cm
    ("OutlineThickness", 2.0, "Contours"), # demi-epaisseur en pixels (a 1080 lignes)
    ("OutlineOpacity", 1.0, "Contours"),
    ("DepthThreshold", 0.025, "Contours"),  # laplacien de 1/profondeur (relatif)
    ("CreaseThreshold", 0.05, "Contours"), # grandes cretes seulement
    ("CreaseOpacity", 0.0, "Contours"),
    ("SkyDepth", 1.0e9, "Contours"),       # cm : au-dela = ciel, laisse intact
    ("DebugView", 0.0, "Global"),
]
VECTORS = [
    ("OutlineColor", unreal.LinearColor(0.01, 0.01, 0.015, 1.0), "Contours"),
    ("FallbackSunDirection", unreal.LinearColor(0.3, 0.2, 0.93, 0.0), "Relief"),
]

HLSL = r"""
// --- Toon simple (style regie) ---
float3 src = Color.rgb;
if (DebugView > 1.5 && DebugView < 4.5) { return (DebugView > 3.5) ? float3(Depth.r > SkyDepth, 0, 0) : (DebugView > 2.5) ? float3(0, Stencil.r / 255.0, 0) : float3(1, 0, 0); }
float d0 = max(Depth.r, 1.0);
// Overlay protege (trace, bornes km, etiquettes : CustomStencil 200) : couleur d'origine.
if (ProtectOverlays > 0.5 && abs(Stencil.r - 200.0) < 0.5 && CDepth.r <= d0 * 1.001 + 5.0) return src;
const float3 LW = float3(0.2126, 0.7152, 0.0722);
// Rayons exprimes a 1080 lignes, mis a l'echelle de la resolution de rendu.
float px = ResolvedView.ViewSizeAndInvSize.y / 1080.0;

// Contours : laplacien de 1/z sur 4 axes (nul sur un plan, invariant d'echelle).
float  t  = max(OutlineThickness * px, 0.5);
float  td = t * 0.7071;
float dL = max(SceneTextureFetch(PPI_SceneDepth, float2(-t, 0)).r, 1.0);
float dR = max(SceneTextureFetch(PPI_SceneDepth, float2( t, 0)).r, 1.0);
float dU = max(SceneTextureFetch(PPI_SceneDepth, float2(0, -t)).r, 1.0);
float dD = max(SceneTextureFetch(PPI_SceneDepth, float2(0,  t)).r, 1.0);
// Ciel intact ; un pixel ciel isole dans le terrain = fissure entre tuiles : bouche.
if (d0 > SkyDepth)
{
    if (max(max(dL, dR), max(dU, dD)) > SkyDepth) return src;
    return (SceneTextureFetch(PPI_PostProcessInput0, float2(-t, 0)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(t, 0)).rgb
          + SceneTextureFetch(PPI_PostProcessInput0, float2(0, -t)).rgb + SceneTextureFetch(PPI_PostProcessInput0, float2(0, t)).rgb) * 0.25;
}
float dA = max(SceneTextureFetch(PPI_SceneDepth, float2(-td, -td)).r, 1.0);
float dB = max(SceneTextureFetch(PPI_SceneDepth, float2( td,  td)).r, 1.0);
float dC = max(SceneTextureFetch(PPI_SceneDepth, float2( td, -td)).r, 1.0);
float dE = max(SceneTextureFetch(PPI_SceneDepth, float2(-td,  td)).r, 1.0);
float lap = max(max(abs(d0 / dL + d0 / dR - 2.0), abs(d0 / dU + d0 / dD - 2.0)),
                max(abs(d0 / dA + d0 / dB - 2.0), abs(d0 / dC + d0 / dE - 2.0)));
float edge = smoothstep(DepthThreshold, DepthThreshold * 1.3, lap);

// Aplats : filtre de Kuwahara. Dans chaque quadrant (centre + 6 lectures sur 2 rayons), moyenne et variance ;
// on garde la moyenne du quadrant le plus uniforme : texture et herbe disparaissent,
// les limites entre zones (route / champ, toit / rue) restent nettes.
float  R = max(SmoothRadius * px, 1.0);
float3 best = src;
float  bestV = 3.0e38;
[unroll] for (int q = 0; q < 4; q++)
{
    float2 sg = float2((q & 1) ? 1.0 : -1.0, (q & 2) ? 1.0 : -1.0) * R;
    float3 s1 = src, s2 = src * src, k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(0.5,  0.15)).rgb; s1 += k; s2 += k * k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(0.15, 0.5 )).rgb; s1 += k; s2 += k * k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(0.4,  0.4 )).rgb; s1 += k; s2 += k * k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(1.0,  0.3 )).rgb; s1 += k; s2 += k * k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(0.3,  1.0 )).rgb; s1 += k; s2 += k * k;
    k = SceneTextureFetch(PPI_PostProcessInput0, sg * float2(0.8,  0.8 )).rgb; s1 += k; s2 += k * k;
    float3 m = s1 / 7.0;
    float  v = dot(max(s2 / 7.0 - m * m, 0.0), float3(1, 1, 1)) / max(dot(m, m), 1e-6);
    if (v < bestV) { bestV = v; best = m; }
}
float3 c  = best;
float  Lc = max(dot(c, LW), 1e-5);

// Paliers francs sur la luminance, espace L/(L+pivot) de la scene HDR pre-exposee.
float s  = max(floor(Steps), 1.0);
float sw = max(saturate(StepSoftness), 1e-3);
float pv = max(LightPivot, 1e-3);
float x  = Lc / (Lc + pv) * s;
float fl = floor(x);
float qv = min((fl + 0.5 + smoothstep(1.0 - sw, 1.0, x - fl)) / s, 0.97);
float Lq = pv * qv / (1.0 - qv);
float3 styled = c * (Lq / Lc) * Brightness;

// Relief en 2 paliers (ombre / soleil) : normale reconstruite depuis la profondeur,
// ombrage relatif au sol plat local (verticale du globe SkyAtmosphere).
float3 sun = SunDir.xyz;
if (dot(sun, sun) < 0.25) sun = FallbackSunDirection.xyz;
sun = sun * rsqrt(max(dot(sun, sun), 1e-6));
float3 sunV = mul(sun, (float3x3)ResolvedView.TranslatedWorldToView);
float  rr  = max(ReliefRadius * px, 1.0);
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
float3 pT  = mul(float4(p0, 1.0), ResolvedView.ViewToTranslatedWorld).xyz;
float3 upT = pT - ResolvedView.SkyPlanetTranslatedWorldCenterAndViewHeight.xyz;
upT = (dot(upT, upT) > 1.0) ? normalize(upT) : float3(0, 0, 1);
float3 upV = mul(upT, (float3x3)ResolvedView.TranslatedWorldToView);
float shade = (dot(nV, sunV) - dot(upV, sunV)) * ReliefContrast;
float rFade = 1.0 - smoothstep(ReliefFadeDistance * 0.5, ReliefFadeDistance, d0);
float relief = 1.0 + (smoothstep(-sw, sw, shade) * 2.0 - 1.0) * ReliefStrength * rFade;
styled *= relief;
if (DebugView > 4.5 && DebugView < 5.5) return relief.xxx * 0.5;

// Grandes cretes (optionnel) : laplacien de 1/z a l'ecart du relief.
float lapR = abs(d0 / rL + d0 / rR - 2.0) + abs(d0 / rU + d0 / rD - 2.0);
edge = max(edge, smoothstep(CreaseThreshold, CreaseThreshold * 1.3, lapR) * CreaseOpacity * rFade);
edge = (edge == edge) ? saturate(edge) * OutlineOpacity : 0.0;   // NaN -> pas de contour
if (!(dot(styled, LW) >= 0.0)) styled = src;                      // NaN -> couleur d'origine

// Couleurs saturees.
float Lo = dot(styled, LW);
styled = max(lerp(Lo.xxx, styled, Saturation), 0.0);
if (DebugView > 5.5 && DebugView < 6.5) return styled;
if (DebugView > 0.5 && DebugView < 1.5) return lerp(float3(1, 1, 1), OutlineColor.rgb, edge);

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
    mat.set_editor_property("blendable_location", unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_DOF)

    def scene_tex(tex_id, y):
        e = mel.create_material_expression(mat, unreal.MaterialExpressionSceneTexture, -900, y)
        e.set_editor_property("scene_texture_id", tex_id)
        return e

    st = unreal.SceneTextureId
    tex = {
        "Color": scene_tex(st.PPI_POST_PROCESS_INPUT0, -300),
        "Depth": scene_tex(st.PPI_SCENE_DEPTH, -150),
        "Stencil": scene_tex(st.PPI_CUSTOM_STENCIL, 150),
        "CDepth": scene_tex(st.PPI_CUSTOM_DEPTH, 300),
    }

    custom = mel.create_material_expression(mat, unreal.MaterialExpressionCustom, -300, 0)
    custom.set_editor_property("code", HLSL)
    custom.set_editor_property("description", "ToonSimple")
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
        unreal.log_error(f"TOONSIMPLE_COMPILE_ERROR {e}")
    stats = mel.get_statistics(mat)
    unreal.log(f"TOONSIMPLE_STATS pixel_instr={stats.num_pixel_shader_instructions} samplers={stats.num_samplers}")
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
    unreal.log(f"TOONSIMPLE_OK {path} inputs={len(inputs)}")


main()
