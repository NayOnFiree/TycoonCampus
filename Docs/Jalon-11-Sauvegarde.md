# Jalon 11 — Sauvegarde et chargement

Ce jalon ajoute une sauvegarde manuelle locale pour le prototype sur **L_Campus_Work**. Après sa validation, faire un point sur le jeu avec le joueur avant de développer la suite.

## Commandes

- **F5** : sauvegarder le campus actuel.
- **F9** : remplacer la partie actuelle par la sauvegarde, puis rester **en pause**.
- **F10** : charger la sauvegarde précédente, également en pause. Cette copie existe après deux sauvegardes réussies.
- **Espace** : reprendre après un chargement. La vitesse choisie est conservée.

Ces commandes fonctionnent aussi dans le planning. Des boutons et le résultat de l'opération sont disponibles dans **P → Personnel et finances**. Hors planning, le résultat apparaît sous la ligne d'activité. Un chargement réussi ferme le planning et annule les aperçus de construction en attente. Un échec laisse la partie actuelle intacte.

**F9 et F10 remplacent les changements non sauvegardés.** F5 conserve les réservations déjà validées, pas les modifications encore dans le formulaire. La sauvegarde est manuelle : démarrer Play crée toujours une session neuve, puis F9 permet de retrouver son campus. Arrêter Play ou fermer Unreal ne déclenche pas de sauvegarde automatique.

## Données conservées

Position et orientation du gymnase, chemins achetés, sélection, caméra et zoom ; date et heure exactes, vitesse ; réservations et identifiants ; recrutement et affectations ; contrat ; trésorerie, investissements, journal et bilans ; séance en cours, participants, résultat prévu, retard, arrivées de groupes et paiements déjà exécutés ; état des figurants et trajet mémorisé.

Le chargement reconstruit les cases et les parcours à partir de la carte. Il restaure les états sans repasser par les achats ni rejouer les heures déjà traitées. Le temps passé hors du jeu ne produit ni revenus ni charges.

Fichiers : `Saved/SaveGames/CampusQuick.sav` et `CampusQuick.sav.bak`. Format binaire versionné avec contrôle d'intégrité et taille limitée. Les valeurs et les obstacles de la carte sont contrôlés avant de modifier la partie. L'écriture passe par un fichier temporaire vérifié ; seule une sauvegarde principale valide peut remplacer la copie de secours. Ce format concerne le prototype actuel ; aucune migration vers de futurs formats n'est encore prévue.

## Tests en jeu

Arrêter **Play**, faire **Ctrl + Alt + F11**, attendre la réussite du **chargement du patch Live Coding**, puis lancer Play sur **L_Campus_Work**. Le premier « Result: Succeeded » ne suffit pas si le lien du patch échoue ensuite.

1. **Retrouver son campus.** Construire un gymnase tourné, le raccorder, recruter Camille et créer quelques réservations. Changer l'angle et le zoom de la caméra. Mettre en pause, noter l'heure et la trésorerie, puis F5. Modifier une réservation et acheter quelques cases. F9 doit rétablir l'heure, l'argent, les chemins, le planning et la caméra sauvegardés, en pause.
2. **Entre deux sessions.** Après F5, arrêter puis relancer Play. F9 doit retrouver le campus, le recrutement et les réservations. On peut aussi fermer puis rouvrir Unreal et répéter F9.
3. **Reprendre une séance.** Préparer un cours public lundi 18–20 h avec Camille, prix 12 EUR, et un raccordement terminé bien avant 18 h. Vers 19 h, mettre en pause et faire F5. Reprendre jusqu'après 20 h, puis noter l'argent et les lignes du journal. F9 doit revenir vers 19 h, avant le paiement. Reprendre jusqu'après 20 h : obtenir le même solde et les mêmes lignes, avec une seule recette de 240 EUR et 20 EUR de frais pour cette séance complète. Refaire F9 et cette reprise : le résultat doit rester identique. Les figurants doivent reprendre leur activité.
4. **Copie précédente.** Sauvegarder un état A avec F5 ; modifier le planning, puis sauvegarder l'état B avec F5. F10 doit retrouver A ; F9 doit retrouver B. Charger ne réécrit aucun fichier.
5. **Interface et caméra.** Tester les boutons dans Personnel et finances. Après chargement, vérifier ZQSD, rotation au clic droit, zoom, sélection, B et C. Charger une sauvegarde prise avant construction doit retirer le gymnase et permettre de le reconstruire normalement.

Test facultatif de fichier manquant : hors Play, renommer temporairement `CampusQuick.sav` en `CampusQuick.sav.test`, lancer Play puis F9. Un message doit signaler le problème sans changer la session. Remettre ensuite le nom d'origine, sans effectuer F5 entre-temps.

## Vérification technique

`Tests/CampusSaveTests.cpp` vérifie l'aller-retour exact, la séance active même après suppression de sa réservation, les paiements, les transitions calendaires, le journal circulaire et le refus des données invalides, tronquées, corrompues ou de version différente. Ces tests du modèle ne valident pas l'affichage ni le chargement Live Coding : les essais ci-dessus restent nécessaires.

Validation du 11 septembre 2026 : compilation Unreal **Succeeded**, puis confirmation que la cible est � jour (`Saved/Logs/BuildJalon11.log`). Les six suites natives sauvegarde, planning, horloge, �conomie, construction et chemins ont r�ussi. Live Coding et essais visuels restent � valider par le joueur.
