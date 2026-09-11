# Jalon 5 — Groupes et besoins hebdomadaires

Deux groupes fixes constituent la population du prototype : **Étudiants A (24 personnes)** et **Équipe basket (16 personnes)**. Le premier a besoin de trois cours universitaires de 2 h par semaine ; le second de deux entraînements de 2 h. Le planning conserve l'identifiant du groupe dans chaque réservation.

Le nouveau champ Groupe propose ces deux groupes et « Sans groupe etudiant ». Choisir l'activité propose automatiquement le groupe correspondant. Université exige Étudiants A ; entraînement exige Équipe basket. Cours publics et locations n'utilisent aucun de ces groupes étudiants. L'accès libre peut être affecté à l'un des deux groupes ou rester général ; il ne couvre pas leurs besoins de cours ou d'entraînement. Une affectation incompatible est refusée sans modifier l'original.

Les deux lignes au-dessus du planning affichent les heures prévues pour l'activité attendue, les objectifs en heures, le nombre de séances de 2 h et le nombre de séances manquantes. Lorsque le nombre de séances demandé est atteint, le statut devient « Objectif couvert ». Des heures supplémentaires sont autorisées. Une séance de 1 h, 3 h ou plus reste planifiable avec alerte, mais n'est pas équivalente aux séances de 2 h demandées. Six heures en un seul bloc ne remplacent donc pas trois séances.

Les compteurs se recalculent depuis les réservations enregistrées : un brouillon ne les modifie pas. Modification, changement de type, déplacement et suppression se répercutent sans double comptage. La préparation ne compte jamais dans ces heures. Tous les blocages du terrain et de sa préparation restent actifs. Avec une seule zone, leur interdiction empêche aussi d'y affecter deux fois un groupe simultanément ; l'agenda partagé entre plusieurs installations viendra avec leur prise en charge.

## Tests en jeu

Arrêter Play, faire **Ctrl + Alt + F11**, attendre le chargement du patch, puis relancer **Play → P**. Commencer avec un planning vide et garder le jeu en pause.

| Test | Manipulation | Résultat attendu |
|---|---|---|
| Besoins initiaux | Ouvrir P | Étudiants A : 0/6 h, 0/3 séances ; Équipe : 0/4 h, 0/2 séances ; besoins manquants visibles |
| Affectation par défaut | Sur lundi 08:00 choisir Université | Groupe Étudiants A proposé |
| Université complète | Créer Université lundi, mercredi et vendredi 08–10 | Étudiants : 6/6 h, 3/3 séances, Objectif couvert |
| Équipe complète | Créer Entraînement lundi et jeudi 14–16 | Groupe Équipe proposé ; 4/4 h, 2/2 séances, Objectif couvert |
| Affectation incompatible | Sélectionner un cours universitaire, choisir Équipe, Enregistrer | Refus explicite ; le cours conserve Étudiants A et les compteurs restent inchangés |
| Durée risquée | Recharger le cours du lundi ; passer sa durée à 4 h puis Enregistrer | Alerte avant enregistrement ; autorisé, 8 h prévues mais seulement 2/3 séances de 2 h, une séance manque |
| Annulation du brouillon | Modifier une autre durée sans enregistrer, puis Annuler les changements | Aucune modification des besoins ou des réservations |
| Retour à l'objectif | Remettre le cours du lundi à 2 h et enregistrer | 6/6 h et 3/3 séances, Objectif couvert |
| Accès libre exclu | Créer mardi 16–18 en Accès libre et lui affecter Étudiants A | Accepté ; l'université reste à 6 h et 3 séances, pas 8 h |
| Déplacement | Déplacer un entraînement de jeudi vers samedi 14–16 | Besoins de l'équipe toujours couverts ; ancienne case libre |
| Suppression | Supprimer un cours universitaire et confirmer | Étudiants : 4/6 h, 2/3 séances, une séance manque ; équipe inchangée |
| Autres activités | Créer dimanche 18–20 Cours public, sans groupe étudiant | Préparation 17–18 ; aucun crédit ajouté aux objectifs des groupes |
| Conservation | Fermer/rouvrir P puis consulter une séance | Groupe et compteurs conservés dans la session |
| Navigation | Faire défiler la grille, modifier des champs, fermer P et déplacer la caméra | Dernières lignes accessibles ; contrôles de caméra inchangés après fermeture |

## Vérification et limites

Tests natifs et compilation du jeu réussis (`Saved/Logs/BuildJalon5.log`). Le chargement Live Coding et les interactions visuelles restent à vérifier dans Unreal.

Le test natif du planning vérifie objectifs initiaux, identifiants de groupe invalides, compatibilité, progression, distinction heures/séances, annulation d'une modification invalide, déplacements, suppressions et exclusion de l'accès libre. Les tests des jalons 3 et 4 sont exécutés avec lui.

Ce suivi est celui de la **semaine type planifiée**, pas des heures réellement assurées. Faire avancer la simulation n'attribue pas de résultats et ne remet pas ce modèle récurrent à zéro. Il n'y a pas encore de personnages, de déplacements de groupes, de recrutement ni de sauvegarde entre sessions Play. Les réservations sans enseignant sont possibles à ce stade ; le personnel arrive au jalon 6.

Prochain jalon : **6 — Personnel et affectations**, avant l'économie.
