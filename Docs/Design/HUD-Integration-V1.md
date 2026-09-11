# HUD visuel V1 — intégration de la maquette

Référence acceptée : Campus-HUD-Maquette-v1.png. Cette livraison modifie les vrais widgets Slate, pas une image plaquée sur le jeu.

## Correspondance avec la maquette

| Élément | Intégration |
|---|---|
| Barre compacte bleu nuit | Trésorerie et bilan groupés, calendrier séparé, pause et vitesses, menu par pictogramme |
| Navigation flottante inférieure | Construire, Chemins, Planning, Personnel, Finances, icônes et raccourcis |
| Fiche claire du gymnase | Titre, raccordement, activité actuelle, occupation planifiée, pratique réalisée, aide contextuelle et action principale |
| Palette et formes | Bleu nuit, blanc cassé, vert menthe, angles arrondis, survols et pictogrammes dessinés par Slate |
| Petites fenêtres | Hauteur de fiche limitée à la fenêtre ; défilement de son contenu |

Les icônes sont vectorielles, sans police de symboles ni téléchargement. Les composants visuels communs sont dans CampusUIStyle.h. La maquette reste une référence ; les états et valeurs sont calculés par le jeu.

Les heures planifiées et réellement effectuées sont explicitement séparées. Les minutes partielles restent visibles, sans arrondi qui laisserait croire qu'un objectif est atteint. Le panneau ne montre plus le nombre technique de figurants. La fiche s'efface pendant le placement pour libérer la carte. Les messages de construction et sauvegarde apparaissent au-dessus du dock, à gauche.

E ouvre Personnel et F ouvre Finances depuis le terrain. Les raccourcis existants ZQSD, P, B, C, R, F5/F9/F10 restent disponibles. Le bouton de pause affiche l'action à effectuer (pause si le temps avance, lecture s'il est arrêté). La vitesse choisie est mise en évidence.

Le terrain, l'éclairage et les modèles 3D sont inchangés. Le planning et les rapports conservent leur structure fonctionnelle du jalon 12 ; cette intégration porte sur la vue campus de la maquette et les contrôles partagés du menu.

## Vérification en jeu

1. Stop Play, Ctrl + Alt + F11, attendre le chargement réussi du patch, puis Play sur L_Campus_Work. Charger avec F9 si souhaité.
2. Vérifier la barre en haut et le dock en bas. Tester pause et ×1/×2/×4 : l'action et la sélection doivent suivre le temps réel du jeu.
3. Construire ou sélectionner le gymnase. Vérifier la fiche claire, la croix de fermeture, Ouvrir le planning et Voir les finances.
4. Créer une réservation puis revenir à la fiche : l'occupation planifiée doit augmenter, les heures réalisées doivent rester inchangées jusqu'à l'exécution.
5. Tester E/F, puis revenir au terrain et vérifier ZQSD, clic droit et molette. Cliquer ou utiliser la molette sur l'interface ne doit pas agir sur le terrain.
6. Essayer une fenêtre de jeu 1280×720 et 1920×1080 : vérifier le défilement de la fiche et l'accès à son bouton principal. Comparer au PNG de référence ; les chiffres peuvent différer selon la sauvegarde.
7. Ouvrir le menu, sauvegarder, charger et reprendre. Les anciens essais de sauvegarde restent applicables.

La compilation est une validation technique ; la conformité visuelle dans l'éditeur reste à confirmer avec une capture en jeu.

Compilation du 11 septembre 2026 : **Succeeded**, journal `Saved/Logs/BuildHUDVisualV1.log`. Le format de sauvegarde et les règles de simulation ne sont pas modifiés. La vérification visuelle de cette version dans l’éditeur reste à faire.
