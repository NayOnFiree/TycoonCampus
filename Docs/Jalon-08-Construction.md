# Jalon 8 — Premier gymnase sur grille

Le campus commence sans gymnase construit pendant Play, avec 100 000 EUR. Le gymnase préplacé des étapes précédentes est masqué uniquement dans la session de jeu : la carte enregistrée n'est pas supprimée ou modifiée. Il est donc normal de le voir encore dans l'éditeur en dehors de Play.

## Commandes

- **B** : ouvrir le placement ou l'annuler.
- **R** : tourner le bâtiment de 90 degrés (quatre orientations).
- **Clic gauche** sur le terrain : construire si l'emplacement est valide.
- **ZQSD, molette, clic droit glissé** : caméra toujours disponible pendant le placement.
- **P** : ouvrir le planning après construction ; pendant le placement, quitte ce mode.

La grille couvre la parcelle plate de 200 × 200 m, avec des cases de 2 m. L'empreinte de 21 × 14 cases contient le bâtiment de 40 × 26 m et ses murs ; après un quart de tour, elle mesure 14 × 21 cases. L'aperçu est un volume filaire avec contour au sol : vert si constructible, rouge sinon. Une flèche indique le côté de l'entrée. Le texte du HUD donne la raison d'un refus.

La totalité de l'empreinte doit rester sur la parcelle. Les chemins, l'entrée et les obstacles statiques visibles de la carte empêchent la pose ; toucher leur bord sans chevauchement reste possible. La validation est recalculée au clic, et non reprise du dernier aperçu.

Après validation, le gymnase apparaît avec sol, murs, terrain de basket et entrée, dans la position et l'orientation choisies. Il est sélectionné. Son intérieur reste visible (pas de toit à ce stade). La sélection au clic et le planning des jalons précédents restent disponibles.

## Budget et périmètre

Le gymnase coûte **60 000 EUR**. L'achat laisse **40 000 EUR**, ajoute une écriture de construction et un investissement de 60 000 EUR. Cet investissement ne gonfle pas les dépenses d'exploitation hebdomadaires. Une annulation, un placement invalide ou un deuxième clic ne retire pas d'argent.

Ce prototype comporte **un seul gymnase**. Après achat, B indique qu'il est déjà construit. Il n'y a pas encore de duplication de bâtiments, déplacement après achat, démolition ou remboursement.

Avant construction, aucun fonctionnement du gymnase n'est simulé et P ne donne pas accès à sa gestion. Après construction, les charges démarrent au prochain minuit, sans rattraper les journées précédentes. Une séance dont le début est déjà passé au moment de l'achat ne démarre pas rétroactivement. Le bâtiment est utilisable immédiatement ; la nécessité de le raccorder à un chemin arrive au jalon 9.

## Tests dans Unreal

Arrêter Play, **Ctrl + Alt + F11**, attendre le chargement réussi du patch, puis relancer Play sur **L_Campus_Work**.

| Test | Manipulation | Résultat attendu |
|---|---|---|
| Terrain initial | Lancer Play | Ancien gymnase absent ; terrain et accès conservés ; trésorerie 100 000 EUR |
| Aperçu | Appuyer sur B, déplacer le curseur dans une zone libre | Grille de 2 m, volume vert et texte « Emplacement valide » ; mouvement par cases |
| Rotation | Appuyer quatre fois sur R | Quatre orientations de l'entrée ; alternance 21×14 / 14×21 ; retour à l'orientation initiale |
| Chemin | Placer l'aperçu sur le chemin ou la place d'entrée, cliquer | Rouge, raison indiquée, aucun bâtiment ni paiement |
| Bord de parcelle | Amener l'aperçu près d'un bord pour qu'il dépasse, cliquer | Rouge et refus ; le centre seul ne suffit pas à rendre l'emplacement valide |
| Annulation | Revenir en zone libre puis appuyer sur B | Aperçu et grille disparaissent ; trésorerie inchangée |
| Achat | B, zone libre à droite du chemin, R si souhaité, clic gauche | Bâtiment visible à cet endroit, bonne orientation, repère de sélection ; solde 40 000 EUR |
| Double achat | Cliquer de nouveau puis B et essayer une autre position | Aucun deuxième bâtiment ni nouveau débit ; message gymnase déjà construit |
| Sélection | B pour quitter si nécessaire, clic dehors puis sur le bâtiment | Repère désactivé puis réactivé au clic simple |
| Gestion | P → Personnel et finances | Investissement 60 000 EUR, écriture Construction, résultat d'exploitation initial 0 EUR |
| Planning | Recruter Camille, créer une séance future et lui affecter Camille | Fonctionnement des jalons précédents conservé dans le gymnase construit |
| Caméra | Fermer P et essayer ZQSD, zoom et rotation ; également avant achat dans une nouvelle session | Commandes conservées |

Pour vérifier l'absence de charges rétroactives : nouvelle session, laisser passer une journée sans construire, constater 100 000 EUR, puis acheter. Le solde devient 40 000 EUR ; les premières charges de 100 EUR arrivent au minuit suivant.

Le contrôle de semaine des jalons 6–7 fonctionne après construction lundi 08:00 avant de lancer le temps : le résultat d'exploitation reste **+480 EUR**, mais la trésorerie devient **40 480 EUR**, puisque le gymnase est maintenant payé. Les chemins achetables ne sont pas encore inclus.

Arrêter Play réinitialise la construction et les données de gestion. La sauvegarde demeure prévue au jalon 11.

## Vérification technique

Tests natifs et compilation du jeu réussis (`Saved/Logs/BuildJalon8.log`). Le chargement du patch Live Coding, le rendu et les interactions restent à valider dans Unreal.

Tests natifs : conversion curseur/grille, coordonnées invalides, rotation, limites exactes, chevauchements, solde exact, fonds insuffisants, paiement unique, investissement distinct, construction tardive et absence de séance rétroactive. Les tests d'exploitation des jalons 6–7 sont également rejoués.

Les obstacles runtime sont vérifiés avec les volumes englobants des acteurs de maillage statique visibles. Le placement vise la carte plate du prototype ; terraformation, pentes et obstacles dynamiques ne sont pas encore pris en charge. Le repérage de l'ancien gymnase utilise les positions, tailles et orientations exactes des pièces du script d'origine, et ne modifie que leur visibilité/collision dans Play.

Prochain jalon : **9 — Chemins et raccordement**.
