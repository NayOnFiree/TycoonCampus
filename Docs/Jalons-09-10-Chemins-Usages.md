# Jalons 9 et 10 — Chemins, déplacements et usages visibles

## Jalon 9 : chemins sur grille

**C** ouvre ou quitte le mode chemins. Maintenir le clic gauche, déplacer la souris, puis relâcher remplit un rectangle de cases. Un clic bref pose une case. **X** alterne construction et suppression ; **C**, **B**, **P**, le menu et le clic droit annulent la sélection en attente. Traverser une fenêtre d'interface annule également le glisser. R reste réservé à la rotation des bâtiments.

Une case visible mesure **4 × 4 m**, soit 80 EUR si elle est entièrement neuve. Le coût au mètre carré reste identique ; les parties déjà présentes ne sont pas refacturées. **Maj** maintenue permet la précision de 2 m pour les raccords étroits d'anciens campus. Les coordonnées internes et le format de sauvegarde restent inchangés. Un rectangle invalide ou trop cher est refusé entièrement, sans débit ni pose partielle.

La place d'entrée et le chemin initial restent protégés. La suppression utilise aussi maintenir-glisser-relâcher, sans remboursement.

Le gymnase, les obstacles statiques de la carte et les limites de la parcelle empêchent le tracé. La construction d'un gymnase ne peut pas recouvrir un chemin, y compris construit par le joueur. Son entrée doit laisser une case raccordable dans la parcelle ; tourner un gymnase placé contre un bord si nécessaire.

## Raccordement et trajet

Une **case cyan** indique l'entrée à raccorder. Le réseau doit aller de la place d'entrée du campus à cette case. Les voisins sont les quatre côtés d'une case : toucher seulement par un coin ne raccorde rien. Le calcul utilise le plus court chemin praticable ; un détour reste utilisable et une route alternative maintient l'accès après une coupure.

Le statut affiché sur le campus et dans Personnel et finances indique si le gymnase est raccordé, la distance totale jusqu'au terrain intérieur et le temps de marche à **1,2 m/s**, arrondi à la minute supérieure. La distance comprend les chemins publics déjà présents : leur longueur n'est pas refacturée.

Une séance commençant sans raccordement est manquée, sans recette ni coût variable. Les charges fixes et le salaire restent dus. Le planning peut être préparé avant raccordement avec une alerte. Reconnecter le gymnase ne relance pas une occurrence déjà commencée ; les suivantes pourront fonctionner. Inversement, supprimer le chemin ne réécrit pas une séance déjà commencée : celle-ci conserve les conditions de son départ. Il n'y a pas d'évacuation en urgence dans ce prototype.

Le premier trajet d'un groupe peut être anticipé avant le début prévu, mais pas avant que le raccordement et la réservation existent. Une programmation ou un raccordement tardif peut donc provoquer un retard. Le temps de pratique réellement assuré est la durée restante jusqu'à l'heure de fin initiale. Une arrivée après la fin fait manquer la séance. Les groupes Étudiants A et Équipe basket restent logiquement dans le gymnase après leur première séance de la journée ; ils ne refont pas le trajet depuis l'entrée à chaque créneau. Leur localisation est réinitialisée pour la journée suivante.

Le journal et le bilan montrent les **minutes assurées** et les retards. Le coût variable de 10 EUR/h est proratisé sur ces minutes, arrondi à l'euro le plus proche. Le cours public reste facturé par personne et par séance. Une location commencée en retard ne remplit pas le contrat : pas de forfait de 300 EUR, mais coût variable pour le temps effectivement utilisé. Les heures planifiées de la semaine type restent distinctes de ces résultats réels.

## Jalon 10 : figurants et activités

Le gymnase affiche jusqu'à **huit figurants simplifiés**, représentant le groupe actuellement montré. Le compteur distingue les participants réellement simulés des figurants visibles. Ce n'est pas une simulation individuelle de toute la population.

Les figurants suivent le parcours jusqu'au gymnase, entrent par sa porte puis occupent le terrain. Université : groupe rassemblé ; entraînement : déplacements autour du terrain ; cours public : exercices synchronisés ; accès libre et club : mouvements libres. Les couleurs des corps différencient également les usages. Les visiteurs sans groupe étudiant repartent après leur séance. Les représentants des groupes étudiants peuvent rester au gymnase entre leurs séances de la journée.

Ces personnages sont des placeholders faits de formes simples, avec animation procédurale des membres. Ils n'ont ni IA individuelle, ni match tactique, ni animations sportives finales. Un seul groupe représentatif est affiché à la fois ; les comptes ne dépendent jamais de son nombre de figurants ni de ses animations. Les trajets suivent le temps simulé ; les exercices ont une animation de présentation. La pause fige les deux. À x4, un court trajet peut passer très vite à l'écran.

## Préparation des tests

Arrêter Play, faire **Ctrl + Alt + F11**, attendre le chargement réussi du patch, puis relancer Play sur **L_Campus_Work**. Rester en pause pour construire et programmer.

### Tests chemins

| Test | Manipulation | Résultat attendu |
|---|---|---|
| Bâtiment isolé | Construire avec B dans une zone libre à droite du chemin, entrée orientée vers celui-ci | Gymnase NON RACCORDE et case cyan devant sa porte ; choisir un emplacement qui laisse quelques cases libres entre le chemin existant et la porte |
| Aperçu | C, maintenir sur une case libre puis glisser | Grandes cases, rectangle vert et prix ; aucun débit pendant le maintien |
| Carré / rectangle | Glisser dans les deux axes, dans les quatre directions | Surface entièrement remplie au relâchement |
| Raccordement | Tracer jusqu'à la case cyan ; utiliser Maj si nécessaire sur un ancien campus | Raccordement et distance actualisés |
| Doublon | Refaire exactement le même rectangle | Aucun coût supplémentaire |
| Obstacle / budget | Inclure un obstacle ou dépasser le budget | Aperçu rouge ; refus complet, aucun débit |
| Annulation | Maintenir puis C, ou déplacer sur l'interface | Rien de construit ; relâcher ne valide pas ensuite |
| Protection | C, X, sélectionner une case du chemin initial | Suppression refusée |
| Coupure | X, glisser sur une zone achetée indispensable au raccordement | Zone retirée sans remboursement, statut NON RACCORDE |
| Réparation | X puis reconstruire cette zone | Prix affiché débité et raccordement restauré |
| Route alternative | Construire un détour qui rejoint le même réseau, puis couper une seule branche | Gymnase toujours raccordé si l'autre branche reste continue |

### Tests usages et simulation

Sur un campus construit et raccordé, recruter Camille puis programmer des séances **futures** :

| Horaire proposé | Activité | Affectation |
|---|---|---|
| Lundi 10–12 | Université | Étudiants A, Camille |
| Lundi 14–16 | Entraînement | Équipe, Camille |
| Lundi 16–17 | Accès libre | Sans groupe étudiant, sans encadrant |
| Lundi 18–20 | Cours public | Sans groupe étudiant, Camille, 12 EUR ; préparation 17–18 |
| Vendredi 20–22 | Location | Accepter l'offre, associer le contrat, sans encadrant ; préparation 19–20 |

1. Lancer à **x1**, observer l'approche juste avant 10 h puis le groupe dans le gymnase. Zoomer à la molette et mettre en pause pour examiner les figurants. Le compteur doit annoncer **24 participants simulés** et au plus **8 figurants** pendant le cours.
2. Reprendre et observer les usages suivants : mouvements d'entraînement, accès libre, puis cours public. À la fin du cours public : **20 participants**, +240 EUR, −20 EUR si la séance est complète. Le fonctionnement de l'économie reste indépendant des animations.
3. Pendant une activité, mettre en pause : horloge, positions et exercices doivent rester stables. Reprendre : activité et mouvements continuent.
4. Vérifier une séance **sans accès** dans une session séparée, ou couper le raccordement avant une séance future. Après sa fin : journal « gymnase inaccessible », aucune minute assurée pour cette séance et aucune recette.
5. Vérifier un **retard initial** dans une nouvelle session : construire et raccorder à lundi 08:00 en pause, puis programmer Université 08–10 avec Camille et lancer. La première arrivée se fait après le trajet : le journal indique un retard et moins de 120 minutes assurées. L'heure de fin reste 10:00.
6. Programmer une deuxième séance universitaire juste après, 10–12 : le groupe étant déjà au gymnase, cette deuxième séance ne doit pas subir à nouveau le trajet d'entrée.
7. Pour retrouver le bilan complet de +480 EUR, reprendre la semaine de contrôle des jalons 6–7, **en décalant le premier cours du lundi à 10–12** pour laisser le temps d'arriver. Avec raccordement anticipé, pas de retard ni de panne d'accès : trésorerie attendue **40 480 EUR moins les achats de chemins**, avant tout autre investissement.

Les mouvements et les couleurs restent à valider visuellement dans Unreal. Fermer/rouvrir P conserve les données de la session ; arrêter Play réinitialise encore tout.

## Vérifications techniques et limites

Compilation finale du jeu réussie : `Saved/Logs/BuildJalons9-10.log`. Tests natifs réussis. Le chargement du patch Live Coding et la validation visuelle ne sont pas remplacés par ces vérifications.

Les tests natifs vérifient coûts et pose atomique, obstacles, chemins protégés, suppression, routes alternatives, absence de liaison diagonale, plus court chemin, accès manquant, départ anticipé, réservation de dernière minute, retard, arrivée après la fin, conditions figées d'une occurrence, présence des groupes, contrat partiel et invariance au regroupement des pas de temps. Les tests précédents de planning, construction et économie sont rejoués.

La grille est séparée de l'objet Unreal et le parcours est recalculé lors des modifications du réseau, pas par figurant. Les pièces de chemins et les figurants utilisent des maillages instanciés. Aucun objectif de 60 FPS n'est déclaré validé sans mesure sur la configuration cible.

La parcelle reste plate et le prototype garde un seul gymnase. Congestion détaillée, files d'attente, plusieurs bâtiments, foule individuelle et assets définitifs sont reportés. Le prochain jalon est **11 — Sauvegarde et chargement**.
