# Jalon 12 — Confort de jeu et interface de gestion

Ce jalon transforme l'interface du prototype sans changer les règles de planning, d'économie, de déplacement ou le format de sauvegarde du jalon 11.

## Livré

- Barre supérieure : trésorerie, résultat de semaine, calendrier, état pause/marche et commandes de vitesse à la souris.

- Navigation : gymnase, chemins, planning, personnel, finances, menu et alertes. Les raccourcis précédents restent disponibles.

- Fiche du gymnase sélectionné : activité actuelle, prochaine séance planifiée, occupation, préparation, minutes de pratique réalisées et raccordement.

- Planning : blocs sur la durée entière des réservations, préparation séparée, formulaire à droite avec défilement. Le clic sur un bloc ouvre la réservation entière.

- Personnel et contrats : recrutement, affectations cumulées, état du contrat et accès à une séance à corriger.

- Finances : tableau comparatif des semaines, trésorerie et investissements séparés, journal en colonnes.

- Menu Échap : pause, reprise, sauvegarde, chargement, copie précédente, rappel des commandes et sortie avec confirmation.

- Consolidation des interactions : annulation des placements provisoires à l'ouverture d'une fenêtre, arrêt de la rotation, nettoyage des widgets et de la gestion d'Échap en fin de session. Les anciennes bandes du HUD sont remplacées par les nouveaux contrôles.

## Fonctionnement du menu

Ouvrir le menu met la simulation en pause. **Reprendre la partie** relance le temps ; refermer avec **Échap** reste en pause. Charger reste également en pause. Les boutons de chargement remplacent les changements non sauvegardés, comme F9/F10 auparavant. Le menu ne sauvegarde pas automatiquement.

Échap est réservé au menu lorsque la fenêtre de jeu a le focus ; les autres fenêtres de l'éditeur conservent leurs commandes. Pour arrêter un essai dans l'éditeur, utiliser le bouton Stop de l'éditeur ou la sortie de session du menu.

Les changements de formulaire non validés ne font pas partie de la sauvegarde. Ouvrir le menu ferme le formulaire. Valider une réservation avant de quitter son édition.

## Tests à faire

Arrêter Play, lancer **Ctrl + Alt + F11**, attendre la réussite du chargement du patch, puis relancer **L_Campus_Work**. Une sauvegarde du jalon 11 peut être chargée avec F9.

1. **Navigation sans raccourcis** : utiliser Gymnase et Chemins pour construire et raccorder. Cliquer Pause/Reprendre et x1/x2/x4. Vérifier que cliquer les contrôles ne place rien sur le terrain.

2. **Fiche du gymnase** : sélectionner le bâtiment d'un clic. Vérifier la fiche, ouvrir son planning, fermer avec P. Fermer la fiche puis sélectionner à nouveau le même bâtiment : elle doit réapparaître.

3. **Blocs du planning** : créer lundi 10–12 h, puis un cours public 18–20 h. Vérifier le bloc de deux heures et la préparation 17–18 h. Modifier un bloc vers un autre jour, puis le supprimer. Vérifier que l'ancien emplacement redevient disponible. Les conflits doivent toujours être refusés.

4. **Alertes** : créer un cours sans encadrant. Avec un gymnase raccordé, le bouton Alertes doit ouvrir une séance à corriger. Recruter Camille dans Personnel puis l'affecter. Sans raccordement, Alertes ouvre l'outil chemins. S'il n'y a aucune de ces alertes, le planning l'indique.

5. **Rapports** : laisser finir une séance et consulter Finances. Les lignes doivent apparaître une seule fois ; comparer les montants avec le solde supérieur. Les investissements ne sont pas inclus dans le résultat d'exploitation. La semaine précédente reste « -- » avant le premier bilan.

6. **Menu** : en marche, ouvrir Menu ou Échap. L'heure doit s'arrêter. Tester sauvegarde, chargement, reprise et annulation de la sortie. Échap referme le menu en restant en pause. La caméra ne doit ni tourner ni zoomer pendant que le menu est ouvert.

7. **Retour au terrain** : après avoir utilisé boutons et menus, tester ZQSD, clic simple, clic droit et molette. Refaire après Stop/Play et après F9.

8. **Lisibilité** : vérifier le planning, son formulaire défilant, la fiche et le menu dans une fenêtre de jeu 1280×720 puis 1920×1080. Signaler tout texte coupé ou bouton inaccessible.

## Limites et suite

La validation visuelle dans Unreal reste à faire par le joueur. Il s'agit d'une première interface de gestion, avec les bâtiments et figurants provisoires existants. Le jeu conserve un seul gymnase. Le menu principal de démarrage, les options graphiques, plusieurs sauvegardes nommées et la gestion de plusieurs installations ne font pas partie de ce jalon.

Prochaine étape proposée : valider cette interface et corriger les difficultés d'utilisation observées, puis décider du menu principal ou de l'équilibrage. Aucun jalon suivant n'est lancé automatiquement.


Validation technique : compilation Unreal réussie (`Saved/Logs/BuildJalon12.log`). Les six suites natives sauvegarde, planning, horloge, économie, construction et chemins passent. L’affichage et les interactions dans l’éditeur n’ont pas été vérifiés automatiquement.
