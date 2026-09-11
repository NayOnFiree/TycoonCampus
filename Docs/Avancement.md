# Avancement — Tycoon Campus

Préférence du joueur : à chaque livraison, indiquer le jalon actuel, ce qui est validé ou reste à tester, et le prochain jalon/sous-jalon. Développer une mécanique à la fois. Une compilation réussie ne vaut pas validation en jeu.

| Jalon | État |
|---|---|
| 0 — Conception | Documents et maquettes livrés |
| 1 — Carte, caméra et sélection | Validé par le joueur, y compris zoom, rotation, clic simple et repère amélioré |
| 2.1 — Horloge et pause | Validé par le joueur |
| 2.2 — Vitesses x1/x2/x4 | Livré ; nouveau rythme de base demandé par le joueur : journée de 5 minutes en x1 |
| 2.3 — Transitions du calendrier et événements horaires | Tests et compilation réussis ; jalon 2 accepté par le joueur |
| 3.1 à 3.4 — Planning complet | Tous les tests en jeu validés par le joueur le 11 septembre 2026 |
| 4 — Disponibilité | Tous les tests en jeu validés par le joueur |
| 5 — Groupes et besoins hebdomadaires | Tous les tests en jeu validés par le joueur |
| 6 — Personnel et affectations | Recrutement de Camille, habilitations, disponibilité, affectations, charge et alertes implémentés. Tests natifs et compilation du jeu réussis ; validation Live Coding et en jeu à faire |
| 7 — Économie | Exécution logique, journal, trésorerie, charges, salaire, prix/demande, contrat ponctuel et bilans implémentés. Tests natifs réussis, dont la semaine à +480 EUR ; compilation réussie (BuildJalons6-7.log). Validation Live Coding et en jeu à faire. Guide : Jalons-06-07-Personnel-Economie.md |
| 8 — Construction sur grille | Validé par le joueur |
| 9 — Chemins et raccordement | Tracé en deux clics, suppression, achat atomique, grille, parcours, raccordement et retards implémentés. Tests natifs et compilation du jeu réussis (BuildJalons9-10.log) ; chargement Live Coding et essais en jeu à faire |
| 10 — Usages visibles | Figurants simplifiés, parcours, activités, pause et distinction participants/figurants implémentés. Compilation du jeu réussie ; chargement Live Coding et validation visuelle à faire. Guide : Jalons-09-10-Chemins-Usages.md |
| 11 — Sauvegarde et chargement | Validé par le joueur |
| 12 — Confort de jeu et interface | Implémenté : barre cliquable, fiche, planning en blocs, personnel, finances et menu. Six suites natives réussies. Compilation Unreal réussie (BuildJalon12.log) ; Live Coding et essais visuels à faire. |

Prochaine étape : validation de cette interface avant de décider de la suite. Aucun menu principal ni multi-bâtiment lancé. Guide : Jalon-12-Confort-Interface.md

## Refonte visuelle du jalon 12

Maquette V1 acceptée par le joueur. Barre bleu nuit, dock inférieur, fiche claire et icônes intégrés dans Slate. Compilation Unreal réussie (BuildHUDVisualV1.log) ; chargement Live Coding et comparaison visuelle à faire en jeu. Guide : Design/HUD-Integration-V1.md. Aucun nouveau jalon lancé.

## Nouvelle direction universitaire

Demande du joueur : refonte complète, en commençant seulement par le HUD pour validation. Inventaire réalisé ; menu principal absent et à créer après validation. HUD marron/crème/bordeaux intégré, contraste vérifié et compilation Unreal réussie (BuildAcademicHUD.log). Validation visuelle en jeu attendue. Planning, personnel, contrats, finances, menu Échap et confirmation de sortie gardent leur style actuel. Voir Design/Refonte-Universitaire.md. Attendre la validation avant la seconde étape.

Correction HUD après capture : fonds remplacés, fiche marron opaque bordée, textes crème, boutons opaques et composition révisée. Compilation cible jeu réussie (BuildHUDOpaque.log). Chargement éditeur et validation visuelle non effectués : capture Windows indisponible et déclenchement Live Coding distant non confirmé. Style non validé ; autres écrans en attente.

## Confort des chemins — reprise du jalon 9

Interface mise en attente à la demande du joueur. Outil de chemins : cases visibles de 4 m, glisser rectangulaire rempli, validation au relâchement, suppression identique, aperçu et prix. Maj garde une précision de 2 m pour les anciens raccords. Sauvegardes et coordonnées existantes conservées. Grille de construction agrandie et pas de placement adapté. Six suites natives réussies, puis tests des rectangles et de la précision réussis. Compilation finale Unreal réussie (Saved/Logs/BuildPathRectangles.log). Chargement Live Coding et geste en jeu restent à vérifier. Prochaine étape : validation de ces commandes, puis choix du joueur ; menus en attente.

## Campus — 01 / heritage

Direction validée par le joueur : maquette locale, révision compacte. HUD natif remplacé dans CampusHeritageHUD.cpp ; ressources haut gauche, navigation bas gauche, horloge bas droite, fiche 252 et objectif réel. Menus et planning inchangés avant validation de cette intégration. Vérification en jeu bloquée par la capture Windows ; aucun rendu présenté comme validé. Guide : Design/Heritage-HUD-Integration.md.

Compilation finale du HUD heritage réussie : BuildHeritageHUD.log. Polices via FCompositeFont, fonds chargés dynamiquement. Aucun essai du nouveau HUD dans l’éditeur ni capture Unreal obtenus ; validation visuelle explicitement en attente.

## Correction du glisser des chemins

Le maintien du clic gauche utilisait la capture avec masquage du curseur. Le code moteur FSceneViewport::OnMouseMove recentre alors le pointeur à chaque mouvement, ce qui empêche un rectangle de suivre la position absolue. Le mode chemins désactive désormais ce masquage avant le clic, en conservant CaptureDuringMouseDown pour recevoir le premier appui. Le masquage caméra est restauré à la sortie du mode. Journaux ponctuels de début et validation ajoutés. Compilation : BuildPathDragFix.log ; test manuel du geste nécessaire.

## Diagnostic de la fiche Gymnase 01

Correction isolée demandée après capture : compensation locale de la courbe DPI, valeurs sans retour à la ligne, fonds et boutons via brosses natives à opacité explicite. Cause exacte des anciens fonds absents non confirmée ; diagnostic runtime ajouté. Contours rectilignes provisoires. Autres panneaux inchangés. Aucun rendu corrigé observé : capture Windows bloquée. Voir Design/GymInspector-Diagnostic.md.

## Audit architecture et automatisation — 11 septembre 2026

Jalon actuel : stabilisation transversale des jalons 9 à 12, sans nouvelle mécanique. Audit livré dans Audit-Architecture-Qualite.md : forces, risques de couplage, architecture cible progressive, pratiques gameplay/UI/graphisme et matrice de tests.

Sept suites natives recompilées et réussies via Scripts/Test-Native.ps1, dont une nouvelle intégration construction/chemins/planning/économie/sauvegarde, pause et déterminisme sur 16 semaines. Rapport : Saved/Tests/Native/08516bb33a014e4388b0083801b387c5/results.json.

Compilation Editor et test TycoonCampus.Runtime.BootAndPanels réussis sur une copie isolée via Scripts/Test-Unreal.ps1 -Isolated, pour éviter la DLL verrouillée par l'éditeur ouvert. Vérifie démarrage, acteurs uniques, caméra, pause, menu, planning interdit avant construction, achat du gymnase et ouverture/fermeture du planning. Rapport : Saved/Tests/Unreal/de6c82a138af43298ee786a9717c1481/index.json. Aucune sauvegarde joueur modifiée. Ce test utilise NullRHI et appelle les actions directement : rendu et gestes réels non validés.

Prochain sous-jalon recommandé : validation du HUD et du glisser des chemins, puis commandes de jeu communes à l'UI et aux raccourcis. Tests d'entrées, sauvegarde disque isolée, références visuelles, performances, packaging et CI restent à compléter. Aucun nouveau style ni refactorisation du gameplay effectué pendant l'audit.

## Commande de construction — parcours accepté, extraction livrée

Le joueur confirme le parcours proposé comme bon et autorise l'étape suivante : construction, chemins, séance, sauvegarde/reprise et panneaux sont acceptés pour la version précédente. Jalon actuel : stabilisation architecture des jalons 9–12.

Commande FCampusConstructionService extraite : aperçu et exécution utilisent la même validation, avec contrôle du budget et du terrain au moment de l'achat. Le bouton et la touche B empruntent le même outil, qui appelle cette commande. L'application sur le bâtiment est privée ; coût partagé, résultats explicites, aucune modification du format des sauvegardes. Voir Construction-Commande.md.

Validation : sept suites natives réussies (Saved/Tests/Native/fb8780ecf01e4e85a5ce7d6f1cbabdc6/results.json). Compilation Editor et scénario Unreal étendu réussis sur la copie de test existante : hors parcelle, entrée hors parcelle, orientation invalide, chemin apparu après aperçu, budget insuffisant, refus sans mutation, achat et double achat interdit, menu et planning. Rapport : Saved/Tests/Unreal/de6c82a138af43298ee786a9717c1481/Project/Saved/Tests/Unreal/e20e6f32123f4767a2fc764ca593348a/index.json.

La session d'éditeur du joueur n'a pas été fermée et n'a pas reçu la nouvelle DLL. Charger cette version dans la session habituelle nécessite une compilation Editor après fermeture, puis réouverture. Les tests moteur utilisent NullRHI ; ils ne constituent pas un nouvel essai des gestes souris.

Prochain sous-jalon : extraire la commande d'achat/suppression des chemins selon le même principe, puis centraliser les transitions entre outils. Pas de deuxième bâtiment lancé.

## Git, collaboration et CI — 11 septembre 2026

Dépôt public NayOnFiree/TycoonCampus initialisé et publié après autorisation explicite du joueur. Git LFS suit les assets Unreal et médias ; caches, sauvegardes, worktrees et runner sont exclus. Un clone neuf et l'intégrité de la carte LFS ont été vérifiés. Le jeton Android File Server local est conservé sous Saved/LocalConfig et absent du dépôt ; ce service Android inutilisé est désactivé dans la configuration commune.

Trois branches et worktrees préparés : work/jouabilite, work/ui-ux, work/graphisme sous .worktrees. Les conversations correspondantes ne sont pas créées automatiquement. AGENTS.md, Docs/Collaboration.md, README, CODEOWNERS, modèle de PR et étiquettes par domaine définissent les responsabilités et l'intégration.

Main protégée : PR, Native tests requis et à jour, résolution des conversations, historique linéaire, aucun force-push ni suppression ; protection appliquée aussi aux administrateurs. Fusion squash ; aucun second approbateur imposé au développeur solo. Les branches des domaines sont conservées après fusion.

Tests : sept suites réussies sur Windows après adaptation du lanceur à PowerShell multiplateforme, puis workflow Native tests réussi sur Ubuntu GitHub avec archivage des rapports. Exécution : https://github.com/NayOnFiree/TycoonCampus/actions/runs/34608655310. Rapport local : Saved/Tests/Native/a9f4e4d0f65c4fc9a456d032449154dc/results.json.

Workflow Unreal integration configuré, manuel sur main, distinct de la CI native. Distribution officielle du runner Windows téléchargée et SHA256 vérifié sous .ci-runner. Après autorisation explicite du joueur, runner TycoonCampus-local enregistré pour ce dépôt et démarré en arrière-plan ; variable UE_ENGINE_ROOT définie. Première exécution GitHub réussie : compilation Editor, scénario moteur (1 succès, 0 échec) et archivage des rapports, https://github.com/NayOnFiree/TycoonCampus/actions/runs/34609101364. Script Start-UnrealRunner.ps1 fourni pour le relancer après redémarrage ; détection d'une instance déjà active vérifiée. Aucun service de démarrage Windows installé. La session Unreal du joueur est conservée.


Jouabilité : achat et suppression des chemins centralisés dans `FCampusPathService`,
transitions exclusives entre construction, chemins, sélection, planning et menu, puis
automatisation des vrais bindings clavier et de la frontière Slate qui bloque les clics
monde derrière un panneau. Le format de sauvegarde et les règles économiques restent
inchangés.

UI/UX : planning dimensionné depuis la géométrie réellement allouée par Slate. En espace
compact, la semaine et le détail alternent ; formulaire, refus et actions restent
accessibles et le focus rejoint le formulaire. Un harnais D3D12 produit une matrice de
captures en 1280 x 720 et 1920 x 1080 aux échelles UI 1,00 / 1,25 / 1,50, avec
métadonnées de géométrie. Les douze captures ont été observées. Le DPI de la fenêtre
mesuré était 1,00 : les échelles 1,25 / 1,50 stressent Slate mais ne simulent pas un
Windows réellement configuré à 125 / 150 %. Restent visibles : glyphe pause U+2161
absent, titre CAMPUS et symbole euro pouvant revenir à la ligne, fiche proche de
l'horloge en 720p à 150 %.

Graphisme : bible Heritage et pilote limités au gymnase. Quatre instances de matière
issues du matériau standard Unreal appliquent pierre chaude, entrée tabac, bois sportif
et socle sombre sans nouvel asset 3D, texture tierce ni Blender. Géométrie, collisions,
sélection, carte, chemins, simulation et sauvegarde sont conservés. Quatre comparaisons
blockout / Heritage en 720p et 1080p sont versionnées sous LFS. La dalle sombre reste à
apprécier en jeu ; gestes réels, quatre orientations et performances ne sont pas encore
mesurés.

Validation avant fusion : chaque pile a réussi ses suites natives, sa compilation Editor
et ses scénarios Unreal isolés ; les branches publiées ont une CI native verte. La branche
d'intégration réunit les trois piles pour un dernier passage natif et Unreal avant fusion
sur `main`. Aucun test isolé n'a fermé l'éditeur du joueur ni utilisé sa sauvegarde.

## Intégration des trois domaines — 11 septembre 2026

Mise à jour 11 septembre 2026 — préparation "construction multi-types" et harmonisation UI planning/finances :

- Côté jouabilité, la sélection de type de construction est portée par `ACampusHUD` (index, disponibilité, libellé) et le mode B utilise ce choix. Les 4 types (Gymnase, Terrain de foot, Terrain de tennis, Décoration) sont activables côté interface pour répondre à la demande de variantes. La mécanique réelle est encore unifiée (un seul comportement d’application de construction), en attente d’un suivi de géométrie propre aux types.
- Côté UI/UX, les boutons principaux de `SCampusPlanningPanel` (fermeture + changement de page + actions de réservation + cellule planning) passent par le style projet `CampusUI::Theme().Quiet` pour une tenue homogène avec le HUD Heritage.
- Côté graphisme, l'option de type de construction est ajoutée dans la barre d’outils Heritage avec style cohérent (`WhitePanelStyle`) et statut actif visible.

- Étape en cours (11/09/2026) : amélioration de la navigation planning/personnel/finances dans `SCampusPlanningPanel` (onglet actif visuel + boutons unifiés) et clarification du statut de construction avec le nom de type actif (`CampusCameraPawn.cpp`).
État de validation : neuf suites natives réussies (`Saved/Tests/Native/55557bc062eb47f492e81cb1d22fbc98`). Le scénario Unreal a réussi avec le cache DDC mémoire : 3 tests réussis (`TycoonCampus.Runtime.BootAndPanels`, `HeritageMaterials`, `ToolInput`) dans `Saved/Tests/Unreal/2e1ee96400374b6d97ad572f70f8c411`. La recompilation Editor locale reste bloquée par l'environnement .NET 10 absent.
