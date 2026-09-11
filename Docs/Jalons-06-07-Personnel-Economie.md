# Jalons 6 et 7 — Personnel, activités assurées et économie

## Livraison

Dans **P**, le bouton **Personnel et finances** ouvre une vue dédiée au recrutement, au contrat, à la trésorerie et au journal. **Retour au planning** permet de reprendre la programmation. La barre du campus affiche aussi la trésorerie et le résultat de la semaine en cours.

**Jalon 6.** Camille Martin est l'encadrante polyvalente du prototype : cours universitaires, entraînements et cours publics. Elle coûte 600 EUR par semaine et est disponible du lundi au vendredi, 08:00–20:00. Le recrutement est unique ; il ne prélève pas de frais immédiats. Le salaire complet est dû à la fin de chaque semaine où elle est employée, même recrutée en cours de semaine et même sans affectation. Pas de licenciement dans ce prototype. Le bilan affiche ses heures affectées et les séances sans encadrant.

Chaque réservation a un champ Personnel. Une affectation avant recrutement, hors disponibilité ou à une activité non encadrée est refusée. Une séance exigeant un encadrant peut rester sans affectation avec alerte ; elle est alors manquée, sans recette ni coût variable, sans heures assurées pour son groupe. L'accès libre et la location n'utilisent pas Camille. Dans ce gymnase à zone unique, le blocage des chevauchements du terrain empêche aussi les doubles affectations simultanées.

**Jalon 7.** Les occurrences du planning sont maintenant exécutées logiquement. À leur début, leur durée, personnel, groupe, prix et contrat sont figés pour cette occurrence. À la fin, le moteur enregistre participation, heures assurées, recettes et dépenses. Les animations et les trajets arrivent plus tard. Une séance sans public est manquée ; les capacités limitent les participations. Les heures de préparation ne sont ni des heures sportives ni des heures supplémentaires de Camille.

La modification ou la suppression d'un modèle n'annule pas une séance déjà commencée. Une réservation commencée ne se rejoue pas une deuxième fois pendant la même semaine après déplacement. Une nouvelle réservation qui tombe pendant une occurrence conservée en cours est manquée avec motif dans le journal. Les changements de modèle concernent les occurrences futures ; pour corriger une séance à venir cette semaine, modifier avant son heure de début.

## Règles financières du prototype

| Élément | Règle |
|---|---|
| Trésorerie initiale | 100 000 EUR ; gymnase préplacé non facturé, construction au jalon 8 |
| Gymnase | 100 EUR à chaque minuit pour la journée écoulée |
| Fonctionnement | 10 EUR par heure d'activité réellement assurée, prélevés en fin de séance |
| Camille | 600 EUR à chaque fin de semaine si recrutée |
| Université | Forfait de 1 200 EUR en fin de semaine ; non facturé par séance, pas encore de pénalité financière pour besoins incomplets |
| Cours public | Tarif entier de 0 à 30 EUR, par personne et par séance, réglé après la séance |
| Demande publique | Base de 8 personnes avant 16 h, 12 entre 16 et 18 h, 20 à partir de 18 h ; ajustement selon le prix et plafond 20 |
| Accès libre | Pas de recette supplémentaire ; 12 participations avant 16 h et 24 après, ou effectif du groupe explicitement affecté ; plafond 30 |
| Contrat club | 20 personnes, vendredi 20–22, préparation 19–20, forfait de 300 EUR payé après prestation |

Pour les cours publics : demande = partie entière de base × (1,6 − 0,05 × prix), puis plafonnement à 20 personnes. À 18 h, 12 EUR donne 20 personnes (240 EUR) ; 30 EUR donne 2 personnes (60 EUR). Le tarif ne dépend pas de la durée : celle-ci augmente en revanche le coût d'exploitation. Le groupe universitaire compte 24 personnes et l'équipe 16, dans leurs capacités respectives de 30 et 20.

Le journal conserve les 128 dernières écritures datées. Les totaux de semaine et la trésorerie ne sont pas limités à ces 128 lignes. Au lundi 00:00, le bilan précédent est conservé, puis les compteurs de la nouvelle semaine repartent à zéro. Les compteurs du planning demeurent ceux de la semaine type prévue.

## Contrat

Accepter l'offre dans Personnel et finances, puis créer une location **vendredi 20:00, 2 h**, avec **Sans encadrant** et l'option **Club vendredi 20-22** dans le champ Location. L'acceptation seule ne programme rien. Supprimer cette réservation conserve le contrat et affiche **À reprogrammer**. S'il n'est pas assuré à l'échéance, il est marqué manqué et ne paie rien. Aucun frais de pénalité n'est ajouté.

Le contrat accepté est ponctuel pour la semaine affichée ; le modèle de réservation reste récurrent. Il faut accepter l'offre suivante pour être payé une autre semaine. Après vendredi 20 h, l'offre vise la semaine suivante. On ne peut pas accepter deux fois la même offre ni remplacer un contrat en attente.

Une trésorerie négative déclenche l'alerte de déficit et bloque le recrutement. Le moteur continue de payer les charges et de recevoir les recettes. Écran de faillite, emprunts, licenciement, autres contrats et revenus avancés restent reportés.

## Tests rapides dans Unreal

Arrêter Play, **Ctrl + Alt + F11**, attendre le chargement réussi du patch, puis **Play → P**. Rester en pause pour programmer. **Espace** bascule la pause ; **3** ou pavé numérique 3 active x4. P et les vitesses restent disponibles dans la vue de gestion.

1. **Séance sans encadrant.** Sans recruter, créer Université lundi 08–10 avec Sans encadrant. L'alerte apparaît mais l'enregistrement fonctionne. Lancer x4 et faire pause après 10 h. Dans Personnel et finances : séance manquée, 0 heure universitaire assurée, trésorerie toujours 100 000 EUR avant minuit.
2. **Recrutement.** Recruter Camille. Le bouton devient inactif et le solde ne change pas immédiatement. Dans le planning, créer lundi 18–20 Cours public, Camille, 12 EUR. Préparation 17–18 visible.
3. **Disponibilité.** Essayer un cours public mardi 19–21 avec Camille puis samedi 14–16 avec Camille. Les deux sont refusés. La séance du lundi reste intacte.
4. **Paiement.** Faire tourner jusqu'après lundi 20 h puis pause avant minuit. Journal : 20 participants, +240 EUR et −20 EUR. Trésorerie : **100 220 EUR**. Fermer/rouvrir P ne doit rien repayer.
5. **Charges.** Passer mardi 00 h : −100 EUR, trésorerie **100 120 EUR**.
6. **Prix et demande.** Programmer mardi 18–20 avec Camille à 30 EUR. Après 20 h : 2 participants, +60 EUR, −20 EUR. Trésorerie **100 160 EUR** avant le minuit suivant. Les conditions du cours du lundi restent inchangées dans le journal.
7. **Contrat à reprogrammer.** Accepter l'offre de la semaine 1, programmer la location vendredi 20–22 avec l'option du contrat. La supprimer : le contrat reste accepté, avec statut À reprogrammer. Recréer la même location et lui réassocier le contrat.
8. **Prestation club.** Après vendredi 22 h : contrat honoré, +300 EUR et −20 EUR, 20 participants. Aucun encadrant salarié nécessaire.
9. **Contrat manqué (autre session).** Accepter l'offre sans créer la location. Passer vendredi 22 h : contrat manqué, aucune recette de location.
10. **Pause et conservation.** Mettre en pause : solde et journal restent stables. Fermer/rouvrir P conserve personnel, comptes, affectations et contrat. Arrêter Play réinitialise encore la partie.

## Semaine complète de contrôle

Sur une **nouvelle session**, avant de lancer le temps, recruter Camille et accepter le contrat. Programmer :

| Jour | Réservations |
|---|---|
| Lundi | Université 08–10 (Camille) ; entraînement 14–16 (Camille) ; public 18–20 (Camille, 12 EUR) |
| Mardi | Accès libre 16–18, sans encadrant |
| Mercredi | Université 08–10 (Camille) ; public 18–20 (Camille, 12 EUR) |
| Jeudi | Entraînement 14–16 (Camille) ; accès libre 16–18, sans encadrant |
| Vendredi | Université 08–10 (Camille) ; location 20–22, sans encadrant, contrat associé |

Laisser les groupes proposés pour université/entraînement ; les autres activités peuvent rester sans groupe étudiant. Le planning doit compter **10 réservations, 20 h d'activité et 3 h de préparation**. Camille a **14 h affectées** et aucune séance ne manque d'encadrant.

Lancer x4 jusqu'au **lundi de la semaine 2, peu après 00:00 et avant 08:00**, puis pause (environ 8 min 20 réelles depuis lundi 08:00). Dans Personnel et finances, le bilan précédent doit afficher :

- Recettes : **1 980 EUR** (480 public + 300 club + 1 200 université).
- Dépenses : **1 500 EUR** (700 gymnase + 600 salaire + 200 fonctionnement).
- Résultat : **+480 EUR** ; trésorerie **100 480 EUR**.
- **10 séances assurées, aucune manquée**, université **6 h**, équipe **4 h**.

La construction n'est pas encore débitée ; le solde de 39 480 EUR du document initial incluait le gymnase et des chemins, qui arriveront avec leur placement.

## Vérification technique

`Tests/CampusOperationsTests.cpp` vérifie affectations, disponibilité, demande, capacité, prix, paiements à la fin, appels répétés, séances figées au début, offre ponctuelle, bilan +480, semaine suivante, intégration avec l'horloge x4, pas de temps groupés, déficit et journal circulaire. `CampusWeeklyScheduleTests.cpp` vérifie les règles de planning et les régressions des jalons 3 à 5.

Les calculs sont vérifiés hors moteur ; compilation et validation visuelle dans Unreal sont suivies dans `Avancement.md`. Prochain jalon : **8 — Construction sur grille**, avec placement du gymnase et coût d'achat.
