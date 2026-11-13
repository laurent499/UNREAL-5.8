# Presets Noir et blanc, Sépia, Duotone

Un seul matériau, `/Game/LTVContent/Materials/Styles/PP_Grade`, piloté par paramètres via la
section `postprocess` du contrat styles v1 (voir `CONTRAT_STYLES.md`). Presets régie :
`data/styles/noir_et_blanc.json`, `sepia.json`, `duotone.json`.

Le matériau est généré par `Tools/Styles/create_pp_grade.py` (script Python éditeur,
relançable : il vide et reconstruit le graphe). Ne pas l'éditer à la main sans reporter
la modif dans le script.

## Paramètres

| Nom | Type | Défaut | Effet |
|---|---|---|---|
| `Amount` | scalaire | 1 | Mélange avec l'image d'origine (0 = Réaliste, 1 = effet plein). Sert aussi de fondu. |
| `Saturation` | scalaire | 0 | 0 = monochrome, 1 = couleurs d'origine (avec le contraste appliqué). |
| `Contrast` | scalaire | 1 | Contraste en espace log autour de `Pivot` (1 = neutre, 1.1–1.3 = punchy). |
| `Brightness` | scalaire | 1 | Multiplicateur de luminance. |
| `Pivot` | scalaire | 0.18 | Gris moyen, point fixe du contraste. |
| `ProtectOverlays` | scalaire | 1 | 1 = les pixels CustomStencil 200 restent intacts. |
| `ShadowTint` | couleur | blanc | Teinte des ombres. |
| `HighlightTint` | couleur | blanc | Teinte des hautes lumières. |

Les deux teintes sont normalisées en luminance dans le shader : seule leur teinte compte,
elles ne changent pas la luminosité. Blanc/blanc = N&B pur ; même teinte des deux côtés =
virage (sépia) ; deux teintes différentes = duotone. Ne pas utiliser `#000000`.

Le `weight` du volume n'est pas un fondu (UE active ou non un blendable matériau) : pour un
fondu ou un N&B partiel, jouer sur `Amount`.

## Point d'insertion et lisibilité des overlays

Blendable Location = **Scene Color Before DOF** (« avant translucence »). Conséquences :

| Élément | Matériau | Rendu sous N&B | Pourquoi |
|---|---|---|---|
| Terrain Cesium, ciel/atmosphère UDS, brouillard, nuages volumétriques, eau | opaque | N&B | passe avant la translucence |
| Textes (noms, km, altitudes...) | `M_RawText` translucide After Motion Blur | **couleur** | composé après le PP, rien à faire |
| Cartes, fonds translucides (`M_Master*`, MI_Black/White...) | translucide | **couleur** | composé après le PP |
| Halo du tracé (`M_Glow`, `M_GlowFilament`) | additif | **couleur** | composé après le PP |
| Tracé (`TrailMaterialLum/Opaque`), panneaux km et fonds `M_PlainRounded`, pictos, drapeaux, photos | masqué | **couleur si stencil 200** | passe opaque : protégé par `CustomStencil = 200` |

La convention stencil 200 du contrat est donc **validée, en complément** du point
d'insertion : elle n'est nécessaire que pour les primitives opaques/masquées. Les translucides
n'écrivent de toute façon pas dans le stencil.

Bloom et halos sont calculés après : la lueur orange du tracé reste orange.

## Coût

- Une passe plein écran, ~20 ALU + 2 lectures de texture : ~0,05–0,1 ms GPU en 1080p.
- `r.CustomDepth=3` : la passe CustomDepth dessine en plus les primitives protégées (tracé,
  bornes, panneaux), quelques dizaines de draw calls ; les coureurs/checkpoints/POI l'écrivent
  déjà (occlusion des textes `M_RawText`).
- Rien au thread de jeu par image. Le matériau est à précharger (le cadre le charge en
  asynchrone au premier `style set`) ; un premier passage en N&B peut compiler le shader
  si le DDC est froid : faire un essai hors antenne après chaque build.

## Points de vigilance

- `Cloud_Fog_PostProcess` d'UDS est en **Scene Color After DOF**, donc après PP_Grade : s'il
  est actif, sa teinte de brouillard nuageux repasserait en couleur. Il n'est pas référencé
  par MainMap aujourd'hui ; à surveiller si on active « Cloud Fog » dans UDS.
- `M_CesiumPostProcess` (Translucency After DOF) lit aussi `CustomStencil` : vérifier en jeu
  qu'il ne réagit pas à la valeur 200.
- Marquer le **tracé** en CustomDepth le fait entrer dans l'occlusion des textes
  `M_RawText` : un texte situé derrière le tracé à l'écran serait masqué en N&B (pas en
  Réaliste). Effet attendu marginal (le tracé est au sol, les textes au-dessus).
- `M_PlainRounded` (panneaux km) : masqué, donc à protéger via la classe `Km`.
