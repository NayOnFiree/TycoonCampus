# Pilote Heritage — gymnase, entrée et chemins

11 septembre 2026 · Proposition documentaire 01 · [Bible graphique](Bible-Heritage.md).
Jalon actuel : stabilisation 9–12, préparation artistique. Aucun changement visuel livré.

## Portion et invariants

Le futur essai porte sur le gymnase unique, son entrée et une courte portion raccordée :
au plus six cases visibles de 4 × 4 m, comprenant un segment droit et un coude. Choisir
ce cadrage dans une session de test isolée avec les commandes existantes. Cette portion
ne prescrit pas une nouvelle position du bâtiment ou un nouveau tracé sur la carte joueur.

| Contrat existant vérifié dans le code | Conséquence pour l'habillage |
|---|---|
| Parcelle de 200 × 200 m ; grille interne de 2 m, outil courant de 4 m | Aucun changement de grille, prix, index ou précision Maj |
| Sol du gymnase de 40 × 26 m, murs de 8 m ; terrain de 28 × 15 m | Conserver tous les volumes et positions actuels ; décor contenu dans l'enveloppe existante |
| Emprise de construction de 21 × 14 cellules, soit 42 × 28 m, permutable à la rotation | Ne pas remplacer l'emprise logique par les dimensions du sol |
| Ouverture centrale côté -X, entre les deux pans de façade | Aucun seuil, poteau ou panneau dans le passage ; aucun déplacement de DoorCell |
| Sélection par une boîte de demi-dimensions 2100 × 1350 × 430 cm | Conserver boîte, réponses de collision et cadre de sélection |
| Chemins et figurants en maillages instanciés | Conserver instanciation, trajets et compte logique indépendants du décor |

Sources : [géométrie](../../Source/TycoonCampus/CampusGymGeometry.h),
[emprise](../../Source/TycoonCampus/CampusConstruction.h),
[bâtiment](../../Source/TycoonCampus/CampusBuilding.cpp),
[chemins et usages](../Jalons-09-10-Chemins-Usages.md).

## Lot proposé et budgets

Budgets de départ à confirmer par profilage ; aucune performance mesurée ou garantie.
Le nombre d'instances est celui du cadrage pilote, pas un plafond de population du campus.

| Élément | Proposition limitée | Budget proposé |
|---|---|---|
| Gymnase | Habillage minéral et bois, dessus ouvert ; géométrie actuelle conservée | Au plus 2 nouveaux meshes de détail, 10 000 triangles LOD0 cumulés |
| Entrée | 1 encadrement contenu dans les pans existants, 1 panneau uni puis inscription si licence résolue | Inclus dans les 2 meshes précédents ; 2 slots de matériau/mesh maximum |
| Chemins | Matière gravier clair sur la portion d'essai | Aucun mesh nouveau ; aucune bordure physique |
| Végétation | 1 arbre répété 2 fois, 1 arbuste répété 4 fois, hors accès | Arbre : 3 000 triangles ; arbuste : 500 ; 1 slot chacun |
| Personnages | Existants conservés | 8 figurants maximum, aucun squelette ou animation ajouté |
| Matériaux et textures | 1 maître opaque, au plus 6 instances de matière | 0 texture au premier essai ; ensuite au plus 2 jeux de 3 textures 1024², sans 4K |

Plafond total : 4 nouveaux meshes réutilisables et 18 000 triangles ajoutés visibles en
LOD0 (10 000 + 2 × 3 000 + 4 × 500). Au plus 20 appels de dessin supplémentaires sur
le même cadrage ; à vérifier, les instances de matière et les ombres pouvant multiplier
les passes. Cible mémoire : moins de 32 Mio de textures résidentes ajoutées, mesurées
après import. Aucun nouveau Tick, acteur de foule, lumière locale ou shader translucide.

Le premier essai peut se limiter aux matières : les plafonds ne sont pas une liste d'assets
à fabriquer obligatoirement. Nanite et Lumen ne deviennent pas des dépendances nouvelles ;
garder les réglages de rendu du projet pendant la comparaison.

## Niveaux de détail et observation

| Échelle de lecture | À préserver | Simplification proposée |
|---|---|---|
| Proche, bras caméra 25 m | Matières, entrée, texte éventuel, figurants et terrain | LOD0 ; détails uniquement là où ils sont visibles |
| Moyenne, bras caméra 80 m | Silhouette, accès et séparation chemin/pelouse | LOD1 des nouveaux meshes : environ 50 % des triangles ; panneau uni |
| Lointaine, bras caméra 240 m | Bâtiment, réseau et grandes masses végétales | LOD2 : environ 20 % ; détails d'entrée et arbustes masquables |

25–240 m correspond aux limites actuelles du bras caméra ; ces valeurs ne sont pas des
distances universelles de bascule LOD. Régler les seuils sur la taille à l'écran après
captures, sans changer le zoom. Ne pas masquer de représentation porteuse d'un état de
jeu pour respecter un budget. Pas de feuillage scintillant ou de changement brutal de silhouette.

Protocole futur : même placement, angle, exposition, réglages, résolution et état simulé
avant/après. Relever CPU/GPU, RAM/VRAM, triangles et appels de dessin après échauffement,
pendant 30 secondes, sur trois passages. Tester pause, x1 et x4 avec huit figurants.
Consigner CPU/GPU exacts et matériel cible ; seuls 16 Go de RAM sont annoncés dans la
conception. Viser 60 FPS (16,7 ms/image) et moins de 1 ms GPU ajouté au pilote, à confirmer.
Si le blockout dépasse déjà ce temps, établir d'abord la mesure de référence et revoir le
budget. Réduire détails, ombres décoratives et textures avant d'élargir le lot.

Captures requises : 1280 × 720 et 1920 × 1080, vue générale et entrée rapprochée,
gymnase sélectionné, raccordé puis non raccordé ; mode chemins avec case cyan et aperçu.
Observer les quatre orientations de placement et plusieurs rotations caméra. Le panneau
HUD reste lisible, l'accès identifiable et le terrain visible. Vérifier les vrais clics
et glissers, y compris sur le décor : pas d'interception nouvelle. Joindre conditions,
version, résolution et résultat ; une maquette n'est pas une capture Unreal.

## Ordre de réalisation et coordination

1. Faire relire la bible et ce lot. Cette PR porte exclusivement sur la proposition.
2. Après validation artistique, préparer les matières et un assemblage d'essai sous
   `Content/TycoonCampus/Art/Heritage/`, avec licence et suivi LFS vérifiés.
3. Coordonner l'intégration avant de brancher l'habillage : `CampusBuilding.cpp/.h`
   est partagé ; l'affectation des matières de chemins concerne `CampusPaths.cpp`
   côté jouabilité. Contrat demandé : présentation seule, aucune mutation du modèle,
   de la collision, du raccordement, de la sélection ou du format de sauvegarde.
4. `L_Campus_Work.umap` et l'éclairage existant appartiennent à l'intégration. Si une
   modification devient nécessaire, préciser les acteurs concernés, obtenir le créneau
   et le verrou LFS. Ne pas relancer `create_work_map.py` ni remplacer cette carte.
5. Exécuter les tests natifs et, si acteurs/C++ modifiés, `Test-Unreal.ps1 -Isolated`,
   puis le protocole visuel ci-dessus sans fermer l'éditeur joueur ni utiliser sa sauvegarde.
6. Livrer les captures pour validation ; mettre à jour une référence visuelle uniquement
   après acceptation. Le prochain lot reste une décision distincte.

## Compte rendu de cette livraison

- Modifications : bible, palette, conventions, inventaire licences, portion pilote,
  budgets provisoires et contrat d'intégration, uniquement dans `Docs/Art`.
- Vérifications : lecture des documents de design et du code des dimensions ;
  `pwsh -NoProfile -File Scripts/Test-Native.ps1` exécuté le 11 septembre 2026 :
  **7 suites réussies sur 7**. Rapport local (non versionné) :
  `Saved/Tests/Native/302c481dc30046669dbb91277dc00984/results.json`.
  L'état du contrôle GitHub `Native tests` sera consigné dans la PR.
- Limites : aucune session Unreal, capture, mesure GPU ou observation des gestes effectuée.
  Aucune compilation Unreal nécessaire pour ces documents. Aucun asset importé ou modifié.
- Prochain sous-jalon : validation de la proposition, puis essai de matières sur le gymnase
  et l'entrée dans le périmètre coordonné. Pas de deuxième bâtiment, nouveau style ou travail Blender.

`Docs/Avancement.md` reste à la tâche d'intégration pour éviter les conflits.
