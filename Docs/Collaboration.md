# Organisation des trois tâches

## Une base commune, trois espaces

Le dépôt de référence est https://github.com/NayOnFiree/TycoonCampus.
`main` est l'intégration stable. Les tâches travaillent dans des worktrees Git séparés :

| Domaine | Branche de départ | Dossier local sous la racine | Responsabilité |
|---|---|---|---|
| Jouabilité | `work/jouabilite` | `.worktrees/jouabilite` | Simulation, commandes, économie, sauvegardes, tests |
| UI / UX | `work/ui-ux` | `.worktrees/ui-ux` | Parcours, widgets, lisibilité, focus, retours d'action |
| Graphisme | `work/graphisme` | `.worktrees/graphisme` | Direction artistique, assets, matériaux, éclairage |

La tâche actuelle coordonne l'intégration dans le dossier racine. Les copies de travail
partagent l'historique Git mais pas leurs fichiers ni leurs caches Unreal. Une nouvelle
tâche doit ouvrir le dossier de son domaine, pas le dossier racine ni celui d'une autre tâche.
La création des dossiers ne crée pas automatiquement des conversations dans Codex.

## Périmètres

- Jouabilité : modèles CampusTimeModel/WeeklySchedule/Operations/PathGrid/SaveCodec,
  services de commandes, tests et documents de mécaniques. Ne pas redessiner le HUD.
- UI/UX : CampusHUD, CampusHeritageHUD, SCampusPlanningPanel, CampusUIStyle,
  CampusAcademicStyle, `Content/UI`, `Docs/UI-Concepts` et documents UI.
  Appeler les commandes partagées ; ne pas déplacer l'économie dans Slate.
- Graphisme : nouveaux assets sous `Content/TycoonCampus/Art`, documents sous
  `Docs/Art`, matériaux et éclairage après accord sur le périmètre. Ne pas modifier
  les règles, dimensions de grille ou collisions pour améliorer uniquement l'apparence.
- Partagés, à intégrer explicitement : CampusBuilding, CampusCameraPawn,
  CampusGameMode, carte L_Campus_Work, configurations, Build.cs, .uproject,
  format de sauvegarde, AGENTS.md, workflows et Avancement.md.

Un fichier partagé n'est pas interdit : une tâche décrit d'abord le besoin et les fichiers
concernés dans sa livraison, puis sa modification est coordonnée avec l'intégration.
Les .umap/.uasset ne se fusionnent pas comme du texte : un seul propriétaire à la fois,
avec verrou LFS (`git lfs lock chemin`, puis `git lfs unlock chemin` après intégration).
Vérifier l'absence de fichiers read-only inattendus et les verrous avant édition dans Unreal.

## Cycle d'une livraison

1. Dans le worktree concerné, vérifier la branche et un état propre ; `git fetch origin`.
2. Incorporer `origin/main` avant une nouvelle livraison. Ne pas réécrire des commits
   déjà partagés : préférer `git merge origin/main` et résoudre les conflits localement.
3. Réaliser un changement limité, exécuter les tests pertinents et committer explicitement
   les fichiers concernés. Vérifier `git diff --cached` et `git lfs status`.
4. Pousser la branche, ouvrir une PR vers main et remplir le modèle fourni.
5. Attendre `Native tests`. Pour changement moteur/UI C++, fournir aussi le rapport local
   Unreal ; pour changement visuel, fournir une capture observée ou déclarer la limite.
6. Intégrer une PR à la fois, idéalement par squash, puis mettre à jour l'avancement.

La protection de main demande le contrôle natif et une PR, interdit la suppression et
le force-push, et demande de résoudre les conversations. Aucun second approbateur n'est
imposé au développeur solo. CODEOWNERS désigne le propriétaire du dépôt.

## CI

- `Native tests` : GitHub Actions, Ubuntu 24.04, sur chaque PR et push main.
  Utilise le même Test-Native.ps1 que Windows et conserve JSON/logs 14 jours.
- `Unreal integration` : déclenchement manuel depuis Actions, uniquement sur main,
  runner Windows x64 label `ue-5.8`, variable de dépôt `UE_ENGINE_ROOT`.
  Compile et lance les tests moteur dans le checkout du runner, distinct du jeu ouvert.
- Le runner Unreal doit être en ligne sur la machine équipée d'UE. Le workflow n'est pas
  un contrôle obligatoire de PR : il ne se déclenche pas sur le code d'un contributeur
  externe dans la machine personnelle. Une PR moteur doit donc joindre un test local
  avant fusion ; le workflow manuel permet la vérification de main après intégration.
- Les binaires du runner, ses credentials et son workspace restent sous `.ci-runner`,
  exclus de Git. Le runner n'est pas un service Windows au démarrage automatique.
  Après redémarrage, lancer `Scripts/Start-UnrealRunner.ps1` sur la machine configurée.
  Ne pas lancer plusieurs instances. Les machines sans runner peuvent utiliser le script
  local Test-Unreal.ps1 à la place.

Les tests NullRHI ne prouvent pas la lisibilité, les gestes souris, les performances GPU
ni le bon fonctionnement d'un jeu packagé. Ces étapes restent dans la feuille de route.

## Briefs de départ à utiliser dans les nouvelles tâches

**Jouabilité** : « Lis AGENTS.md et les documents de collaboration. Poursuis l'extraction
de la commande d'achat/suppression des chemins sur le modèle de la construction.
Préserve les sauvegardes et le comportement validé, ajoute les tests de refus sans mutation. »

**UI/UX** : « Lis AGENTS.md et les documents de collaboration. Audite le planning aux petites
résolutions et propose une disposition compacte cohérente avec le HUD validé. Commence
par une proposition documentée ; ne modifie pas les règles économiques ni la direction artistique. »

**Graphisme** : « Lis AGENTS.md et les documents de collaboration. Prépare une bible graphique
courte pour le campus et le gymnase, cohérente avec la direction déjà validée : palette,
silhouettes, matériaux, éclairage et budget d'assets. Commence par les références et conventions,
sans remplacer la carte ni changer les collisions ou la jouabilité. »

## Particularités locales

Le jeton Android File Server préexistant est conservé dans Saved/LocalConfig, jamais dans Git.
Le service Android inutilisé est désactivé dans la configuration partagée. Cela ne change
pas le rendu ni la simulation Windows. Les références de rapports dans les anciens documents
sont des résultats locaux historiques ; ils ne sont pas inclus dans un clone du dépôt.
