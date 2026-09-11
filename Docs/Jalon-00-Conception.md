# Tycoon Campus — Jalon 0 : conception du prototype

Version 0.1 — 10 septembre 2026. Document de conception initial.

Évolution validée pendant le développement : le joueur a demandé un rythme de base plus rapide. La référence actuelle est une journée de 5 minutes réelles en x1 (puis 2 min 30 en x2 et 1 min 15 en x4). Elle remplace les valeurs initiales de 20 minutes ci-dessous. Voir `Jalon-02-01-Horloge.md` pour les règles actuelles.

## 1. Intention et décisions acquises

Construire un campus sportif et optimiser l'utilisation des mêmes installations selon les heures. Priorités : planning, construction, économie, puis équipes. Le joueur arbitre entre obligations universitaires, entraînements, services étudiants et activités commerciales.

Jeu Windows solo hors ligne. Projet associé à Unreal Engine 5.8, confirmé dans le fichier `.uproject`. Développement solo débutant, petit budget, placeholders et assets achetés acceptés ; Blender disponible. Les caractéristiques exactes du processeur et du GPU restent à préciser ; 16 Go de RAM annoncés. La cible de 60 FPS nécessite une machine de référence, elle n'est pas une performance garantie.

Une journée de 24 heures dure environ 20 minutes réelles à vitesse normale. Semaine récurrente de sept jours, semestre de quatre semaines, pause et accélération. Grande carte à parcelles à terme ; une seule petite parcelle dans le prototype. Groupes simulés, personnages représentatifs en nombre limité. Les trajets sont calculés sur les chemins, indépendamment du nombre de figurants affichés.

Architecture visuelle européenne fictive, végétation abondante, semi-réaliste et formes lisibles. Références exprimées : développement de Cities: Skylines, construction de Planet Zoo, lisibilité de Two Point Campus, fenêtres et tableaux de Big Ambitions. Il s'agit de principes souhaités, pas de reproductions de ces jeux. L'image conceptuelle reste à fournir ; aucun asset final n'est spécifié avant sa réception.

## 2. Périmètre du premier prototype — jalons 1 à 11

Inclus : une entrée, une parcelle, chemins simples, un gymnase complet avec une seule zone de basket, caméra aérienne, toit masquable, cinq usages, réservations récurrentes, groupes, personnel essentiel, contrat simple, trésorerie, rapports, déplacements calculés, représentation animée simple, sauvegarde et chargement.

Le gymnase est préplacé pendant la mise au point du planning ; sa construction devient disponible au jalon 8. Les modules seront conçus comme une extension possible, mais leur placement arrive au jalon 14. Les zones divisibles arrivent au jalon 15. Le prototype utilise donc une seule zone entière.

Reportés : football, piscine, modules achetables, zones simultanées, expansion, vraie satisfaction évolutive, usure et pannes, automatisation, diplômes et examens, recrutement sportif, blessures, matchs, sponsors, météo, vacances avec effets, saisons sportives, recherche, mods, multijoueur et création libre de pièces. Le calendrier compte les semestres mais ne simule pas encore le cursus. Le temps de préparation est une indisponibilité planifiée, pas encore une simulation du service d'entretien.

## 3. Boucle du joueur

Lire les besoins → construire et relier le gymnase → affecter lieux, horaires et personnel → traiter les blocages → lancer la journée → observer participation, retards et comptes → ajuster les occurrences futures → consulter le bilan de semaine.

Décision recherchée : faut-il utiliser une soirée pour satisfaire une obligation encore incomplète, attirer le public ou honorer un contrat ? Un espace libre n'est pas automatiquement une occasion rentable.

## 4. Règles des cinq usages

| Usage | Demande | Encadrement | Recette du prototype | Résultat suivi |
|---|---|---|---|---|
| Cours universitaires | Besoin obligatoire : trois séances de 2 h par semaine pour un groupe | Enseignant habilité | Forfait universitaire hebdomadaire, non facturé par séance | Heures réellement assurées / 6 h |
| Entraînement | Objectif : deux séances de 2 h par semaine | Entraîneur habilité | Aucune recette directe | Heures réalisées / 4 h |
| Accès libre étudiant | Fréquentation selon l'horaire | Pas de salarié affecté dans cette simplification initiale | Inclus dans le forfait universitaire | Participations accueillies et refusées |
| Cours public | Demande selon l'heure et le prix | Encadrant habilité | Prix par participant et par séance | Participants et recette réelle |
| Location à un club | Offre précisant jour, durée, effectif et montant | Encadrement fourni par le club | Forfait payé après prestation complète | Contrat honoré ou manqué |

Ces règles d'encadrement sont des conventions de jeu provisoires. Un encadrant du prototype peut posséder plusieurs habilitations. Les cinq usages emploient le même terrain de basket ; pas de conversion en un autre sport.

## 5. Règles du planning

- Ouverture proposée : 08 h–22 h tous les jours, soit 98 heures disponibles par semaine. Début et fin sur une heure entière ; durée minimale 1 h. Pas de séance traversant minuit dans le prototype.
- Intervalles de type [début, fin[ : une séance finissant à 12 h peut être suivie d'une autre à 12 h seulement si aucune préparation n'est nécessaire.
- Une réservation contient activité, groupe, zone, jour, début, durée, personnel éventuel, tarif, équipement requis et éventuel contrat.
- Préparation provisoire : 1 h avant chaque cours public et chaque location ; 0 h avant les autres usages. Cette heure occupe le terrain et doit rester dans les heures d'ouverture. Pas de préparation ajoutée deux fois à une même séance.
- Blocages : terrain ou salarié déjà occupé, même groupe affecté simultanément ailleurs, activité incompatible, équipement requis absent, créneau hors ouverture, chevauchement de préparation. Le message identifie la ressource et la réservation concernées.
- Choix autorisés avec alerte : trajet trop court, demande supérieure à la capacité, objectifs hebdomadaires incomplets, réservation sans encadrant affecté. Une séance exigeant un encadrant ne s'exécute pas tant que celui-ci manque ; aucune recette de séance n'est alors reçue.
- La capacité est un plafond réel. Les personnes en trop sont refusées ; pas de dépassement implicite.
- Les modifications du modèle récurrent portent sur les occurrences non commencées. L'historique reste inchangé ; une activité en cours n'est pas réécrite.
- Une exception datée représente une annulation ponctuelle. Un contrat accepté conserve son obligation même si sa réservation est supprimée ; le jeu affiche alors « contrat à reprogrammer ».
- Un bâtiment inaccessible empêche le déroulement effectif des activités. La réservation peut rester visible avec alerte, afin de permettre la planification avant raccordement.

Les doublons physiques sont impossibles ; les mauvaises décisions de gestion restent possibles et explicables.

## 6. Valeurs de départ proposées — à équilibrer, non validées par le joueur

| Paramètre | Proposition |
|---|---:|
| Capital initial | 100 000 € |
| Gymnase complet | 60 000 € |
| Chemin | 10 € / m |
| Charges fixes du gymnase | 100 € / jour |
| Coût variable d'une activité | 10 € / heure réellement utilisée |
| Encadrant polyvalent | 600 € / semaine, disponibilité lun.–ven. 08 h–20 h |
| Forfait universitaire | 1 200 € / semaine |
| Groupe universitaire | 24 personnes, 6 h / semaine |
| Équipe | 16 personnes, 4 h / semaine |
| Capacité cours universitaire / entraînement / libre / public / club | 30 / 20 / 30 / 20 / 30 |
| Cours public | 12 € / participant / séance de 2 h |
| Contrat exemple | Vendredi 20 h–22 h, 300 €, 20 participants |
| Marche pour le calcul des trajets | 1,2 m/s |
| Vitesses proposées | Pause, ×1, ×2, ×4 |

Demande publique de test par séance de 2 h : 8 personnes avant 16 h, 12 entre 16 h et 18 h, 20 à partir de 18 h, pour un prix de 12 €. Demande ajustée = arrondi inférieur de demande de base × max(0 ; 1 − 0,05 × (prix − 12)). Tarif autorisé : 0 à 30 €. Participants = minimum de la demande et de la capacité. Cette formule est un outil de test, pas l'équilibrage final.

Accès libre : demande de 12 participants en journée, 24 à partir de 16 h, pour une séance de test de 2 h. Les participations ne sont pas des visiteurs uniques.

Forfait universitaire versé une fois en fin de semaine ; les manquements sont d'abord affichés en heures. Le prototype n'invente pas encore de système de réputation. Charges fixes quotidiennes et salaires hebdomadaires sont dus même sans activité. Construction et chemins sont des investissements séparés du résultat d'exploitation. Trésorerie négative : alerte et nouveaux achats bloqués ; écran de faillite définitive reporté.

Le nettoyage préparatoire est inclus dans les charges fixes du prototype. Aucun agent de nettoyage, coût horaire supplémentaire ou usure n'est simulé avant le jalon dédié.

## 7. Semaine de référence et compte de contrôle

| Jour | Activités | Préparation réservée |
|---|---|---|
| Lundi | Université 08–10 ; équipe 14–16 ; public 18–20 | 17–18 |
| Mardi | Libre 16–18 | Aucune |
| Mercredi | Université 08–10 ; public 18–20 | 17–18 |
| Jeudi | Équipe 14–16 ; libre 16–18 | Aucune |
| Vendredi | Université 08–10 ; club 20–22 | 19–20 |
| Samedi | Disponible | Aucune |
| Dimanche | Disponible | Aucune |

Sans retard ni annulation : 20 heures d'activité et 3 heures de préparation. Occupation utile = 20 / 98 = 20,4 %. Réservation totale = 23 / 98 = 23,5 %. Ces deux mesures sont distinctes. Couverture universitaire : 6 / 6 h ; équipe : 4 / 4 h.

Deux cours publics de 20 personnes à 12 € rapportent 480 €. Recettes : 1 200 + 480 + 300 = 1 980 €. Dépenses d'exploitation : 700 de charges fixes + 600 de salaire + 200 de fonctionnement = 1 500 €. Résultat hebdomadaire : +480 €, hors construction. Avec 100 m de chemin, trésorerie après construction : 39 000 € ; après cette semaine : 39 480 €.

Le scénario constitue un point de contrôle favorable, pas une stratégie optimale ni une promesse de rentabilité générale.

## 8. Déplacements et représentation

Chaque groupe conserve sa localisation logique et son heure de disponibilité. Temps de trajet = longueur du plus court chemin praticable / vitesse de marche. Le premier départ peut être anticipé depuis l'entrée ; entre deux réservations, départ après la fin de la précédente. Si l'arrivée dépasse le début prévu, seules les minutes restantes comptent comme pratique ; la fin de séance ne décale pas les suivantes. Une arrivée après la fin produit une séance manquée.

Les personnages affichés représentent ces groupes, sans devenir la source de vérité de leurs résultats. Plafond visuel initial proposé : 30 figurants sur le campus. Les indicateurs « personnes simulées » et « personnages affichés » ne doivent jamais être confondus. Si une animation ou un figurant est retardé pour des raisons visuelles, cela ne change pas le résultat logique du groupe.

## 9. Données minimales et responsabilités

| Élément | Données minimales |
|---|---|
| Installation | Identifiant stable, type, position, orientation, entrée, zones, coût, état de raccordement |
| Zone | Identifiant, installation parente, capacité par usage, équipements, horaires |
| Activité | Type, durée, équipements, habilitation requise, préparation, règle de revenus |
| Groupe | Identifiant, catégorie, effectif, objectifs hebdomadaires, localisation et disponibilité |
| Personnel | Identifiant, habilitations, salaire, disponibilité, affectations |
| Réservation | Identifiant, jour récurrent, début, durée, zone, groupe, personnel, prix, contrat éventuel |
| Occurrence | Date, réservation source, état, arrivées, minutes assurées, participants et montant réel |
| Contrat | Identifiant, fenêtre autorisée, durée, effectif, montant, accepté/honoré/manqué |
| Finance | Écritures datées : montant, catégorie, origine ; trésorerie et bilans calculés |

Séparer calendrier, validation des réservations, calcul de demande, déplacements, exécution des occurrences et écritures financières. L'interface affiche leurs résultats ; les animations les représentent. Ne pas faire recalculer l'économie depuis un écran ou un personnage.

Prévoir une sauvegarde versionnée avec identifiants stables dès les premières données : date courante, placements, chemins, modèles, exceptions, occurrences actives, groupes, personnel, contrats et écritures. La fonctionnalité complète arrive au jalon 11 ; un rechargement ne doit pas verser une recette deux fois.

Répartition technique proposée pour discussion au démarrage : Blueprints pour caméra, assemblage visuel, interactions et présentation ; noyau de règles dans des composants séparés, avec recours limité au C++ si nécessaire. Pas de framework complexe, plugin acheté ni système massif d'agents imposé à ce stade.

## 10. Trois écrans à maquettter

### A. Écran principal

La vue campus occupe l'essentiel de l'espace. Barre supérieure : trésorerie, date et vitesse. Navigation courte : campus, planning, installation. Sélectionner le gymnase ouvre sa fiche. Une alerte prioritaire mène à la réservation concernée. Les jauges de satisfaction ne sont pas affichées avant leur vraie simulation.

### B. Planning hebdomadaire

Jours en colonnes et heures en lignes. Activités nommées, couleurs secondaires par usage, préparation explicitement visible. Au clic, détail de la réservation ; sur un créneau libre, formulaire de création. Dans le jeu futur : champs activité, groupe, durée, personnel, prix et validation avant enregistrement. La maquette du jalon 0 permet seulement de naviguer et inspecter un exemple ; elle n'est pas un éditeur fonctionnel du jeu.

Les besoins non couverts doivent être visibles près du planning. Conflit bloquant : motif et ressource en cause. Alerte : conséquence attendue sans interdiction. Une couleur seule ne porte jamais l'information.

### C. Fiche du gymnase

Nom, raccordement, capacité, activité courante, réservations du jour, occupation utile et bilan de semaine. Actions prioritaires : ouvrir le planning, revenir au campus. Les fonctions reportées ne reçoivent pas de faux boutons actifs. La maquette propose une densité normale ou compacte pour évaluer la lisibilité des tableaux.

La vue du terrain est schématique : elle sert à décider de la disposition de l'interface et ne représente pas la future qualité graphique. L'image conceptuelle sera ajoutée ultérieurement comme référence visuelle, sans bloquer les règles de gestion.

## 11. Critères d'acceptation du prototype

- Partir d'une parcelle vide, construire et raccorder le gymnase.
- Créer puis retrouver la semaine de référence et ses cinq usages.
- Refuser une double réservation de terrain, de groupe ou de personnel avec une explication.
- Empêcher une activité d'occuper l'heure de préparation d'une autre.
- Autoriser un risque de retard avec alerte ; le retrouver dans les minutes réellement assurées.
- Ne pas exécuter un cours sans encadrant ni dépasser une capacité.
- Sans incidents, retrouver les 20 h utiles, 3 h de préparation et +480 € de la semaine de contrôle.
- À vitesse accélérée, conserver le même résultat qu'à vitesse normale.
- Sauvegarder pendant une séance puis recharger sans perdre les obligations ni dupliquer les recettes.
- Pouvoir expliquer pourquoi modifier un créneau améliore ou dégrade le résultat.

## 12. Sortie du jalon 0 et prochaine petite tâche

Livrables préparés : périmètre, règles, hypothèses chiffrées, semaine de référence, modèle de données, critères de validation et trois vues de maquette. Les propositions de prix et de fonctionnement restent modifiables après les premiers essais ; elles ne sont pas des décisions déjà confirmées par le joueur.

À compléter sans bloquer cette conception : image conceptuelle, modèle exact du CPU et GPU. Prochaine tâche de développement, lorsque demandée : jalon 1.1, vérifier le projet existant et préparer une petite carte de travail. Ne pas enchaîner automatiquement sur l'implémentation à partir de ce document.
