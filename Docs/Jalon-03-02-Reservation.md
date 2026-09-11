# Jalon 3.2 — Première réservation

Dans le planning (P), cliquer sur une case libre. Le jour et le début sont ceux de la case. Choisir l'activité et la durée dans les listes, puis cliquer **Créer la réservation**. Toutes les heures concernées affichent l'activité avec une couleur associée. Cliquer sur n'importe quelle heure occupée affiche les bornes complètes de la réservation.

Activités : cours universitaires, entraînement, accès libre, cours public et location club. Durée : de 1 à 14 heures, dans l'ouverture 08:00–22:00. Deux réservations adjacentes sont acceptées ; un chevauchement, même partiel, est refusé avec explication. Une erreur ne remplace ni ne modifie une réservation existante.

Les réservations sont stockées dans le bâtiment, avec un identifiant stable dans la session, indépendamment de la fenêtre. Fermer et rouvrir P les conserve. Elles définissent une semaine type ; leur exécution en simulation n'est pas encore branchée. Arrêter Play réinitialise les données : la sauvegarde entre parties arrive au jalon prévu.

Cette étape n'ajoute pas encore groupe, personnel, prix, préparation, modification ou suppression. Prochaine étape 3.3 : modifier et supprimer une réservation existante. Les contraintes plus détaillées arrivent ensuite.

## Validation

Correctif Live Coding : suppression du stockage `std::vector`, qui introduisait le symbole externe non résolu `std::_Xlength_error` pendant la liaison du patch. Le stockage utilise désormais un tableau borné aux 98 créneaux possibles par zone (7 jours × 14 heures), avec compteur et contrôle de capacité. Ce plafond correspond aux horaires du prototype, et non au nombre de réservations de tout le campus. Le test remplit également les 98 créneaux et vérifie le refus d'une réservation supplémentaire.

Le modèle C++ utilisé par le bâtiment est testé directement dans `Tests/CampusWeeklyScheduleTests.cpp`. Tests réussis : création, occupation de toute la durée, intervalles adjacents, doublons, inclusion et chevauchement, indépendance des jours, dernière heure d'ouverture, réservation de journée entière, valeurs invalides et identifiants non consommés par les erreurs.

Essai en jeu : créer lundi 10:00–12:00, vérifier les deux cases ; créer 12:00–13:00 pour vérifier l'adjacence ; essayer 09:00–11:00 pour vérifier le refus du chevauchement ; essayer 21:00–23:00 pour vérifier la fermeture ; fermer/rouvrir P et consulter la réservation. Compilation et tests du modèle ne remplacent pas cet essai de l'interface.
