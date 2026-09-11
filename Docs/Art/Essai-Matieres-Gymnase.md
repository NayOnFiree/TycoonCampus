# Essai de matières Heritage — gymnase et entrée

11 septembre 2026. Jalon actuel : stabilisation 9–12, premier essai de matières après
acceptation de la [bible](Bible-Heritage.md). Branche `codex/graphisme-matieres-gymnase`,
PR empilée vers `codex/graphisme-bible-heritage` ; la PR #3 reste indépendante et non fusionnée.

## Réalisation

Quatre instances dynamiques du matériau moteur `BasicShapeMaterial` sont créées une
fois au BeginPlay du bâtiment. Ses paramètres `Color` et `Roughness` permettent cet
essai sans texture ni nouveau matériau binaire. Les teintes sRGB sont converties en
linéaire avant affectation. Les instances sont partagées entre les pièces de même finition.

| Pièces existantes | Finition | sRGB | Rugosité |
|---|---|---|---|
| Trois murs arrière/latéraux | Pierre/enduit chaud | `#C8BBA4` | 0,85 |
| Deux pans encadrant l'ouverture -X | Tabac | `#82664E` | 0,75 |
| Rectangle du terrain sportif | Bois sportif mat, sans veinage | `#AF8964` | 0,65 |
| Dalle mince sous le terrain | Sombre, finition métal peint | `#303735` | 0,80 |

Le sombre couvre aussi la portion de dalle visible autour du terrain. Il s'agit d'un
aplat de métal **peint**, sans métal nu réfléchissant : le matériau moteur reste
diélectrique. Aucun poteau, linteau ou bande de géométrie n'a été ajouté pour porter
cette finition. Ce choix reste à apprécier visuellement avant toute extension.

Les cinq lignes du terrain gardent leur matériau original. Les figurants, le cadre de
sélection et les chemins conservent également leurs matériaux. La palette ne modifie
pas les matériaux partagés du blockout. Le gymnase préplacé de la carte reste inchangé
dans l'éditeur ; l'essai concerne le gymnase runtime construit ou repris en partie.

## Contrat conservé

`CampusBuilding.cpp/.h` constitue le seul raccord de production, autorisé pour ce lot.
Aucun changement de `CampusGymGeometry`, transformation des douze pièces, ouverture,
boîte de sélection, collision, emprise, commande, modèle ou sauvegarde. Aucun changement
de carte, d'éclairage enregistré, de chemins, de personnage ou de HUD. Aucune allocation
de matière par Tick. Aucun mesh, texture, lumière ou animation ajouté.

## Licences et LFS

Code et documentation originaux. Référence au matériau et au cube fournis par Unreal,
déjà disponibles dans l'installation moteur ; aucun asset moteur copié dans le dépôt.
Aucune texture tierce, police ou ressource externe importée ; Blender n'est pas utilisé.
Les éventuelles captures PNG de test relèvent de la règle LFS existante. Les résultats
locaux sous `Saved` restent exclus du versionnement. Aucun binaire partagé à verrouiller.

## Vérification

- Tests natifs : sept suites réussies, rapport local
  `Saved/Tests/Native/04be7825c56b47f1a1aa321faa71cf43/results.json`.
- Test moteur `HeritageMaterials` : contrôle des douze transformations, absence de
  collision décorative, invisibilité avant achat, boîte de sélection, palette linéaire,
  présence des paramètres, opacité et partage des quatre instances. Le scénario existant
  `BootAndPanels` conserve la validation de construction et des panneaux.
- `Test-Unreal.ps1 -Isolated` : compilation Editor réussie (400 s). Le premier
  démarrage a échoué car ce nouveau worktree contenait les pointeurs LFS des assets.
  Après `git lfs pull` et copie des binaires dans le projet isolé, relance du même
  script avec `-SkipBuild` **dans cette copie** : deux tests réussis, zéro échec.
  Journal de compilation : `Saved/Tests/Unreal/9937e1c2504e4d21ae82e75fc28a9ec4/build.log`.
  Rapport runtime : `Saved/Tests/Unreal/9937e1c2504e4d21ae82e75fc28a9ec4/Project/Saved/Tests/Unreal/8f025dce6e2040c3ae8bb7f6514cd2ec/index.json`.
  Les sources du test compilé et de la livraison ont été comparées par SHA256.
- Après ajustement de l'attente des shaders pour les captures, dernière recompilation
  incrémentale réussie (72 s) et deux tests à nouveau réussis dans la même copie.
  Rapport final : `Saved/Tests/Unreal/9937e1c2504e4d21ae82e75fc28a9ec4/Project/Saved/Tests/Unreal/0e9e84a260f241f3bf784e2b1bf00d25/index.json`.

## Observation reproductible

Le test moteur possède deux options réservées au processus isolé : `-CampusArtCapture`
cadre le gymnase acheté par `BootAndPanels` avec la caméra existante ;
`-CampusArtBaseline` réaffecte uniquement les matériaux blockout à ce spécimen avant
capture. Cette comparaison avant/après n'est pas une modification ni une sauvegarde de
la carte. Exécuter toute la suite `TycoonCampus.Runtime` dans les deux cas.

Même caméra (bras 90 m, inclinaison -55°, azimut -35°, champ 60°), même placement issu
du test, simulation en pause, bâtiment sélectionné et non raccordé, aucun figurant.
L'adaptation automatique d'exposition est désactivée dans le processus de capture.
La compilation des assets doit finir, puis 15 secondes d'attente précèdent la capture
avec HUD ; le déclenchement vérifie encore l'absence d'assets en compilation.
Produire séparément
1280×720 et 1920×1080 avec le même moteur et les mêmes réglages. Les rendus ne constituent
ni une référence approuvée, ni un test de geste souris, ni un profilage de performance.

Captures obtenues sous Unreal 5.8, D3D12, GTX 1660, i5-6500, 16 Go de RAM,
avec `-RenderOffscreen -windowed -ForceRes`. Les paramètres `-ResX` et `-ResY`
fixent chaque résolution. La session joueur reste ouverte. Les premières images
avec messages de préparation shaders ont été écartées ; le journal des images retenues
confirme zéro compilation d'asset en attente au déclenchement.

| Résolution | Blockout | Heritage |
|---|---|---|
| 1280×720 | [Avant](Captures/Essai-Matieres/Baseline-1280x720.png) | [Essai](Captures/Essai-Matieres/Heritage-1280x720.png) |
| 1920×1080 | [Avant](Captures/Essai-Matieres/Baseline-1920x1080.png) | [Essai](Captures/Essai-Matieres/Heritage-1920x1080.png) |

Observation : entrée tabac distincte des murs, terrain visible, cadre et case cyan
conservés. La dalle sombre accentue les ombres déjà fortes du blockout ; sa valeur est
un point de validation artistique. Le premier emplacement légal du scénario est près
du bord de parcelle, d'où le fond noir hors terrain. Aucun décor n'a été ajouté pour
masquer cette limite du cadrage. Les consignes du HUD sont celles de la branche de base.
Pas de validation des quatre orientations, du raccordement, des gestes réels, des
figurants actifs, de la reprise de sauvegarde en rendu ou du jeu packagé dans cet essai.

Prochain sous-jalon : appréciation des aplats en jeu, en particulier la dalle sombre
et l'entrée, puis ajustement limité des teintes/rugosités selon le retour. Les textures,
la végétation, la signalétique, les chemins et Blender restent hors de ce lot.
