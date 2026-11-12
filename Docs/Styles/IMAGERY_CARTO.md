# Section `imagery` : fond cartographique (preset Carto)

Handler : `UCartoImagerySubsystem` (module Subsystems), enregistré auprès de `UStyleSubsystem`.

## Principe

- Les tuiles photo Google ne reçoivent **jamais** la carte : les conditions Google Maps Platform
  interdisent de mélanger leur contenu avec une carte non Google, et changer le matériau d'un
  tileset Cesium le recharge entièrement.
- En Carto, un tileset **Cesium World Terrain** (ion 1, transitoire, tag `LTV_StyleTerrain`) est créé
  avec la carte en overlay `Overlay0` (matériau Cesium par défaut). Il charge masqué ; les tilesets
  photo restent à l'antenne jusqu'à ce qu'il soit chargé (≥ 99 %, `swapTimeoutS` max), puis ils sont masqués.
- Retour en Réaliste : photos réaffichées (encore en cache en mode `hide`, donc instantané), relief détruit.
  Rien ne reste dans le monde.
- Les collisions restent celles des photos (masquées mais présentes). Le drape du tracé (`Path.cpp`)
  ignore le relief de style : le cache de drape garde les hauteurs Google.

## Clés

```json
"imagery": {
  "source": "mapbox",              // mapbox | osm | url
  "style": "mapbox/outdoors-v12",  // mapbox : compte/style
  "retina": false,                 // mapbox : tuiles @2x (1024 px), 4x plus lourdes
  "url": "https://.../{z}/{x}/{y}.png", // source url ; {y} XYZ (nord), "yUp": true pour TMS ; {mapboxToken} remplacé par la clé locale
  "tileSize": 256, "maxLevel": 19,  // source url
  "photoTiles": "hide",            // hide (photos continuent de charger, retour instantané) | suspend (économie réseau, retour = rechargement)
  "terrainAssetId": 1,             // asset ion du relief
  "terrainOffsetM": 0,             // décalage vertical du relief (m), si le tracé s'y enfonce sur les crêtes
  "maximumScreenSpaceError": 2,    // netteté de la carte (plus bas = plus net, plus de tuiles)
  "swapTimeoutS": 8,               // attente max du relief avant bascule (0 = immédiat)
  "attribution": "© Mapbox © OpenStreetMap" // mention à afficher à l'antenne (loguée côté Unreal)
}
```

## Clé Mapbox

Jamais dans le JSON ni dans git. Lue au moment d'appliquer le style, dans l'ordre :
1. variable d'environnement `LTV_MAPBOX_TOKEN` ;
2. fichier `%LOCALAPPDATA%\LTV\mapbox_token.txt` (la clé seule).

Sans clé, le style est refusé (log `[Carto]`) et le rendu photo reste à l'antenne.
L'URL contenant la clé n'est jamais loguée par le projet ; Cesium peut l'écrire dans le log
en cas d'échec de tuile (clé publique `pk.` de toute façon).

## Limites connues

- OSM (`tile.openstreetmap.org`) : serveur communautaire, tests seulement, pas d'usage antenne.
- Passer d'un style carto à un autre recharge la carte (bref relief blanc pendant le chargement).
- World Terrain est plus lisse que Google : le tracé drapé sur Google peut flotter en forêt
  ou s'enfoncer sur les crêtes ; `terrainOffsetM` négatif (ex. -3) aide pour les crêtes.
- Le tracé additif ressort le mieux sur `carto_dark` / `carto_outdoors`.
