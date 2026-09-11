# Jalon 1.1 — Carte de travail

10 septembre 2026. Première étape de développement.

## Réalisé

- Projet existant vérifié : Unreal Engine 5.8.2, module C++ de base.
- Carte `Content/TycoonCampus/Maps/L_Campus_Work.umap` créée.
- Parcelle plane de 200 × 200 m, entrée et allée de 4 m de largeur.
- Gymnase provisoire de 40 × 26 m, murs de 8 m, ouverture côté entrée et terrain de basket schématique de 28 × 15 m.
- Cinq matériaux simples, éclairage directionnel, ciel et point de départ en hauteur.
- Objets rangés dans les dossiers Terrain, Access, Gym, Lighting et View de l'Outliner.
- Carte définie comme carte de démarrage du jeu et de l'éditeur.

Le bâtiment n'a volontairement pas de toit pour observer l'intérieur. Ce n'est pas encore une fonction de toit masquable. L'allée n'est pas encore un réseau de déplacement simulé. Les dimensions sont des repères de travail, pas une certification d'installation sportive.

## Ouvrir dans Unreal

Dans le Content Browser, ouvrir `Content > TycoonCampus > Maps > L_Campus_Work`. Si l'éditeur est déjà ouvert, il faut ouvrir cette carte manuellement ; le changement de carte de démarrage s'applique à la prochaine ouverture du projet. Sélectionner `Gym_Floor_40x26m` dans l'Outliner et appuyer sur F pour cadrer le bâtiment.

La caméra tycoon n'existe pas encore. Utiliser pour le moment les commandes habituelles du viewport de l'éditeur.

## Vérification effectuée

Création par le moteur, sauvegarde puis rechargement et contrôle de la présence des objets essentiels. Journal : `Saved/Logs/CreateWorkMap.log`. Rapport : `Saved/CampusWorkMapReport.json`. Le raccord géométrique de l'allée à l'entrée du bâtiment a aussi été corrigé et rechargé. Vérification sans rendu (`nullrhi`) : ni qualité visuelle, ni FPS, ni comportement en Play ne sont validés à cette étape.

Le premier lancement de génération a rencontré un problème de chemin du cache Zen. La génération a abouti avec un cache fichier dans `DerivedDataCache`, via des arguments de lancement ponctuels. Aucun changement permanent de configuration du cache n'a été ajouté au projet.

## Script de création

`Scripts/Editor/create_work_map.py` documente les dimensions et construit les assets dans Unreal. Il refuse de remplacer une carte existante pour protéger les modifications futures. Aucun plugin Python n'est nécessaire pour ouvrir la carte : Python a été activé seulement pendant la génération.

## Prochaine mécanique

Jalon 1.2 : caméra aérienne, déplacement, rotation et zoom. Commencer par le déplacement seul, puis vérifier avant d'ajouter rotation et zoom.
