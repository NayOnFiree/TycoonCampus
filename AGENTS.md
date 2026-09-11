# TycoonCampus — contrat commun aux tâches

Projet Unreal Engine 5.8, C++ / Slate, Windows. Lire `Docs/Avancement.md` (dernier état),
`Docs/Collaboration.md` (périmètres) et le document de la mécanique concernée avant modification.
L'audit est historique : les derniers ajouts à Avancement font foi.

## Travail isolé et intégration

- Une tâche utilise son propre worktree et sa branche. Vérifier `git status --short --branch`
  et `git worktree list` avant d'éditer. Ne pas modifier les autres worktrees.
- `main` est la version d'intégration. Les travaux passent par une branche et une PR,
  même s'ils sont effectués par le propriétaire du dépôt. Ne pas forcer un push.
- Branches de départ : `work/jouabilite`, `work/ui-ux`, `work/graphisme`.
  Une seule tâche active par branche/worktree ; branches `feature/...` possibles ensuite.
- Livrer une mécanique ou un changement visuel cohérent à la fois. Ne pas lancer de
  refonte globale, deuxième bâtiment ou nouveau style sans demande correspondante.
- Une tâche d'intégration réunit les modifications une par une, vérifie les contrats et
  exécute les tests pertinents. Ne pas fusionner les assets binaires automatiquement.

## Contrats de code

- Préserver les modèles de simulation indépendants d'Unreal et déterministes.
- L'UI appelle les commandes applicatives et lit l'état ; ne pas introduire de débit,
  règle de réservation ou exécution économique dans les widgets.
- Construction : utiliser `FCampusConstructionService::Evaluate/Execute`. Ne pas rendre
  `ACampusBuilding::ApplyConstruction` public. La commande revalide avant mutation.
- Ne pas modifier le format de sauvegarde sans compatibilité explicite et tests de reprise.
- Les périmètres et fichiers partagés sont dans `Docs/Collaboration.md`. En cas de besoin
  transversal, décrire le contrat et coordonner l'intégration avant d'éditer le fichier partagé.

## Validation et compte rendu

- Changement de règles/sauvegarde : `pwsh -NoProfile -File Scripts/Test-Native.ps1`.
- Changement d'acteurs, d'interface C++ ou de commande : ajouter compilation/test Unreal
  `pwsh -NoProfile -File Scripts/Test-Unreal.ps1 -Isolated` si l'éditeur est ouvert.
- Ne pas fermer une session Unreal ni écraser une sauvegarde joueur pour les tests.
- Les documents seuls ne nécessitent pas de recompilation Unreal. La CI native reste requise.
- NullRHI ne valide ni le rendu, ni les vrais gestes souris. Pour l'UI/art, fournir captures
  et conditions d'observation si exécutées ; dire explicitement ce qui reste non observé.
- Une référence visuelle n'est mise à jour qu'après validation du changement voulu.
- Chaque livraison précise : jalon actuel, modifications, tests réellement exécutés,
  limites, prochain sous-jalon. Mettre le résumé détaillé dans le document du domaine ;
  l'intégration tient `Docs/Avancement.md` à jour pour éviter les conflits.

## Assets et fichiers locaux

- Garder les assets Unreal et médias sous LFS ; vérifier `git lfs status` avant livraison.
- Avant d'éditer un .umap/.uasset partagé, coordonner son propriétaire et utiliser
  `git lfs lock chemin` quand le serveur est disponible. Libérer le verrou après intégration.
- Ne jamais versionner Saved, Intermediate, Binaries, caches, installations de runner,
  tokens ni configuration privée. Ne pas ajouter une police ou un asset tiers sans sa licence.
