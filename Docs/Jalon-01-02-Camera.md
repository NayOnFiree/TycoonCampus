# Jalon 1.2a — Déplacement de la caméra aérienne

## Ouvrir la carte dans l'éditeur français

1. Cliquer sur **Tiroir à contenu**, en bas à gauche, ou utiliser **Ctrl + Espace**.
2. Dans le contenu du projet, ouvrir **TycoonCampus > Maps > L_Campus_Work** par double-clic.
3. L'onglet de niveau doit afficher **L_Campus_Work**, et non **Sans titre**.

## Fonctionnement implémenté

- Caméra aérienne initialement orientée vers le gymnase.
- **ZQSD** par défaut ou **flèches** pour se déplacer sur le plan du campus. W et A ne sont plus affectées.
- Déplacement relatif à l'orientation de la caméra, sans modification de hauteur.
- Vitesse indépendante du nombre d'images par seconde, sans accélération en diagonale.
- Point visé limité à ±90 m du centre sur les deux axes.
- Curseur visible ; maintenir le **bouton droit** et déplacer la souris pour tourner autour du point visé. Relâcher restitue le curseur à sa position initiale.
- Rotation horizontale libre ; inclinaison limitée entre 25° et 80° vers le sol.
- **Molette vers le haut** : rapprocher ; **vers le bas** : éloigner, avec transition douce.

Les classes `CampusCameraPawn` et `CampusGameMode` se trouvent dans `Source/TycoonCampus`. Le mode de jeu est déclaré dans `Config/DefaultEngine.ini` ; les touches sont dans `Config/DefaultInput.ini`. Vitesse et limite sont des propriétés exposées à Unreal. Un Blueprint dérivé pourra conserver leurs réglages lorsque nous préparerons la personnalisation de la caméra.

## Compilation et essai

Les nouvelles classes et les réglages de configuration doivent être chargés avant l'essai. Live Coding (**Ctrl + Alt + F11**) permet beaucoup de modifications de fonctions C++ sans fermeture, mais ne remplace pas systématiquement un redémarrage pour les nouvelles classes, les constructeurs et la configuration.

Une fois la compilation réussie et les réglages chargés : ouvrir la carte, cliquer **Jouer**, puis cliquer dans la vue pour lui donner le focus. Tester les touches et les limites ; **Échap** arrête le jeu. Les commandes habituelles du viewport hors jeu sont indépendantes de la caméra créée ici.

Si la caméra par défaut d'Unreal apparaît, vérifier dans **Paramètres du monde** que le mode de jeu utilise **CampusGameMode** (ou hérite du réglage du projet). Une session d'éditeur déjà ouverte peut conserver l'ancien réglage jusqu'au redémarrage.

## Vérifications manuelles attendues

- Le lancement présente le gymnase depuis le haut.
- Haut/bas et gauche/droite suivent l'écran sans changer l'altitude.
- Deux directions simultanées ne rendent pas le déplacement plus rapide.
- Aux limites de la parcelle, le déplacement s'arrête sans trembler.
- Relâcher une touche ou quitter le focus arrête le déplacement.

La présence de ce document ne signifie pas que ces essais visuels ont été exécutés. Le compte rendu de conversation précise les validations réellement effectuées.

## Ajout de la rotation — jalon 1.2b

Les déplacements restent relatifs à l'orientation courante. La souris fonctionne en déplacement relatif pendant la rotation pour ne pas buter au bord de l'écran. Les déplacements de souris ne sont pas multipliés par le temps d'image. La perte de focus doit libérer la rotation : à vérifier aussi avec Alt+Tab pendant un glissement.

Les affectations de touches sont relues au début d'une nouvelle session Play pour appliquer ZQSD après Live Coding. Arrêter Play avant de compiler, puis relancer Play pour installer les nouveaux événements de souris.

Essais : faire un tour complet, atteindre les deux limites d'inclinaison, vérifier ZQSD après rotation, relâcher le bouton droit et vérifier le curseur. W/A doivent rester sans effet.

Validation du 10 septembre 2026 : compilation Live Coding réussie, patch_1 chargé par l'éditeur à 21:55 (journal `Saved/Logs/TycoonCampus.log`). Le build jeu séparé a été arrêté une fois cette validation obtenue. Les essais de manipulation en Play restent à effectuer ; le déplacement précédent a été confirmé fonctionnel par le joueur.

## Zoom — jalon 1.2c

La molette modifie la distance au point visé, sans changer ce point ni le champ de vision. Chaque cran vers le haut réduit la distance cible de 15 % ; vers le bas, il applique le facteur inverse. Les crans rapides se cumulent sur la destination. La distance reste entre 25 et 240 m, avec un démarrage à 90 m. Il s'agit de la longueur du bras de caméra, pas de son altitude verticale.

Le lissage exponentiel ne dépend pas du nombre d'images par seconde ; aucune multiplication par le temps d'image n'est appliquée aux crans de molette. Le zoom vers le curseur et la collision avec les futurs bâtiments hauts ne sont pas inclus. La distance minimale conserve la caméra au-dessus des murs de 8 m du décor actuel, même à l'inclinaison minimale.

Essais en Play après compilation : zoomer dans les deux sens ; insister aux limites ; inverser rapidement la molette ; combiner zoom, rotation et ZQSD. Le point visé et l'inclinaison doivent rester stables pendant le zoom seul. Arrêter puis relancer Play pour charger l'événement de molette.

Validation : cible TycoonCampus Win64 Development compilée et liée avec succès (journal `Saved/Logs/BuildZoom.log`, 223 secondes pour cette première compilation complète). Cela vérifie le code, pas encore la manipulation en jeu. Dans l'éditeur déjà ouvert, charger cette modification avec Ctrl + Alt + F11, puis relancer Play.

## Correctif après essai utilisateur : molette sans effet

Le joueur a signalé l'absence de zoom. L'axe `MouseWheelAxis` est remplacé par les événements `MouseScrollUp` et `MouseScrollDown`, uniquement à la pression, sans double affectation de l'axe. Le calcul et les limites du zoom sont conservés.

Le journal de sortie doit afficher `CampusCamera: wheel zoom v2 ready` au démarrage de Play, puis `CampusCamera: wheel 1` ou `wheel -1` à chaque cran, avec la distance cible. Ces traces sont provisoires et permettent de vérifier la version chargée et l'acheminement des événements. Une cible inchangée à 2500 ou 24000 cm correspond à une limite normale.

La compilation du jeu n'actualise pas à elle seule le module de l'éditeur déjà ouvert. Après modification : arrêter Play, compiler avec Live Coding, attendre le succès, puis relancer Play et placer le pointeur dans la vue de jeu. Le fonctionnement visuel reste à confirmer ; la date ancienne du DLL principal ne suffit pas à déterminer quels patchs Live Coding sont chargés.

## Correctif rotation après validation du zoom

Le joueur confirme le zoom fonctionnel mais signale la rotation inactive. Le mode GameAndUI reste maintenant stable : le viewport capture la souris uniquement pendant le clic droit, masque temporairement le curseur et le restitue au relâchement. Les callbacks du bouton changent seulement l'état de rotation, sans modifier le mode de saisie au milieu de l'événement. MouseX/MouseY passent par deux axes nommés, comme le déplacement clavier. Le zoom n'est pas modifié.

La trace `CampusCamera: right-drag v3 ready` confirme la nouvelle version au démarrage de Play ; `rotation started` et `rotation stopped` permettent de contrôler le bouton. Tester clic droit + déplacement horizontal puis vertical, relâchement, répétition, Alt+Tab et zoom entre deux rotations. Compilation seule et validation en jeu restent distinctes.

## Diagnostic rotation v4

Le journal de l'essai utilisateur confirme que la v3 est chargée et que les événements de clic droit arrivent correctement. Le problème ne peut donc plus être attribué à un correctif non chargé ou à un clic absent. Les axes nommés ajoutés pendant la session sont supprimés : la caméra lit maintenant `GetInputMouseDelta` sur le contrôleur après traitement de la saisie. Les rotations restent conditionnées par le clic droit. La sensibilité précédente multipliait les valeurs MouseX/Y déjà atténuées (0,07 dans le projet) par 0,2 ; le multiplicateur devient 2,5.

La trace `right-drag v4 ready` identifie cette version. Les traces `orbit delta ... -> yaw ... pitch ...` mesurent les mouvements reçus et les angles appliqués. Le contrôle visuel automatique a échoué sur la capture Windows ; l'essai utilisateur et ces traces sont nécessaires avant de valider le correctif.
