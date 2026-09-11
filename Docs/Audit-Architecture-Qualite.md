# Audit de la base et qualité — 11 septembre 2026

## Verdict

La base est saine pour un prototype à un gymnase. Elle ne doit pas être jetée. Elle n'est pas encore organisée pour multiplier les bâtiments et les écrans sans augmenter fortement le risque de régression.

Le meilleur investissement est maintenant de stabiliser une boucle complète : construire → raccorder → réserver → exécuter → expliquer le résultat → sauvegarder/reprendre. Ajouter des mécaniques avant cette stabilisation compliquerait les trois niveaux à la fois.

Périmètre : lecture des sources C++, configurations, tests et documentation. Aucun rendu du jeu observé pendant cet audit. Les observations UI et graphisme portent sur le code et les validations manquantes, pas sur une appréciation visuelle du résultat. Les recommandations moteur s'appuient aussi sur la documentation Epic citée en fin de document.

## Ce qui est déjà bien

- Horloge, planning, économie, grille et codec sont du C++ indépendant d'Unreal. Conserver cette séparation : elle rend la simulation rapide à tester et reproductible.
- Les suites existantes testent déjà des refus et des régressions, pas seulement les cas heureux. Certains tests sont déjà de l'intégration entre modèles ; leur emplacement hors moteur ne les rend pas tous unitaires.
- Les opérations évitent de payer plusieurs fois la même heure et capturent une séance commencée avant modification du planning.
- Le service de sauvegarde utilise un fichier temporaire, une relecture, une copie de secours et une validation des contraintes de carte avant application. C'est une bonne défense contre les pertes de partie.
- Les figurants utilisent des instances de meshes. Bon choix pour un prototype de gestion, à mesurer à plus grande échelle.
- Avancement.md distingue explicitement compilation et validation par le joueur. Garder cette honnêteté, mais remplacer une partie des vérifications répétitives par des tests exécutables.

## Problèmes et risques prioritaires

| Priorité | Constat dans les sources | Conséquence | Action conseillée |
|---|---|---|---|
| P1 | CampusBuilding.h possède planning, trésorerie, exécution et animation ; GetOperations/GetSchedule exposent des références modifiables | Les futures salles risquent de posséder chacune leur budget ; les règles peuvent être contournées par les widgets | État de campus central et commandes applicatives avant le multi-bâtiment |
| P1 | SCampusPlanningPanel.cpp recrute, accepte les contrats et modifie directement le planning ; CampusCameraPawn.cpp gère caméra, placement, chemins et sauvegarde | Une règle ou un mode doit être synchronisé à plusieurs endroits | Extraire progressivement commandes de jeu et contrôleur des outils |
| P1 | CampusSaveService.cpp est friend de plusieurs acteurs et réapplique leurs détails privés, y compris caméra et figurants | Chaque changement d'acteur peut casser la restauration ; modèle et présentation sont fortement liés | Capture/validation/application explicites par composant ; versionner et tester les migrations |
| P1 | Pas de scénario existant qui exerce les vrais gestes souris et le rendu | Les modèles peuvent passer avec un glisser inutilisable ou un panneau illisible | Automation Driver, tests fonctionnels et références visuelles à ajouter après le smoke test |
| P2 | Building::Tick et de nombreux attributs du HUD recherchent les acteurs via TActorIterator | Coût répété et dépendance implicite au premier acteur trouvé | Références faibles mises en cache, identifiants de bâtiments, notifications de changement |
| P2 | Horloge et bâtiment tickent séparément ; les événements de l'horloge ne pilotent pas directement l'économie | Ordre de simulation implicite ; les observateurs peuvent voir un état intermédiaire | Un orchestrateur avance l'horloge et la simulation avant de publier les changements |
| P2 | GameMode masque le blockout en reconnaissant position, échelle et rotation ; SaveService accepte un nom de carte précis | Une modification de niveau peut casser le comportement silencieusement | Tags/composants pour le blockout et identifiant de scénario/carte stable |
| P2 | Planning : grille minimum 620 + éditeur 320, alors que le conteneur peut descendre à 320 ; compensation DPI locale dans HeritageHUD | Risque de débordement et de tailles incohérentes à petite résolution ou DPI élevé | Politique DPI commune, passage en vue compacte, vérifier les tailles réelles |
| P2 | Styles Heritage, Academic et CampusUI coexistent ; police Heritage chargée depuis Windows | Incohérences et captures dépendantes de la machine | Un thème actif, polices embarquées avec licence appropriée, widgets communs |
| P2 | Lumen, Virtual Shadows et ray tracing activés, sans budget de performance observé | La qualité visuelle n'a pas encore de coût maîtrisé | Définir une machine cible et profiler avant de modifier ces options |
| P2 | Aucun dossier .git dans ce workspace | Pas d'historique Git local visible ni de CI attachée | Mettre le projet sous versionnement avec exclusion des caches et stockage adapté aux assets |

Ce tableau distingue des limites de conception et des risques ; il ne prétend pas que chacun produit déjà un bug visible. Les ressources UI/Heritage sont déjà déclarées dans DirectoriesToAlwaysStageAsNonUFS : le packaging n'est pas oublié, mais doit être essayé sur un build réellement distribué.

## Architecture cible, sans réécriture générale

```text
Entrées joueur / widgets Slate
          ↓ commandes : construire, réserver, recruter, charger
Service de session de campus (orchestration Unreal)
          ↓
Modèle déterministe : temps, budget commun, bâtiments par ID, planning, accès
          ↓ résultats et changements d'état
Présentation : acteurs 3D, figurants, sons et modèle de vue UI

Sauvegarde ↔ instantané versionné de la session
Tests → mêmes commandes que le joueur + tests des entrées réelles
```

Une implémentation possible est un UWorldSubsystem qui possède la session. Garder GameMode pour l'initialisation et les règles de démarrage. Conserver les modèles indépendants du moteur sous ce service. Pas besoin de multiplier les modules Unreal, d'adopter ECS ou de migrer Slate vers UMG pour résoudre ces problèmes.

Chaque commande doit valider puis appliquer atomiquement et retourner un résultat explicite : réussite, budget insuffisant, terrain occupé, personnel absent, etc. Le bouton et le raccourci passent par la même commande. L'UI lit un état et traduit le résultat ; elle ne débite pas l'argent elle-même. Le modèle de vue peut rester une petite structure C++ : aucun framework supplémentaire obligatoire.

Séparer la trésorerie du campus des opérations d'une salle, puis donner un ID stable à chaque bâtiment. Enregistrer les références par ID dans les sauvegardes. Extraire ensuite la présentation des figurants du bâtiment. Traiter le changement du format de sauvegarde comme une fonctionnalité avec fixtures anciennes et règles de compatibilité explicites.

## Les trois niveaux à faire fonctionner ensemble

### Jouabilité

- Décrire chaque mécanique en termes d'action, coût, délai, effet et cause d'échec. Exemple : une séance non raccordée doit être refusée à l'exécution et son motif visible dans le planning/journal.
- Un seul état de jeu fait foi. Les participants visuels illustrent l'activité ; ils ne déterminent pas les recettes.
- Pause, vitesse, sauvegarde et reprise doivent conserver les mêmes résultats économiques. Définir aussi le comportement des modifications pendant une séance.
- Rejouer un scénario déterministe pour comparer avant/après. Si de l'aléatoire est ajouté, sauvegarder la graine et l'état du générateur.
- Pour l'équilibrage, compléter les tests de correction par des simulations de stratégies : joueur prudent, dépenses rapides, planning vide, contrats manqués. Mesurer trésorerie, temps d'attente et progression ; ne pas affirmer qu'un jeu est amusant parce que les comptes sont exacts.

### UI / UX

- Priorité visuelle : argent et temps → action courante → objet sélectionné → détails. Toujours montrer le mode actif, le prix et la raison d'un refus.
- Définir les transitions entre sélection, construction, chemins, planning et menu. Échap annule/ferme de manière cohérente ; un clic UI ne construit pas dans le monde derrière.
- Unifier espacements, typographie, états survol/pressé/désactivé, messages et focus clavier. La couleur seule ne doit pas porter une alerte.
- Vérifier 1280×720, 1920×1080 et 2560×1440, puis DPI Windows 100/125/150 %. Ces cas sont une proposition de matrice, pas des configurations validées.
- Faire un petit essai avec une personne qui découvre le jeu : peut-elle construire, raccorder et expliquer pourquoi une séance échoue sans consigne orale ? Ce contrôle humain reste indispensable.

### Graphisme

- Rédiger une courte bible : palette, silhouettes, échelle, matériaux, éclairage, lisibilité des chemins et sélection. Finaliser une portion jouable du campus avant de produire tous les assets.
- Comparer le HUD dans Unreal avec la direction approuvée, et dans plusieurs états : valeurs longues, alerte, pause, planning plein. Une maquette HTML n'est pas une preuve du rendu Slate.
- Fixer caméra, heure, résolution, qualité et état de simulation pour les captures. Stabiliser les animations et laisser chauffer le rendu avant la prise ; éviter de masquer des zones critiques pour faire passer le test.
- Choisir la référence GPU/pilote pour les comparaisons, stocker référence/actuel/différence. Une référence nouvelle doit être approuvée visuellement, jamais réenregistrée automatiquement après un échec.
- Définir une cible mesurable, par exemple 60 FPS à 1080p sur la machine minimale choisie. Mesurer temps CPU/GPU, p95/p99 des frames, mémoire et chargements avec Unreal Insights ; les seuils restent à calibrer.

## Automatisation livrée

### Modèles et intégration

Depuis la racine du projet :

```powershell
pwsh -NoProfile -File Scripts/Test-Native.ps1
```

Prérequis : g++ C++17 disponible dans PATH (ou paramètre -Compiler). Le script découvre les suites, les recompile, exécute chaque binaire avec timeout et écrit logs + results.json dans un dossier unique Saved/Tests/Native. Il retourne un code d'échec si une suite échoue ou si aucune n'est trouvée.

CampusIntegrationTests.cpp ajoute : achat du gymnase + chemins + réservation payante, absence de recette anticipée, pause sans mutation, sauvegarde en pleine activité/reprise identique, simulation de 16 semaines en une avance ou par heures et bouclage du journal. C'est un test d'intégration des modèles ; il ne valide pas les collisions ni les gestes dans Unreal.

### Intégration Unreal

```powershell
pwsh -NoProfile -File Scripts/Test-Unreal.ps1
# Si l'éditeur est ouvert : copie temporaire, compilation et test indépendants
pwsh -NoProfile -File Scripts/Test-Unreal.ps1 -Isolated
# Moteur ailleurs : ajouter -EngineRoot 'X:/Epic/UE_5.8'
# Réutiliser une compilation connue à jour : ajouter -SkipBuild
```

Compile la cible Editor puis lance un processus Unreal de jeu séparé, avec la carte du campus et le filtre TycoonCampus.Runtime. Le test contrôle les acteurs uniques, la caméra possédée, la pause initiale, le menu, le refus du planning avant construction, la construction et le débit, puis l'ouverture/fermeture du planning. La mutation est limitée à ce monde de test ; aucune sauvegarde joueur n'est écrite. Le flag CampusAutomation interdit l'exécution accidentelle dans une session ordinaire.

Le lanceur impose un timeout et exige un rapport JSON frais avec au moins un succès, aucun échec et aucun test restant. Logs et rapport vont dans Saved/Tests/Unreal. Il utilise NullRHI : il valide l'intégration et les états des panneaux, **pas leur rendu, leur hit testing ou le comportement réel du curseur**.

Le mode -Isolated copie Source, Config, Content et le fichier projet dans le dossier du rapport ; il évite les DLL verrouillées par une session d'éditeur ouverte. Cette copie et ses fichiers de compilation sont conservés pour diagnostic et occupent de l'espace disque. Ne pas combiner -Isolated et -SkipBuild. En mode normal, fermer l'éditeur avant compilation si sa DLL est chargée.

### À compléter pour une validation complète

| Niveau | Scénarios | Fréquence proposée | Preuve attendue |
|---|---|---|---|
| Entrées réelles | Clic UI sans action monde, rectangle de chemins au relâchement, annulation, zoom/rotation, reprise après menu | À chaque changement d'input/UI | Automation Driver/Functional Tests + assertions sur l'état |
| Sauvegarde disque | Écriture, reprise, secours, fichier corrompu, ancienne version, obstacles modifiés | À chaque changement de persistence | Slot de test isolé + état avant/après ; rendre d'abord le chemin injectable |
| Visuel | HUD, fiche, planning, alertes aux résolutions définies | À chaque changement UI/art | Captures Unreal et différences face aux références approuvées |
| Performance | Campus chargé, beaucoup de chemins/figurants, simulation accélérée | Nuit ou changement structurel | Traces CPU/GPU, percentiles et mémoire comparés aux budgets |
| Packaging | Cook/package Development, lancement sans éditeur, ressources et sauvegarde, puis smoke Shipping adapté | Avant livraison | Build installable et logs d'une session réelle |
| Expérience joueur | Compréhension, lisibilité et rythme | Chaque mécanique complète | Retour humain et décision de validation |

Utiliser l'Automation Framework et Functional Testing pour les scénarios moteur, Automation Driver pour les gestes Slate, Screenshot Comparison pour les images et Gauntlet pour orchestrer les sessions/builds packagés. Ajouter ces couches progressivement : elles ne sont pas toutes implémentées par les deux lanceurs présents.

Pour la CI future : agent Windows avec compilateur et version UE installée, lancement des deux scripts, archivage des rapports même en cas d'échec, arrêt de la livraison en cas d'échec. Les tests GPU doivent utiliser une machine graphique stable. Aucun service CI ni tâche planifiée n'a été créé : aucun dépôt distant/agent d'exécution n'est configuré dans ce workspace. Les scripts rendent l'exécution automatique possible dès maintenant depuis une commande.

## Ordre recommandé

1. Exécuter les contrôles livrés et valider visuellement le HUD actuel et le geste des chemins. Ne pas ajouter un nouveau style entre-temps.
2. Extraire un premier parcours de commande, par exemple construire, avec les mêmes tests avant/après. Centraliser ensuite les modes d'outil.
3. Ajouter les tests d'entrées réelles et de sauvegarde disque isolée ; approuver les premières références visuelles.
4. Déplacer le budget vers la session, ajouter les IDs et la compatibilité de sauvegarde avant un deuxième bâtiment.
5. Activer CI, essais packagés et budgets de performance sur un scénario représentatif.

Une mécanique est terminée quand sa règle, son intégration, son retour UI, son rendu et sa reprise de sauvegarde ont été vérifiés au niveau pertinent. L'automatisation réduit les régressions ; elle ne remplace pas la décision artistique ou les essais de jouabilité.

## Références officielles

- [Epic — Automation Test Framework](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine) : catégories de tests moteur, fonctionnels et comparaison de captures.
- [Epic — Gauntlet Automation Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-in-unreal-engine) : orchestration de sessions et de builds.

## Résultats de cette intervention

- Sept suites natives recompilées et réussies, dont la nouvelle intégration. Rapport : Saved/Tests/Native/08516bb33a014e4388b0083801b387c5/results.json.
- Compilation Editor réussie sur copie isolée et test TycoonCampus.Runtime.BootAndPanels réussi (1 succès, 0 échec). Rapport : Saved/Tests/Unreal/de6c82a138af43298ee786a9717c1481/index.json. La session d'éditeur ouverte a été conservée.
- Aucun rendu GPU, test de gestes réels, packaging ou mesure de performance réalisé. Ces couches restent explicitement à ajouter/valider.
