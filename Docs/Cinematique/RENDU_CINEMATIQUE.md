# Level cinéma : export vidéo d'un survol de course

Le live (MainMap, OWL SRT) n'est pas touché. Le rendu cinéma passe par un level à part, un Level Sequence et Movie Render Queue (MRQ), sans la contrainte des 25 i/s : chaque image prend le temps qu'il faut.

## Ce que fait le code

- **Trace cinéma** (`ACinematicTrail`) : on lui donne l'id de la course, on clique **Charger la course**, il télécharge `race/<id>/path` depuis la régie (mêmes en-têtes d'accès que le live), recentre le géo-référencement Cesium sur le départ et dessine le tracé avec le même cylindre et le même matériau que le live. Les points sont sauvegardés avec le level : plus besoin de la régie ensuite, ni pendant le rendu.
- **Caméra cinéma sur tracé** (`ACinematicTrailCamera`, une CineCamera) : sa position ne dépend que de réglages animables dans Sequencer. `Progress` (0 départ, 1 arrivée), `HeightM`, `BackM`, `SideM`, `OrbitDeg` (0 derrière, 90 côté, 180 face), `LeadM`, `AimAtPivot`, `PitchOffsetDeg`, plus tout le réglage CineCamera (focale, ouverture, mise au point). Pas de lissage d'une image à l'autre : la prévisualisation Sequencer et le rendu sont identiques.
- Plugin **Movie Render Queue** activé dans le `.uproject`.

## À faire à la main (une fois)

1. **Compiler** : récupérer la branche, puis compiler l'éditeur fermé (fait une fois sur ce poste, à refaire si la branche bouge).
2. **Créer le level** : File > New Level > Empty Level, enregistrer sous `/Game/Lvls/CinematicMap`.
3. **Copier le décor depuis MainMap** : ouvrir MainMap, sélectionner dans l'Outliner `CesiumGeoreference`, le tileset Google (Cesium3DTileset), Ultra Dynamic Sky et Ultra Dynamic Weather, les lumières, le PostProcessVolume et le SkyAtmosphere s'il existe. Ctrl+C, rouvrir CinematicMap, Ctrl+V. **Ne pas copier** le RaceManager, le pawn, les acteurs OWL ou la régie.
4. **World Settings** de CinematicMap : GameMode Override = `GameModeBase`, pour que le live (RaceManager, HTTP, OWL) ne démarre pas pendant le rendu.
5. **Placer le tracé** : Place Actors, chercher « Trace cinema », le poser n'importe où. Dans le panneau Détails, mettre `RaceID` (l'id de la course dans la régie), cliquer **Charger la course**. Une notification donne le nombre de points et la longueur. Ajuster `HeightOffsetM` si le tracé flotte ou s'enfonce, puis **Recalculer**. `TrailColor`, `TrailWidth`, `Glow` règlent l'aspect.
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
- Pas de coureurs ni de checkpoints pour l'instant : seulement le tracé et la caméra.
- Le cap de la caméra se calcule sur le plan horizontal du départ. Pour une course très longue (plus de 100 km) on peut voir une légère inclinaison loin du départ.
