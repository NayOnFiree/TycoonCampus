# Jalon 1.3 — Sélection du gymnase

## Comportement

- Clic gauche sur le volume du gymnase : sélectionner l'installation entière.
- Quatre angles turquoise fins apparaissent au sol autour de son emprise.
- Clic sur le terrain, une allée ou le ciel : désélectionner et masquer le cadre.
- Clic répété sur le même bâtiment : conserver la sélection.
- Clic gauche pendant une rotation : ignorer, pour éviter les sélections accidentelles.
- Le cadre ne bloque ni les clics ni les déplacements. La caméra et la molette gardent leur fonctionnement validé.

## Organisation provisoire

`CampusBuilding` représente l'installation logique. Son volume de sélection englobe les éléments séparés du gymnase de travail. Le GameMode crée une seule instance au début de la partie, à l'emplacement du gymnase préplacé. Cette liaison provisoire sera remplacée par le placement de bâtiments au jalon construction. Le cadre utilise des meshes ordinaires et un matériau non éclairé, pas un tracé de debug.

La sélection est locale au joueur et conserve une référence faible, pour supporter plus tard la suppression du bâtiment. Les dimensions de ce premier volume sont fixes ; les empreintes configurables et les autres bâtiments viendront avec la construction.

Le matériau `M_Selection` est créé indépendamment de la carte ; aucun décor existant n'a été déplacé ou remplacé. Le script de création ne remplace pas un matériau existant.

## Essai

Arrêter Play, compiler avec Ctrl + Alt + F11, attendre la réussite, relancer Play. Cliquer sur le gymnase puis sur le terrain. Refaire l'essai depuis plusieurs angles et distances, ainsi que pendant une rotation au clic droit. Les traces `CampusSelection: Gymnase 01 selected` et `CampusSelection: cleared` distinguent la détection du clic de l'affichage du cadre.

La fiche détaillée du bâtiment, la grille et le placement ne font pas partie de cette étape. La construction sur grille a été retenue avec le joueur ; ses cases et empreintes seront précisées avant le jalon 8.

## État

Le déplacement, la rotation v4 et le zoom ont été confirmés par le joueur. Le matériau de sélection a été généré et rechargé par Unreal. La compilation et l'essai de sélection sont suivis séparément ; le compte rendu de conversation indique les contrôles réellement terminés.

## Retour utilisateur : double-clic et cadre

Le mode `CaptureDuringRightMouseDown` ne transmettait pas systématiquement la pression gauche sans capture existante (condition `bTemporaryCapture || bProcessInputPrimary` dans `SceneViewport.cpp`). Il est remplacé par `CaptureDuringMouseDown`, qui traite la première pression des deux boutons. Le maintien du clic droit reste la condition de rotation. Le grand rectangle a été remplacé par quatre angles de 4,5 m, épaisseur 18 cm, pour un repère provisoire moins chargé. Les finitions artistiques et l'adaptation visuelle au zoom viendront plus tard.
