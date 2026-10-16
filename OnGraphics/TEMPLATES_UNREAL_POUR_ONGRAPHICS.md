# Templates Unreal (TrailSimulator) servis par OnGraphics

Document de passation pour la session Claude qui travaille sur **OnGraphics**.
Il décrit ce qu'Unreal sait afficher, le JSON qu'il attend, et comment l'obtenir
depuis une maquette Figma avec la mécanique d'import qu'OnGraphics a déjà
(`figma-import.js`, `template.schema.json`, calques `#<id>`).

Rédigé côté Unreal le 15/10/2026. Les choix de Laurent sont récapitulés au §7
(tous tranchés).

---

## 1. Le contexte en deux minutes

- **TrailSimulator** (Unreal 5.8) affiche une course en 3D sur la carte, diffusée en
  direct (OWL SRT, 25 fps à tenir absolument). Trois éléments graphiques y sont
  habillés par un template :
  - **Checkpoint** : un empilement vertical posé sur un pied (picto principal,
    bandeau du nom, bandeau d'infos distance et altitude, filet, bandeau météo, filet).
  - **POI** : même empilement, sans la distance.
  - **Coureur** : un cartouche au-dessus du coureur (barre de couleur, photo,
    bandeau du nom, bandeau du club avec drapeau) posé sur un pied.
- La **régie** (app Python FastAPI, port 5050, dépôt `laurent499/UNREAL`) sert les
  données de course à Unreal. Chaque course a un champ `templateName`.
- Depuis le 14/10/2026, la régie a une page **Templates** et sert
  `GET /template/{name}`. Unreal lit cette route au chargement de la course,
  **avant** de faire apparaître checkpoints, POI et coureurs, et la relit sur la
  commande `group=template&action=reload&name=…` (bouton « Appliquer dans Unreal »).
- Rien du template ne se recalcule à chaque frame : il s'applique une fois à
  l'apparition, puis à chaque rechargement.

**Ce que Laurent veut** : OnGraphics devient la source des templates (créés depuis
Figma), expose le template Unreal en JSON sur un endpoint, et la régie le récupère
pour le servir à Unreal.

```
Figma ──(import OnGraphics)──> template OnGraphics ──GET /unreal/template/{nom}──> régie ──GET /template/{nom}──> Unreal
```

---

## 2. Réponse à « conventions Figma ou JSON de template ? »

**Les deux, chacun à sa place** (recommandation) :

1. **Côté Figma** : OnGraphics a déjà la bonne mécanique (un lien Figma par
   graphic, des calques `#<id>` lus comme fonds et textes, une palette à jetons).
   Il suffit de déclarer **trois graphics de plus** dans le schéma (checkpoint,
   POI, coureur Unreal) avec leurs parties nommées. La maquette Figma n'a alors
   rien de spécial à savoir sur Unreal : un composant par élément, des calques
   bien nommés.
2. **Côté Unreal** : OnGraphics **ne renvoie pas son template brut** (il est
   fait pour du CSS : sélecteurs, écarts à l'origine, jetons). Il expose un JSON
   **déjà traduit** au format Unreal décrit ci-dessous, couleurs résolues en hexa.
   Unreal n'a ainsi aucune logique OnGraphics à connaître, et le format reste stable
   si le schéma OnGraphics évolue.

Le JSON de template « à la main » reste possible (c'est ce que fait l'éditeur de
la régie), mais il ne profite pas de l'import Figma.

---

## 3. Le contrat : `GET /unreal/template/{nom}`

### 3.1 Routes attendues sur OnGraphics

| Route | Réponse |
|---|---|
| `GET /unreal/templates` | `{ "templates": [ { "name": "TEMPLIERS", "updatedAt": "2026-10-15T07:00:00Z" } ] }` : les templates qui ont un habillage Unreal |
| `GET /unreal/template/{nom}` | le template au format ci-dessous, `200` ; `404` si inconnu |

- Le **nom** est celui que l'on met dans `templateName` du JSON de course :
  lettres, chiffres, `-` et `_`, 40 caractères au plus. Pas d'espace ni d'accent :
  il sert aussi de nom de fichier côté régie.
  **Décidé** : c'est le même nom que le template OnGraphics de l'épreuve (une
  épreuve = un nom pour vMix et pour Unreal).
- Pas d'authentification aujourd'hui entre la régie et Unreal (réseau local).
  Si OnGraphics en exige une, le dire : la régie l'ajoutera.

### 3.2 Format (version 1)

Tous les champs sont facultatifs : un champ absent prend la valeur d'origine
indiquée. La correspondance des noms de champs ignore la casse.

```json
{
  "version": 1,
  "base": "GTWS",
  "globals": {
    "primary": "#FF3C00FF",
    "secondary": "#FFFFFFFF",
    "accent": "race",
    "textLight": "#FFFFFFFF",
    "textDark": "#000000FF",
    "cornerRadius": 0.0,
    "slant": 0.0,
    "pictoSet": "GTWS"
  },
  "checkpoint": {
    "foot": "race",
    "mainPicto": "#FFFFFFFF",
    "nameBkg": "#FF3C00FF",
    "nameBorder": "#FF3C00FF",
    "nameText": "#FFFFFFFF",
    "infosBkg": "#FFFFFFFF",
    "infosText": "#000000FF",
    "infosPicto": "#000000FF",
    "line1": "race",
    "weatherBkg": "#FFFFFFFF",
    "weatherText": "#000000FF",
    "line2": "race"
  },
  "poi": { "...": "mêmes clés que checkpoint" },
  "runner": {
    "foot": "race",
    "nameBkg": "#FFFFFFFF",
    "nameText": "#000000FF",
    "clubBkg": "#FF3C00FF",
    "clubText": "#FFFFFFFF",
    "slash": "#98BF16FF",
    "showPhoto": true,
    "showClub": true,
    "showFlag": true,
    "bkgLeftMargin": 250.0,
    "bkgRightMargin": 50.0,
    "height": 200.0
  }
}
```

### 3.3 Règles de valeur

- **Couleur** : `#RRGGBBAA` en sRGB (8 chiffres, l'alpha compte), **ou** la chaîne
  `"race"` qui veut dire « la couleur de la course » (chaque course a sa couleur,
  connue seulement d'Unreal). C'est le seul jeton qu'Unreal résout :
  `primary`, `accent`, `ink`, `paper`… doivent arriver **déjà remplacés par leur hexa**.
- **`base`** : mise en page prédéfinie, c'est-à-dire la famille de classes Unreal à
  faire apparaître : `"Generic"`, `"GTWS"`, `"UTMB"` ou `"Nike"`. La géométrie
  (meshes, empilement, tailles) vient de cette base ; le template n'en règle que
  l'habillage. Pour un template issu de Figma : **`"GTWS"`** (cartouche coureur
  nom + club, empilement le plus proche des graphics OnGraphics).
- **`globals`** : surtout informatif. Unreal lit les couleurs **par élément** ; les
  globales servent de valeurs par défaut dans l'éditeur de la régie. Envoyer quand
  même la palette, elle sera affichée.
- **`pictoSet`** : nom d'un dossier de pictos **déjà présent dans le projet Unreal**
  (`/Game/LTVContent/2D/Pictos/<pictoSet>/`, un picto par type de checkpoint ou de
  POI, plus `loc` et `peak`). Existants : `Generic`, `GTWS`. Unreal ne sait pas
  encore charger un picto depuis une URL (voir §5).
- **`cornerRadius`** (0 à 1, relatif à la hauteur du bandeau) et **`slant`**
  (biseau en degrés, positif = bord incliné « / ») : stockés et affichés dans
  l'aperçu de la régie, **pas encore rendus dans Unreal** (le matériau de fond
  doit être modifié). Les envoyer quand même.
- **`runner.height`** : hauteur du cartouche au-dessus du coureur, en unités Unreal
  (100 Generic, 200 GTWS, 230 UTMB, 250 Nike).
- **`runner.bkgLeftMargin` / `bkgRightMargin`** : marges des bandeaux nom et club
  autour du texte, en unités Unreal (250 à gauche côté photo, 50 à droite pour
  GTWS). **Décidé : lues dans Figma dès la v1**, à titre de test (voir §4.4).
- **`showPhoto` / `showClub` / `showFlag`** : présence de l'élément **dans le
  design**. L'opérateur peut toujours le masquer en direct ; il ne peut pas
  l'afficher si le design l'a retiré.

### 3.4 Ce qu'Unreal ne lit pas (inutile de l'envoyer)

Polices, graisses, tailles et casse des textes, positions, échelles, animations,
libellés. Les textes 3D d'Unreal (TextRender) utilisent des polices importées dans
le projet à la compilation ; une police Figma ne peut pas s'y appliquer au vol.

---

## 4. Ce qu'il faut dans Figma (et dans le schéma OnGraphics)

### 4.1 Trois graphics de plus

Dans `public/blueprints/gtws/template.schema.json` (le modèle Figma en hérite),
ajouter trois familles. Elles n'ont pas de page d'antenne vMix : leur `sel`
peut viser un faux markup d'aperçu (voir §4.5).

| Famille (`family`) | Composant Figma (suggestion de nom) |
|---|---|
| `unreal-checkpoint` | `Unreal / Checkpoint` |
| `unreal-poi` | `Unreal / POI` |
| `unreal-runner` | `Unreal / Coureur` |

Un lien Figma par famille, comme pour les autres graphics (carte « Maquette
Figma » de sa section). Lecture **par parties** : aucune lecture dédiée n'est
nécessaire, sauf pour les variantes du coureur (§4.3).

### 4.2 Calques nommés : checkpoint et POI

Du haut vers le bas, comme dans Unreal. Chaque nom correspond à une clé du JSON.

| Calque Figma | Type | Clé Unreal | Remarque |
|---|---|---|---|
| `#mainPicto` | picto (instance ou vecteur) | `mainPicto` | seule sa **couleur** est lue |
| `#nameBkg` | rectangle / frame | `nameBkg` | fond du bandeau du nom ; son **arrondi** donne `cornerRadius` (rayon ÷ hauteur) |
| `#nameBorder` | rectangle (filet sous le nom) | `nameBorder` | utilisé par la base Generic ; GTWS l'ignore |
| `#nameText` | texte | `nameText` | couleur du texte |
| `#infosBkg` | rectangle | `infosBkg` | bandeau distance / altitude |
| `#infosText` | texte | `infosText` | |
| `#infosPicto` | picto | `infosPicto` | couleur des pictos distance et altitude |
| `#line1` | rectangle fin | `line1` | filet sous les infos |
| `#weatherBkg` | rectangle | `weatherBkg` | bandeau météo |
| `#weatherText` | texte | `weatherText` | |
| `#line2` | rectangle fin | `line2` | filet sous la météo |
| `#foot` | rectangle vertical | `foot` | le pied |

**Couleur de la course (décidé)** : une partie peinte avec la **variable Figma
« Couleur Primaire »** prend la couleur de la course dans Unreal : OnGraphics
écrit `"race"` pour cette partie (c'est l'équivalent du « Sync OnAir » d'OnGraphics,
où la primaire suit la couleur de la course). Toute autre couleur est envoyée
en hexa.

Point technique : l'API REST ne donne le **nom** d'une variable que sur un plan
Enterprise, mais elle donne sur tous les plans l'**id** de la variable liée à un
remplissage (`boundVariables.fills[].id`). Pour reconnaître « Couleur Primaire »
sans le nom :
- un calque repère `#race` dans le fichier (une pastille peinte avec la variable)
  donne son id, et toute partie liée au même id devient `"race"` ;
- à défaut de repère, une partie dont la couleur résolue est égale à la valeur de
  « Couleur Primaire » (la primaire lue par l'import) devient `"race"`. Moins sûr :
  une couleur identique posée à la main serait aussi prise pour la course.

### 4.3 Calques nommés : coureur

| Calque Figma | Type | Clé Unreal |
|---|---|---|
| `#slash` | rectangle (barre de couleur à gauche) | `slash` (base Generic) |
| `#photo` | image | présence → `showPhoto` |
| `#nameBkg` | rectangle | `nameBkg` |
| `#nameText` | texte | `nameText` |
| `#clubBkg` | rectangle | `clubBkg` |
| `#clubText` | texte | `clubText` ; présence → `showClub` |
| `#flag` | image | présence → `showFlag` |
| `#foot` | rectangle vertical | `foot` |

Les noms sont volontairement proches de ceux du synthé vMix (`#name`, `#club`,
`#flag`, `#photo`) : une maquette peut décliner le synthé en cartouche 3D.
Un **jeu de variantes** (avec / sans photo, avec / sans club) se lit comme pour le
synthé : la variante retenue dit ce qui est présent dans le design.

### 4.4 Ce que la maquette ne doit pas chercher à régler

- **Les dimensions** : la géométrie vient de la base Unreal (meshes). Un bandeau
  plus haut dans Figma ne le sera pas dans Unreal.
- **Les polices et la typographie** (§3.4).

**Marges du cartouche coureur (décidé : dès la v1, à tester).** Dans le composant
`Unreal / Coureur` :
- `bkgLeftMargin` = distance entre le bord gauche de `#nameBkg` et le début de
  `#nameText` (la zone de la photo comprise) ;
- `bkgRightMargin` = distance entre la fin de `#nameText` et le bord droit de
  `#nameBkg` (avec un texte à largeur automatique, c'est le padding droit) ;
- conversion en unités Unreal : multiplier les pixels par un facteur
  `UNREAL_UNITS_PER_PX`, à garder en **constante réglable** dans OnGraphics.
  Valeur de départ : **3,2** (le nom du coureur GTWS fait 90 unités de haut pour un
  texte d'environ 28 px). Il sera calibré au premier essai dans Unreal : la
  maquette GTWS doit redonner à peu près 250 et 50.

### 4.5 Aperçu dans l'éditeur OnGraphics (facultatif mais utile)

L'aperçu OnGraphics est une vraie page d'antenne. Pour voir les trois éléments
Unreal, le plus simple est un petit markup HTML/CSS qui les imite (empilement et
cartouche), dans `template-demo.js`, avec les classes visées par les `sel` des
nouvelles familles. La régie a déjà une imitation de ce genre
(`public/templates.html` du dépôt UNREAL, fonctions `stackPreview` et
`runnerPreview`), réutilisable comme point de départ. L'aperçu reste approximatif :
le rendu exact se vérifie dans Unreal avec « Appliquer dans Unreal ».

### 4.6 Traduction template OnGraphics vers JSON Unreal

À faire dans un module serveur (par exemple `unreal-export.js`) appelé par
`GET /unreal/template/{nom}` :

1. Charger le template OnGraphics (écarts) **fusionné avec les valeurs d'origine**
   du schéma, pour avoir chaque couleur effective.
2. Pour chaque partie des trois familles, prendre le fond (`surfaces[].bg`) ou la
   couleur de texte (`texts[].color`) effective ; résoudre les jetons (`accent`,
   `ink`, `paper`…) avec la palette effective (celle de la famille si elle en a
   une, sinon la globale) ; écrire `#RRGGBBAA` ; garder `"race"` tel quel.
3. `globals.primary` ← `accent`, `globals.secondary` ← `paper`,
   `globals.textDark` ← `ink`, `globals.textLight` ← `paper` (informatif).
4. `cornerRadius` ← forme `round` : rayon ÷ hauteur du bandeau du nom (0 à 1) ;
   `slant` ← forme `bevel` : son angle ; sinon 0.
5. `base` ← `"GTWS"` ; `pictoSet` ← `"GTWS"` tant qu'Unreal ne charge pas de picto
   par URL.
6. Options booléennes de la famille coureur ← `showPhoto`, `showClub`, `showFlag`.
7. Parties liées à la variable « Couleur Primaire » ← `"race"` (§4.2).
8. `runner.bkgLeftMargin` / `bkgRightMargin` ← marges lues × `UNREAL_UNITS_PER_PX` (§4.4).

Test simple : le template OnGraphics `GTWS` d'origine doit redonner, à la
couleur près, l'exemple du §3.2 (c'est l'habillage GTWS actuel d'Unreal).

---

## 5. Limites actuelles côté Unreal (ce qui viendra plus tard)

| Sujet | Aujourd'hui | Plus tard |
|---|---|---|
| Arrondi, biseau | stockés, rendus seulement dans l'aperçu de la régie | matériau de fond paramétrable |
| Pictos | dossiers du projet (`pictoSet`) | possible : PNG exportés par OnGraphics, téléchargés par Unreal comme les photos des coureurs (ajouterait `pictoBaseUrl`) |
| Bases UTMB et Nike | mise en page et hauteur seulement, couleurs ignorées | à brancher si besoin |
| Polices | non | demanderait des polices importées dans le projet, choisies par nom |

---

## 6. Côté régie (fait par la session Unreal, une fois l'endpoint en place)

- Un réglage « URL OnGraphics » dans les paramètres de la régie.
- `GET /template/{nom}` : la régie demande d'abord à OnGraphics, garde une copie
  locale (pour les directs sans OnGraphics joignable) et, à défaut, sert ses
  templates d'origine (`GTWS`, `Generic`).
- La liste du champ Template des courses inclura les templates d'OnGraphics.
- **Décidé** : dans l'éditeur de la régie, les templates venus d'OnGraphics sont
  en **lecture seule** (on les retouche dans Figma ou dans OnGraphics) ; les
  templates locaux restent modifiables.

---

## 7. Choix de Laurent (15/10/2026)

1. Même nom de template pour vMix et Unreal : **oui**.
2. Couleur de la course : les parties peintes avec la **variable Figma « Couleur Primaire »** (§4.2).
3. Éditeur de la régie : **lecture seule** pour les templates d'OnGraphics.
4. Marges du cartouche coureur : **lues dans Figma dès la v1**, facteur de conversion à calibrer au test (§4.4).
