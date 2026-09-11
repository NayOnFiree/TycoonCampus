# Transitions entre outils — 11 septembre 2026

Jalon actuel : stabilisation des jalons 9–12. Ce sous-jalon est empilé sur la commande des chemins (PR #4), sans autre mécanique ni changement visuel.

## Cartographie de départ

`CampusCameraPawn` portait deux booléens indépendants pour construction et chemins. Le HUD détenait séparément le menu et le widget planning ; plusieurs callbacks annulaient eux-mêmes les outils. Échap passait par le pawn, le préprocesseur du HUD et les widgets de menu/planning, en ouvrant systématiquement le menu. Les chemins vérifiaient le widget sous le curseur, contrairement à la sélection et à la construction. Le chargement réinitialisait directement les champs privés du pawn.

## Contrat livré

`ECampusToolMode` représente un mode exclusif : sélection, construction, chemins, planning ou menu. `FCampusToolMode::Next` définit les transitions sans dépendance Unreal. Le pawn détient le mode et applique les transitions dans `SetToolMode` ; le HUD reçoit uniquement la demande d'afficher/fermer les fenêtres et continue de gérer leurs widgets et leur focus. Personnel et finances restent des pages du mode planning.

| Action | Depuis le terrain | Depuis le planning | Depuis le menu |
|---|---|---|---|
| B / Gymnase | Active construction, ou revient à sélection si déjà actif | Sans effet | Sans effet |
| C / Chemins | Active chemins, ou revient à sélection si déjà actif | Sans effet | Sans effet |
| P / Planning | Annule l'outil et ouvre le planning si le gymnase est construit | Ferme le planning | Sans effet |
| Menu explicite | Annule l'outil et ouvre le menu | Ferme le planning et ouvre le menu | Ferme le menu |
| Échap | Annule construction/chemins ; en sélection, ouvre le menu | Ferme le planning | Ferme le menu |

Une fenêtre de planning indisponible ramène à la sélection avec le message existant. Fermer une fenêtre ne restaure jamais un ancien outil ou tracé. Le menu conserve son comportement temporel : ouverture en pause, fermeture avec Échap en restant en pause, bouton Reprendre relançant le temps.

Toute transition nettoie les deux ancres du glisser et les validités d'aperçu, arrête la rotation et remet à zéro le déplacement. Le masquage du curseur lors de la capture est ajusté immédiatement : désactivé pour chemins, restauré à leur sortie. X annule le rectangle courant avant de changer achat/suppression. Le clic droit et la perte de focus conservent leur annulation du tracé. Les rectangles, la précision Maj, la rotation du placement et les commandes économiques sont inchangés.

La sélection, la construction et le départ du glisser passent par le même garde de clic monde : mode non modal, curseur sur le viewport du terrain et absence de rotation. Un clic identifié UI invalide le geste provisoire sans sélectionner, construire ni démarrer un tracé. Un relâchement ultérieur ne peut donc pas valider cet ancien tracé. L'aperçu de construction et le démarrage de rotation refusent aussi le curseur sur une UI. Le hit-test Slate utilisé est celui déjà présent pour les chemins.

`CancelTools()` conserve une modale ouverte tout en nettoyant les gestes ; le chargement ferme déjà le planning avant cet appel et garde le menu ouvert. Le raccord dans `CampusSaveService.cpp` remplace uniquement les anciennes affectations d'état d'outils. Aucun champ sauvegardé, codec, règle économique ou format n'est modifié.

## Périmètre coordonné

Accord d'intégration obtenu avant modification de `CampusCameraPawn.h/.cpp`, du routage et cycle de fenêtres de `CampusHUD.h/.cpp`, de l'Échap de `SCampusPlanningPanel.cpp` et du seul nettoyage d'outils dans `CampusSaveService.cpp`. Aucune modification du HUD visuel, de `CampusBuilding`, des assets, des configurations ou d'Avancement. Les nouveaux fichiers du domaine sont le modèle de mode, sa suite native et ce document ; le scénario Unreal existant est étendu.

## Validation

Huit suites natives réussies avec `pwsh -NoProfile -File Scripts/Test-Native.ps1`. Rapport : `Saved/Tests/Native/ef0b5cc94b5f439c8982e3801941a84a/results.json`. La nouvelle suite couvre les trente transitions de la matrice, les séquences Échap et le refus des clics UI, modaux ou pendant une rotation.

Le scénario Unreal vérifie les modes et les fenêtres, les ouvertures depuis les boutons, le planning indisponible avant construction, les annulations d'ancres/rotation, les pages personnel/finances, la fermeture sans réactivation d'outil, le maintien du menu pendant le nettoyage et la pause à sa fermeture. Pour chacun des cinq modes, il injecte la classification « clic UI » dans le même gestionnaire que l'entrée réelle, puis un relâchement : grille, modèle économique/journal et sélection restent inchangés, et le geste provisoire est invalidé.

Compilation Editor réussie sur la copie isolée existante, dont Source a été synchronisé. Rapport de compilation : `Saved/Tests/Unreal/1bc2b23dd31f439fb4adad9657c76213/Project/Saved/Tests/Unreal/268a2fdffdda44dd8dc657cafc5ecb4a/build.log`. Les deux premiers lancements ont expiré avant chargement de la carte et avant toute assertion, à 180 puis 600 secondes. Journaux `268a2fdffdda44dd8dc657cafc5ecb4a/runtime.log` et `423c8cf91dac42cb881658c9422c2ae2/runtime.log` sous le même dossier de rapports. Une compilation/test Unreal d'une autre tâche était simultanément active ; contention suspectée, cause exacte non démontrée. Les scripts ont arrêté uniquement leurs propres processus de test. Dernière relance effectuée seule après libération du moteur, avec -SkipBuild -TimeoutSeconds 600 : scénario TycoonCampus.Runtime.BootAndPanels réussi (1 succès, 0 échec, 0 non exécuté), sortie code 0. Rapport : Saved/Tests/Unreal/1bc2b23dd31f439fb4adad9657c76213/Project/Saved/Tests/Unreal/236c8b785356442197ae993a86691291/index.json. Processus terminé, créneau moteur libéré. La carte LFS de cette copie était hydratée (45 111 octets).

Limite explicite : NullRHI et l'injection de la classification UI ne valident pas le hit-test Slate avec une vraie souris, la capture Windows ni le rendu. La session du joueur et sa DLL sont conservées. Essais interactifs à effectuer : B/C puis Échap ; glisser vers une fenêtre et relâcher ; cliquer le HUD dans chaque outil ; ouvrir/fermer planning et menu puis refaire clic simple, rotation et chemins avec Maj.

Prochain sous-jalon : observation interactive de ces transitions et du filtrage souris après intégration des PR empilées. Aucun nouveau panneau, style ou bâtiment engagé.
