# Preset Dessin animé (`toon_simple`)

Rendu dessin animé peu détaillé : grands aplats de couleurs saturées, 2 tons, gros contours
noirs, textures, herbe et végétation gommées. Plus simple et plus graphique que le preset
Cell shading (`PP_CellShading`), dont il reprend la base (relief depuis la profondeur, ciel
intact, fissures bouchées, protection des overlays).

- Matériau : `/Game/LTVContent/Materials/Styles/PP_ToonSimple`, généré par
  `Tools/Styles/create_pp_toonsimple.py` (relançable, ne pas éditer le graphe à la main).
- Preset régie : `data/styles/toon_simple.json` (section `postprocess`, `protectOverlays: true`).
- Blendable Location : Scene Color After DOF (comme Cell shading).
- Coût : ~0,10 ms GPU en 1920x1080 (RTX 5070 Ti, trace Insights du 12/11/2026, pire cas
  avec le ciel traité aussi). Négligeable pour tenir 25 i/s.
- Réaliste inchangé : aucun volume ni matériau hors de ce style.

## Ce qui change par rapport au Cell shading

| | Cell shading | Dessin animé |
|---|---|---|
| Lissage | flou 9 points, 2 px | filtre de Kuwahara 24 points, 6 px : les zones deviennent des aplats aux bords nets |
| Paliers de lumière | 4 (+ détail de la photo) | 2, marches franches, aucun détail réinjecté |
| Relief | 3 paliers | 2 (ombre / soleil) |
| Contours | 1 px, profondeur + normales | 2 px de demi-épaisseur, profondeur sur 4 axes (traits ronds) ; pas de normales (bruitées sur Cesium) |
| Couleurs | saturation 1,15 | saturation 1,45 |
| Échelle | en pixels de rendu | rayons exprimés à 1080 lignes, mis à l'échelle de la résolution |

## Paramètres (section `scalars` du JSON)

| Nom | Défaut | Effet |
|---|---|---|
| `Intensity` | 1 | 0 = image d'origine, 1 = effet plein |
| `ProtectOverlays` | 1 | pixels CustomStencil 200 laissés intacts |
| `SmoothRadius` | 6 | rayon du Kuwahara (px à 1080 l.). Plus grand = moins de détails, mais taches au-delà de ~9 |
| `Steps` | 2 | paliers de luminance (3 = plus de nuances, plus « camouflage » sur la photogrammétrie) |
| `StepSoftness` | 0.04 | 0 = marche franche, 1 = dégradé |
| `LightPivot` | 0.3 | luminance du milieu de l'échelle des paliers |
| `Saturation` | 1.45 | 1 = couleurs d'origine |
| `Brightness` | 1 | gain après paliers |
| `ReliefStrength` | 0.3 | force de l'ombrage toon du relief (0 = aucun) |
| `ReliefContrast` | 2 | sensibilité de l'ombrage à la pente |
| `ReliefRadius` | 5 | écart (px) pour la normale reconstruite |
| `OutlineThickness` | 2 | demi-épaisseur des contours (px à 1080 l.) |
| `OutlineOpacity` | 1 | opacité des contours |
| `DepthThreshold` | 0.025 | seuil des contours : 0.01 = traits sur chaque maison (trop chargé), 0.05 = silhouettes seules |
| `CreaseThreshold` / `CreaseOpacity` | 0.05 / 0 | lignes de crêtes en plus (coupées par défaut) |
| `OutlineColor` (vecteur) | `#0A0A0FFF` | couleur des contours |
| `DebugView` | 0 | 1 contours, 3 stencil, 4 ciel, 5 relief, 6 aplats sans contours |

## Limites connues

- La photogrammétrie Google n'a pas d'objets séparés : en ville, les maisons deviennent des
  blocs clairs cernés de noir, pas des formes « dessinées ». C'est le plus simple qu'on puisse
  tirer d'un post-process.
- La végétation (flore, herbe) est lissée par le filtre, pas retirée. La couper vraiment
  demanderait une section de style dédiée côté C++ (ex. densité de flore à 0).
- Le ciel UDS reste réaliste (nuages volumétriques), comme dans le Cell shading.
