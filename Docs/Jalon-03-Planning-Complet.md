# Jalon 3 — Planning hebdomadaire complet

Périmètre livré : consultation de la semaine type, cinq activités, création, modification de l'activité/du jour/de l'heure/de la durée, suppression confirmée, prévention des chevauchements et total des heures planifiées. Le planning concerne le terrain entier du gymnase préplacé, ouvert de 08:00 à 22:00.

Cliquer sur une heure occupée sélectionne toute la réservation et préremplit ses quatre champs. Les champs sont un brouillon : **Enregistrer** applique les changements ensemble. Une erreur conserve intégralement l'original. **Annuler les changements** recharge les valeurs enregistrées. Changer de case ou fermer P abandonne aussi les changements non enregistrés. Une modification conserve l'identifiant de la réservation.

**Supprimer** demande une confirmation dans le panneau. **Garder la reservation** annule cette demande. **Confirmer la suppression** libère toutes les heures de la séance. Les identifiants supprimés ne sont pas réutilisés pendant la session.

Le compteur du haut affiche le nombre de réservations, leurs heures cumulées et leur part des 98 heures disponibles. Il mesure le planning prévu, pas la fréquentation réelle. Les conflits identifient l'activité, le jour et les horaires de la réservation déjà présente. Les séances adjacentes sont permises à ce jalon.

## Procédure de test en jeu

Arrêter Play avant de compiler avec **Ctrl + Alt + F11**. Attendre la réussite du chargement du patch Live Coding, pas seulement le premier message « Result: Succeeded ». Relancer Play puis appuyer sur **P**. Commencer avec une semaine vide et garder le jeu en pause pour ces essais.

| Test | Manipulation | Résultat attendu |
|---|---|---|
| Création | Cliquer lundi 10:00, laisser Université, choisir 2 h, puis Créer | Lundi 10–12 occupé ; compteur 1 réservation, 2 h |
| Adjacence | Cliquer lundi 12:00, choisir Entraînement, 1 h, puis Créer | Accepté ; compteur 2 réservations, 3 h |
| Chevauchement | Cliquer lundi 09:00, choisir 2 h, puis Créer | Refus identifiant le cours de 10–12 ; compteur toujours 2 et 3 h |
| Sélection complète | Cliquer lundi 11:00 | Les deux heures du cours sont sélectionnées ; formulaire début 10:00, durée 2 h |
| Modification et déplacement | Dans ce formulaire, choisir mardi, 14:00, 3 h et Cours public, puis Enregistrer | Lundi 10–12 libre ; mardi 14–17 occupé ; compteur 2 réservations, 4 h |
| Refus sans perte | Modifier cette séance vers lundi 11:00, 2 h et Enregistrer | Refus à cause de l'entraînement 12–13 ; la séance reste mardi 14–17 |
| Annulation du brouillon | Cliquer Annuler les changements | Le formulaire retrouve mardi, 14:00, 3 h, Cours public |
| Fermeture | Essayer de modifier cette séance à 21:00 pour 2 h | Refus après 22:00 ; mardi 14–17 conservé. Annuler les changements |
| Suppression annulée | Cliquer Supprimer puis Garder la reservation | La séance reste présente |
| Suppression confirmée | Cliquer Supprimer puis Confirmer la suppression | Mardi 14–17 libre ; seul l'entraînement demeure, compteur 1 réservation, 1 h |
| Réutilisation et cinq usages | Créer mardi 14–15 en Accès libre, mercredi 18–19 en Cours public, jeudi 20–22 en Location club et vendredi 08–10 en Université | Toutes acceptées ; avec l'entraînement, les cinq usages sont présents : 5 réservations, 7 h |
| Conservation en session | Fermer P, rouvrir P et sélectionner jeudi 21:00 | Planning conservé ; détail de la location complète 20–22 |
| Commandes | Dans P : Espace puis touches 1/2/3 (ou pavé numérique) ; fermer P et essayer ZQSD, rotation et molette | Pause/vitesses fonctionnent ; caméra bloquée dans P et disponible après fermeture |
| Affichage | Réduire la fenêtre et faire défiler le planning | Accès aux sept jours et à la dernière ligne 21:00 ; champs et boutons accessibles |

Les réservations sont un modèle récurrent conservé pendant la session Play. Arrêter Play les efface actuellement. L'exécution des activités, l'historique, les obligations et la sauvegarde ne sont pas simulés par ce jalon. Il n'y a donc pas encore de restriction de modification liée à une séance en cours.

## Vérification technique

Tests du modèle réussis et compilation du jeu réussie : `Saved/Logs/BuildJalon3.log`. Le chargement du patch Live Coding dans l'éditeur et les interactions visuelles restent à valider avec la procédure ci-dessus.

Le test natif `Tests/CampusWeeklyScheduleTests.cpp` vérifie création, bornes, chevauchements, semaine pleine, déplacement, modification qui recouvre son ancien horaire, rejet sans mutation, identification du conflit, suppression en début/milieu/fin, réutilisation des cases, identifiants et compteurs. Le modèle n'utilise pas `std::vector`, pour éviter la dépendance de liaison rencontrée avec Live Coding.

Prochain jalon : **4 — Disponibilité**, en commençant par l'heure de préparation avant les cours publics et les locations, son affichage et les conflits correspondants. Groupes, personnel et économie suivent leur ordre de développement.
