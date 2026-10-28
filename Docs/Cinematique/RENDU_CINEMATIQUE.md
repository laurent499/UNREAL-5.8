# Level cinéma : export vidéo d'un survol de course

Le live (MainMap, OWL SRT) n'est pas touché. Le rendu cinéma passe par un level à part, un Level Sequence et Movie Render Queue (MRQ), sans la contrainte des 25 i/s : chaque image prend le temps qu'il faut.

## Ce que fait le code

- **Trace cinéma** (`ACinematicTrail`) : on lui donne l'id de la course, on clique **Charger la course**, il télécharge `race/<id>/path` depuis la régie (mêmes en-têtes d'accès que le live), recentre le géo-référencement Cesium sur le départ et dessine le tracé avec le même cylindre et le même matériau que le live. Il récupère aussi le setup de la course (`race/<id>`, pour la couleur et le template), ses checkpoints (portés par le tracé) et ses POI (`race/<id>/pois`). Tout est sauvegardé avec le level : la régie n'est plus nécessaire ensuite, sauf pour relire le template au début du rendu (à défaut, les valeurs d'origine s'appliquent).
- **Checkpoints et POI** : dans l'éditeur, de simples repères nommés (orange pour les checkpoints, bleu pour les POI), pour cadrer les plans. Au rendu, ce sont les vrais panneaux du live (Generic, GTWS, UTMB ou Nike selon le template) qui apparaissent, visibles d'emblée et tournés vers la caméra. On les coupe avec `bSpawnCheckpoints` et `bSpawnPois`.
- **Caméra cinéma sur tracé** (`ACinematicTrailCamera`, une CineCamera) : sa position ne dépend que de réglages animables dans Sequencer. `Progress` (0 départ, 1 arrivée), `HeightM`, `BackM`, `SideM`, `OrbitDeg` (0 derrière, 90 côté, 180 face), `LeadM`, `AimAtPivot`, `PitchOffsetDeg`, plus tout le réglage CineCamera (focale, ouverture, mise au point). Pas de lissage d'une image à l'autre : la prévisualisation Sequencer et le rendu sont identiques.
- Plugin **Movie Render Queue** activé dans le `.uproject`.

## À faire à la main (une fois)

1. **Compiler** : récupérer la branche, puis compiler l'éditeur fermé (fait une fois sur ce poste, à refaire si la branche bouge).
2. **Créer le level** : File > New Level > Empty Level, enregistrer sous `/Game/Lvls/CinematicMap`.
3. **Copier le décor depuis MainMap** : ouvrir MainMap, sélectionner dans l'Outliner `CesiumGeoreference`, le tileset Google (Cesium3DTileset), Ultra Dynamic Sky et Ultra Dynamic Weather, les lumières, le PostProcessVolume et le SkyAtmosphere s'il existe. Ctrl+C, rouvrir CinematicMap, Ctrl+V. **Ne pas copier** le RaceManager, le pawn, les acteurs OWL ou la régie.
4. **World Settings** de CinematicMap : GameMode Override = `GM_Trail` (le pawn du live, `TrailDynamicPawn`, est nécessaire aux panneaux des checkpoints et des POI). Le live ne démarre pas pour autant : sans RaceManager dans le level, aucune course n'est chargée. Poser un PlayerStart près du départ (ou aucun : le pawn apparaît à l'origine, qui est le départ).
5. **Placer le tracé** : Place Actors, chercher « Trace cinema », le poser n'importe où. Dans le panneau Détails, mettre `RaceID` (l'id de la course dans la régie) et `MainURL` (la même adresse que le live, par exemple `http://127.0.0.1:5050/regie/2/` pour la régie locale ; `simulacre.ltvprod.cc` ne répondait plus le 27/10), cliquer **Charger la course**. Une notification donne le nom de la course, sa longueur et le nombre de checkpoints et de POI. Si les repères sont trop petits ou trop gros pour ton cadrage, régler `MarkerTextSize`. Ajuster `HeightOffsetM` si le tracé flotte ou s'enfonce, puis **Recalculer**. `TrailColor`, `TrailWidth`, `Glow` règlent l'aspect.
6. **Placer la caméra** : Place Actors, « Camera cinema sur trace », la poser, puis dans Détails mettre `Trail` = le Trace cinéma. Bouger le curseur `Progress` pour vérifier.
7. **Séquence** : Cinematics > Add Level Sequence, `/Game/Cinematics/LS_Survol`. Ajouter la caméra à la séquence (glisser depuis l'Outliner), puis dans sa piste ajouter **Progress** (+ Track > Property) : clé à 0 au début, clé à 1 à la fin. Ajouter un **Camera Cut** sur cette caméra. Durée et cadence : par exemple 60 s à 25 ou 50 i/s. Ne pas ajouter de piste Transform sur cette caméra (la position vient de Progress).
   - Pour varier les plans : clés sur `HeightM`, `OrbitDeg`, `BackM`, la focale. Plusieurs caméras et plusieurs Camera Cuts pour un montage.
   - L'heure d'Ultra Dynamic Sky peut aussi se clé (propriété Time of Day sur l'acteur UDS).

## Rendu

Window > Cinematics > Movie Render Queue, **+ Render**, choisir `LS_Survol`, puis le réglage (Unsaved Config) :

- **Output** : 1920x1080 ou 3840x2160, dossier de sortie.
- **Format** : `.png` ou `.exr` (séquence d'images, le plus sûr), ou Apple ProRes si le plugin est activé.
- **Anti-aliasing** : Spatial Sample Count 1, Temporal Sample Count 8 (flou de mouvement propre), Override Anti Aliasing = None. **Engine Warm Up Count** = 120 ou plus : laisse le temps à Cesium de charger les tuiles du premier plan.
- **Game Overrides** : laisser par défaut.

Puis **Render (Local)**. Le rendu se fait depuis l'éditeur, qui a Internet (Cesium télécharge les tuiles Google). Ne pas lancer le jeu live en même temps.

## Limites connues

- Cesium charge les tuiles en temps réel pendant que MRQ avance image par image. Si une image sort floue ou trouée, augmenter Engine Warm Up Count, le Temporal Sample Count (plus de temps par image), ou baisser `Maximum Screen Space Error` du tileset (plus de détail, plus lent).
- Pas de coureurs : tracé, checkpoints, POI et caméra.
- Les panneaux n'existent qu'au rendu (et en Play In Editor) : pour vérifier leur aspect avant un long rendu, lancer un rendu court de quelques images, ou Play (Simulate ne suffit pas, il faut le pawn).
- La taille des panneaux suit la distance caméra comme en live (réglages Min/Max de la course enregistrés sur ce poste).
- Le cap de la caméra se calcule sur le plan horizontal du départ. Pour une course très longue (plus de 100 km) on peut voir une légère inclinaison loin du départ.

## Nuages de points et modèles géoréférencés (film Mont Blanc)

Le plugin **LiDAR Point Cloud** est activé. Il lit les formats LAS, LAZ et E57. Les OBJ s'importent comme des meshes ; pour les PLY, passer par Blender ou CloudCompare vers OBJ ou FBX.

Fichiers reçus (dossier `cinema/`, en Lambert-93 avec altitudes IGN69) :

| Fichier | Contenu | Points | Emprise | Altitudes |
|---|---|---|---|---|
| `GEOSNAP-Mont_Blanc_2026.laz` | photogrammétrie du sommet (Metashape), en couleur | 55 M | 440 × 350 m | 4 665 à 4 809 m |
| `lidar_HD_nuage.laz` | dalle LiDAR HD IGN, sans couleur | 20 M | 3 × 2 km | 3 523 à 4 807 m |

Le socle rocheux sous la glace (mesures radar) n'est dans aucun de ces fichiers : il faut le demander aux géomètres, de préférence dans le même système (Lambert-93 / IGN69).

### Importer et caler un nuage

1. Glisser le `.laz` dans le Content Browser (par exemple `/Game/Cinematics/MontBlanc/`). L'import du fichier de 55 M points prend quelques minutes. Laisser les réglages par défaut : le plugin recentre le nuage et garde ses coordonnées Lambert d'origine dans l'asset.
2. Glisser l'asset dans CinematicMap (cela crée un LidarPointCloudActor).
3. Place Actors, poser un **Calage Lambert-93**. Dans Détails : `Target` = le nuage, puis cliquer **Caler sur Cesium**. Une notification donne la position trouvée : pour le sommet, environ 6.8654 E, 45.8325 N.
4. Si le nuage flotte ou s'enfonce de quelques dizaines de centimètres par rapport à Cesium (écart entre les géoïdes IGN69 et EGM96), régler `HeightOffsetM`, puis caler de nouveau.
5. **Découper Cesium** masque les tuiles Google sous l'emprise du nuage, qui les remplace. `ClipMarginM` élargit la découpe. À utiliser pour le sommet. Pour la dalle LiDAR HD, ça découperait 3 × 2 km : à voir selon le plan.
6. Si on recharge ensuite une course avec `bCenterGeoreferenceOnStart`, l'origine Cesium bouge : recliquer **Caler sur Cesium**. Le plus simple est de charger la course d'abord et de caler les nuages ensuite.

Pour un mesh (OBJ) au lieu d'un nuage, renseigner `ManualLambert93Origin` : les coordonnées Lambert-93 (est, nord, altitude, en m) du point (0, 0, 0) du mesh, axes X est, Y sud, Z haut, en cm.

### Version documentaire (sans points, sans trou)

Les nuages de points ne sont pas exploitables tels quels à l'antenne : on les transforme en meshes texturés (scripts dans `Tools/Cinematique`, `pip install "laspy[lazrs]" numpy pillow opencv-python-headless`), tous recentrés sur la même origine Lambert-93 `999911 6533232 4779` :

1. `laz_to_mesh.py GEOSNAP-Mont_Blanc_2026.laz glace_sommet --origine 999911 6533232 4779` : sommet au pas de 50 cm + orthophoto du relevé.
2. `recentre_obj.py socle.obj socle_local.obj` : socle rocheux recentré.
3. `terrain_lidar.py lidar_HD_nuage.laz terrain_lidarhd --origine 999911 6533232 4779 --dessous glace_sommet.obj --socle socle_local.obj` : terrain 3 × 2 km au pas de 2 m, texturé avec l'orthophoto IGN (téléchargée), qui passe sous le sommet et sous le socle, plonge sous Google sur son bord, et écrit le contour de découpe Google (`terrain_lidarhd_decoupe.json`).
4. `masque_socle.py socle.obj GEOSNAP-Mont_Blanc_2026.laz masque_socle.png` : emprise du socle pour le décalottage.

Dans CinematicMap : chaque mesh a son Calage Lambert-93 (`ManualLambert93Origin` = l'origine ci-dessus). Le tileset Google utilise `MI_CesiumCinema` (copie découpable du matériau du live). Le contour de découpe est collé dans `ClipOutlineLambert93` d'un Calage, puis **Découper Cesium**. Matériaux : `M_GlaceSommet` (neige du relevé sans ses ombres photo ; réglages CouleurNeige, ForceDetail, FlouOmbres ; translucide au-dessus du socle selon `MPC_Decalottage.Decalottage`), `M_Terrain`, `M_Socle`.

La séquence `LS_MontBlanc` (45 s à 25 i/s) : `Reveal` du Trace cinema de 0 à 1 en 30 s (le tracé se déroule depuis le Nid d'Aigle, `bReverseDirection`), la caméra suit la tête du tracé (`bFollowTrailReveal`) et tourne autour du sommet à la fin. La piste « Material Parameter Collection » (MPC_Decalottage) est en place, mais ses clés sont à poser à la main : + sur la piste > Decalottage, clé 0 à l'image 800, clé 1 à l'image 1050.

### Export propre (documentaire)

**Avant le rendu, dans CinematicMap :**
- Tileset Google (`Cesium3DTileset_0`) : `Forbid Holes` coché (déjà fait), `Preload Siblings` coché (déjà fait). Pour le rendu final, `Maximum Screen Space Error` à 4 (au lieu de 16) : tuiles plus fines, chargement plus long. Revenir à 16 pour travailler dans l'éditeur.
- Trace cinema : bouton **Draper sur le relief** relancé si le tracé ou les meshes ont bougé (meshes dans `DrapeMeshes`).
- Enregistrer le niveau et la séquence.

**Movie Render Queue** (Window > Cinematics > Movie Render Queue) : **+ Render**, choisir `LS_MontBlanc`, carte CinematicMap. Cliquer sur « Unsaved Config » et régler :
1. **Output** : résolution 3840 × 2160 (UHD) ou 1920 × 1080 ; dossier de sortie ; nom `{sequence_name}.{frame_number}`. Laisser « Use Custom Frame Rate » décoché (25 i/s de la séquence).
2. **Format** (+ Setting) : pour l'étalonnage, **EXR 16 bits** ; pour un fichier direct, **Apple ProRes 422 HQ** (plugin « Apple ProRes Media » à activer) ; à défaut, PNG 16 bits. Retirer le JPG par défaut.
3. **Anti-aliasing** : Spatial Sample Count 1, **Temporal Sample Count 16** (lisse le tramage de la glace qui s'efface, le flou de mouvement et les apparitions de tuiles), **Engine Warm Up Count 150**, Render Warm Up Count 64, « Use Camera Cut for Warm Up » **décoché** (la coupe caméra commence à l'image 0).
4. **Game Overrides** : **Game Mode Override = `GM_Trail`** (indispensable : avec le mode de jeu par défaut de Movie Render Queue, il n'y a pas de pawn du live et l'image sort noire). Laisser « Cinematic Quality Settings » et « Flush Grass Streaming » cochés.
5. **Console Variables** (facultatif) : le nom dans la première colonne, la valeur dans la seconde (`r.MotionBlurQuality` / `4`), pas sur la même ligne.
6. Enregistrer ce réglage comme préréglage (« Save as Preset », par exemple `Doc_MontBlanc_UHD`) pour les prochains rendus.

**Rendu** : d'abord un test court (Output > Custom Playback Range, par exemple images 2500 à 2700 pour le décalottage), puis **Render (Local)**. L'éditeur doit avoir Internet (tuiles Google). Les points colorés et les textes orange de l'éditeur ne sortent pas au rendu.

**Montage** : importer la séquence d'images à 25 i/s. Les passes séparées (tracé seul, glace seule) sont possibles avec les « Stencil Layers » de MRQ si le monteur veut les retoucher.

### Le « décalottage »

Idée de mise en scène, à monter quand on aura le socle :

- Glace = nuage GEOSNAP (ou LiDAR HD), socle = nuage ou mesh radar, calés tous les deux avec un Calage Lambert-93.
- Le plugin LiDAR fournit des **Lidar Clipping Volumes** (Place Actors, « Lidar Clipping Volume », mode Clip Inside) : les points de la glace à l'intérieur du volume disparaissent. On anime la position ou l'échelle du volume dans Sequencer pour « soulever » la calotte, de haut en bas ou en tranche.
- Variante : animer l'opacité ou la taille des points du nuage de glace (réglages de rendu de l'acteur) pour un fondu vers le socle.

### Tracé de la voie normale

La course 218 (MONT BLANC) est dans la régie : poser un Trace cinema avec `RaceID` = 218 et cliquer **Charger la course**. Le tracé va du Nid d'Aigle au sommet. Les refuges de Tête Rousse et du Goûter s'afficheront comme panneaux dès qu'ils seront saisis dans la régie, en checkpoints (sur le tracé) ou en POI.
