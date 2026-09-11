# Jalon 4 — Disponibilité du gymnase

Une heure de préparation est automatiquement réservée avant chaque cours public et chaque location club. Université, entraînement et accès libre n'en nécessitent pas. La durée saisie reste celle de l'activité. Préparation et activité doivent entièrement tenir entre 08:00 et 22:00 : un cours public peut commencer à 09:00 au plus tôt.

La préparation apparaît dans la grille avec le libellé « Preparation » et une couleur distincte. Cliquer dessus sélectionne la réservation associée : le formulaire indique le début de l'activité, pas celui de la préparation. Le détail indique les deux intervalles ; le brouillon explique l'occupation proposée avant enregistrement.

Les chevauchements sont refusés entre activités, entre préparations, et entre activité et préparation, quel que soit l'ordre de création. Une activité peut finir exactement au début d'une préparation. Une erreur conserve la réservation initiale. Déplacer, supprimer ou changer le type de la séance recalcule son occupation sans laisser de préparation isolée.

Le bilan sépare heures d'activité et leur pourcentage utile, heures de préparation, puis total réservé et son pourcentage sur 98 h. La préparation ne crée pas une deuxième réservation et n'est jamais comptée comme pratique sportive.

## Tests dans Unreal

Arrêter Play, compiler avec **Ctrl + Alt + F11**, attendre le chargement réussi du patch Live Coding, puis relancer **Play → P**. Partir d'une semaine vide et rester en pause pour les essais.

| Test | Action | Résultat attendu |
|---|---|---|
| Préparation visible | Créer lundi 18:00–20:00, Cours public | Préparation 17–18, activité 18–20 ; compteur 1 réservation, 2 h d'activité, 1 h de préparation, 3/98 h au total |
| Clic sur la préparation | Cliquer lundi 17:00 | La séance entière est sélectionnée ; formulaire début 18:00, durée 2 h |
| Empiéter sur la préparation | Cliquer lundi 16:00 et créer Université pour 2 h | Refus ; le cours public reste intact |
| Limite autorisée | Sur ce brouillon, réduire la durée à 1 h et créer | Université 16–17 acceptée, juste avant la préparation |
| Nouvelle préparation en conflit | Sur une case libre, choisir lundi 20:00, Location club, 2 h | Refus : sa préparation 19–20 empiète sur le cours public |
| Annulation | Sélectionner le cours public, changer le début à 19:00, puis Annuler les changements | Retour à 18:00 ; préparation toujours 17–18 |
| Déplacement | Déplacer le cours public vers mardi 18:00 et enregistrer | Lundi 17–20 libéré ; préparation mardi 17–18, activité mardi 18–20 |
| Changement d'usage | Passer cette séance mardi en Accès libre et enregistrer | Mardi 17–18 redevient disponible ; activité 18–20 conservée |
| Ajout de préparation refusé | Créer Université mardi 17–18, puis tenter de repasser la séance 18–20 en Cours public | Refus ; la séance reste en accès libre |
| Ouverture | Sur mercredi, essayer Location club à 08:00 pour 2 h, puis à 09:00 | 08:00 refusé ; 09:00 accepté avec préparation 08–09 et activité 09–11 |
| Fermeture | Créer jeudi Location club 20–22 ; essayer ensuite de passer sa durée à 3 h | Création acceptée avec préparation 19–20 ; modification refusée, original conservé |
| Suppression liée | Cliquer jeudi 19:00 puis Supprimer et Confirmer | Les trois cases 19–22 se libèrent ; aucun bloc de préparation ne reste |
| Conservation | Fermer et rouvrir P | Séances et préparations conservées dans la session |

Contrôle facultatif sur une nouvelle session : programmer la semaine de référence du jalon 0. Le bilan doit afficher 10 réservations, **20 h d'activité (20,4 %), 3 h de préparation et 23/98 h au total (23,5 %)**.

## Vérification et limites

Tests natifs et compilation du jeu réussis le 11 septembre 2026 (`Saved/Logs/BuildJalon4.log`). Le chargement Live Coding et la présentation dans l'éditeur restent à vérifier par les essais ci-dessus.

Les tests natifs du planning couvrent les deux sens de conflit, l'ouverture, les changements d'activité, le déplacement, la suppression et la semaine de référence. Un contrôle indépendant par cases horaires compare aussi toutes les paires d'activités d'une heure sur la journée.

Ce jalon réserve du temps de préparation ; il ne simule pas encore un agent d'entretien. Les réservations restent une semaine type sans exécution des activités ni sauvegarde entre sessions Play. Les tests de l'ancien jalon 3 impliquant des cours publics ou locations doivent désormais tenir compte de l'heure préparatoire.

Prochain jalon : **5 — Groupes et besoins hebdomadaires**, avant le personnel et l'économie.
