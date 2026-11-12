# Styles de rendu : contrat v1

Un **style** est un rendu global (Réaliste, Noir et blanc, Carto, Cell shading...) choisi
à un seul endroit : la page **Styles** de la régie. Ce document fixe ce que se partagent
la régie, le cadre Unreal (`UStyleSubsystem`) et les fils qui construisent les presets.

## 1. Le fichier de style (régie)

Rangé dans la régie : `data/styles/<nom>.json` (nom : lettres, chiffres, `_`, `-`).
Le style `realiste` est intégré, il n'a aucune section et ne coûte rien.

```json
{
  "name": "noir_et_blanc",
  "label": "Noir et blanc",
  "description": "N&B contrasté, tracé et étiquettes en couleur",
  "postprocess": [
    {
      "material": "/Game/LTVContent/Materials/Styles/PP_Grade.PP_Grade",
      "weight": 1.0,
      "scalars": { "Saturation": 0.0, "Contrast": 1.2 },
      "vectors": { "Tint": "#FFFFFFFF" }
    }
  ],
  "imagery": { "source": "mapbox", "style": "mapbox/outdoors-v12", "hidePhotoTiles": true },
  "sky": { },
  "trail": { }
}
```

Règles :
- **Toute section est optionnelle.** Une section absente = rendu d'origine pour ce domaine.
  `realiste` = aucune section.
- Les clés de premier niveau autres que `name`, `label`, `description` sont des **sections**.
  Chaque section est confiée au handler du même nom (voir 3). Une section sans handler
  est ignorée (log d'avertissement), jamais d'erreur.
- Le contenu d'une section appartient au fil qui écrit le handler : il peut ajouter des clés
  sans toucher au cadre ni recompiler un `.h`.
- Couleurs : chaîne `#RRGGBB` ou `#RRGGBBAA` (sRGB), comme les templates.
- Secrets (clé Mapbox...) : **jamais dans le JSON de style ni dans git**. La section
  `imagery` dit seulement `"source": "mapbox"` ; la clé est lue localement côté Unreal
  ou côté régie (`settings.json` local, hors dépôt), au choix du fil Carto.

## 2. Commandes

Côté **Unreal** (`/command` sur le port 8200, via le proxy régie `:5050/command`) :

| Commande | Effet |
|---|---|
| `group=style&action=set&name=<nom>&data=<json>` | Applique le style donné en ligne (`data` = JSON complet du style, encodé URL). |
| `group=style&action=set&name=<nom>` | Sans `data` : Unreal lit `MainURL + style/<nom>` sur la régie, puis applique. |
| `group=style&action=set&name=realiste` | Retour au rendu d'origine (aucune lecture réseau). |
| `group=style&action=reload` | Relit et réapplique le style courant. |

La régie envoie toujours `data` (pas de dépendance au réseau ni à CF-Access côté Unreal).

Côté **régie** (port 5050) :

| Route | Effet |
|---|---|
| `GET /api/styles` | Liste `[{name, label, description, builtin, sections}]` + `current`, `default`, `races`. |
| `GET/PUT/DELETE /api/styles/{nom}` | Lire / enregistrer / supprimer un style. |
| `POST /api/styles/{nom}/apply` | Envoie `style set` à Unreal et le mémorise comme style courant. |
| `GET /style/{nom}` | JSON brut pour Unreal (repli quand `data` manque). |
| `GET/PUT /api/styles/assignments` | `{ "default": "realiste", "races": { "215": "carto_outdoors" } }` |

**Par course** : quand la régie relaie `group=race&action=movetorace&idrace=X`, elle envoie
ensuite le style de la course X (ou le défaut global) s'il diffère du style courant.

## 3. Le cadre Unreal : `UStyleSubsystem` (module Subsystems)

`UGameInstanceSubsystem`. Tout se passe au moment du changement de style, **rien par image**.

```cpp
#include "StyleSubsystem.h"

struct FStyleSectionHandler
{
    // Appliquer la section (le style courant la contient). Rappelé si le style change
    // et contient encore la section : le handler doit accepter d'être réappliqué.
    TFunction<void(UWorld* World, const TSharedRef<FJsonObject>& Section)> Apply;
    // Revenir au rendu d'origine pour ce domaine (style sans cette section).
    TFunction<void(UWorld* World)> Revert;
};

UStyleSubsystem* Styles = GameInstance->GetSubsystem<UStyleSubsystem>();
Styles->RegisterSectionHandler(TEXT("imagery"), Handler);   // dans Initialize / BeginPlay
Styles->UnregisterSectionHandler(TEXT("imagery"));          // dans Deinitialize / EndPlay (Revert appelé si actif)

Styles->ApplyStyleJson(Name, JsonString);  // ce que fait la commande style set
Styles->ApplyStyle(Name);                  // lecture régie puis application
Styles->GetCurrentStyleName();             // "realiste" par défaut
Styles->OnStyleChanged;                    // FOnStyleChanged(const FString& Name)
```

- Un handler enregistré **après** l'application d'un style reçoit tout de suite sa section.
- Une section qui n'est pas un objet JSON (ex. tableau `postprocess`) est passée enveloppée :
  `{ "items": [ ... ] }`.
- Changement de niveau : le cadre réapplique le style courant dans le nouveau monde
  (`OnPostWorldInitialization` → au `BeginPlay` du monde).

### Section `postprocess` : gérée par le cadre

Le cadre fournit le handler `postprocess` commun (N&B, cell shading et tout autre preset
de post-process l'utilisent, aucune autre infrastructure à écrire) :

- Chaque entrée = un matériau de post-process (domaine Post Process) → une
  `UMaterialInstanceDynamic` avec ses `scalars` / `vectors`, poids `weight` (0..1).
- Les entrées sont posées dans l'ordre sur **un seul `APostProcessVolume` non borné**
  créé par le cadre (priorité 1000, `WeightedBlendables`). Aucun volume en Réaliste.
- Réappliquer un style avec le même matériau ne recrée pas l'instance : on ne change
  que les paramètres (bascule instantanée, pas de compilation de shader en direct si
  le matériau est déjà chargé ; les matériaux de style sont à précharger).
- Le point d'insertion (avant/après tonemapping, avant translucence) est réglé **dans
  le matériau** (Blendable Location), pas dans le JSON.

### Lisibilité du tracé, des bornes et des étiquettes

Convention réservée : **CustomStencil = 200** marque un overlay à protéger.
Un matériau de style lit `SceneTexture:CustomStencil` et laisse passer la couleur d'origine
quand le stencil vaut 200. Le marquage des acteurs (tracé, km, étiquettes : `bRenderCustomDepth`,
`CustomDepthStencilValue = 200`, `r.CustomDepth=3`) est fait **par le cadre** quand une entrée
porte `"protectOverlays": true` et défait au retour en Réaliste. Le fil N&B valide la convention
(alternative : overlays translucides rendus après le post-process « Before Translucency »).

## 4. Régie : page Styles

Une page `/styles` : liste des presets, bouton Appliquer (style courant en surbrillance),
éditeur JSON simple par style, défaut global et style par course.
Les fils de presets ajoutent leur preset en déposant `data/styles/<nom>.json`
(ou un intégré dans `_BUILTINS` de `app/routers/styles.py`).

## 5. Contraintes communes

- Réaliste strictement identique : aucun volume, aucune instance, aucun changement d'imagery.
- 25 i/s prioritaire, rien par image au thread de jeu, bascule sans crash en direct.
- Un nouveau `.h` impose une compilation complète éditeur fermé : prévenir l'utilisateur.
