# Refonte universitaire — inventaire et validation en deux étapes

## Périmètre demandé

Refaire tous les écrans avec une identité marron foncé, crème et bordeaux. S'inspirer de l'organisation lisible des fenêtres de gestion de Big Ambitions. Première livraison : uniquement l'interface en partie. Attendre la validation du joueur avant d'appliquer le thème aux autres écrans.

## Inventaire vérifié dans Source, Content et Config

| Écran ou état | Implémentation actuelle | Première étape | Après validation |
|---|---|---|---|
| Vue campus : barre financière, calendrier, pause, vitesses | CampusHUD.cpp | Nouveau thème universitaire | Ajustements selon essais |
| Navigation : construire, chemins, planning, personnel, finances | CampusHUD.cpp | Nouveau thème, disposition adaptable | Harmonisation globale |
| Fiche gymnase sélectionné : activité, occupation, pratique, alertes | CampusHUD.cpp | Nouveau thème, fiche défilante | Ajustements selon essais |
| Construction et chemins : consignes, validation et erreurs | CampusHUD.cpp + CampusCameraPawn.cpp | Messages du HUD harmonisés ; logique conservée | Vérifier tous les états |
| Notifications de sauvegarde et chargement | CampusHUD.cpp + CampusSaveService.cpp | Présentation en partie harmonisée | Harmoniser aussi les menus |
| Menu Échap : reprendre, sauvegarder, charger, copie précédente, commandes | CampusHUD.cpp | Style existant conservé | Refaire avec le thème validé |
| Confirmation de sortie et annulation | CampusHUD.cpp | Style existant conservé | Harmoniser et vérifier perte non sauvegardée |
| Planning hebdomadaire, cases vides, réservations et préparation | SCampusPlanningPanel.cpp | Style existant conservé | Refaire avec le thème validé |
| Formulaire de réservation, choix, tarifs, groupe, encadrant, contrat | SCampusPlanningPanel.cpp | Style existant conservé | Harmoniser les contrôles, focus et erreurs |
| Suppression confirmée, annulation des changements, alertes de planning | SCampusPlanningPanel.cpp | Style existant conservé | Harmoniser tous les états |
| Personnel, recrutement et charge | MakeManagement | Style existant conservé | Refaire avec le thème validé |
| Offres de contrat et états accepté/programmé/honoré/manqué | MakeManagement | Style existant conservé | Refaire avec le thème validé |
| Finances : comparaison des semaines, investissements, journal | MakeFinance | Style existant conservé | Refaire avec le thème validé |
| Menu principal de démarrage | **Absent** : GameDefaultMap ouvre L_Campus_Work directement | Aucun menu créé avant validation | Créer Continuer / Nouvelle partie / Charger / Quitter avec un parcours complet |

Aucun Widget Blueprint ou autre carte de menu trouvé dans Content : l'interface existante est construite en Slate. Les listes déroulantes sont des contrôles du formulaire et doivent également être couvertes par la seconde étape.

## Direction graphique de la première étape

- Marron foncé #30231F : barre supérieure, navigation, messages sur le terrain.
- Crème #FAF3E7 : texte principal sur les surfaces foncées. La fiche est marron opaque.
- Bordeaux #762E40 : action principale, progression et vitesse sélectionnée.
- Texte secondaire #D8C6B1 sur marron.
- Typographie sans empattement existante, graisse pour titres et valeurs, corps lisible ; légendes discrètes. Aucun téléchargement de police.
- Marges de panneaux 22 unités Slate, contours rectangulaires explicites, palette et styles regroupés dans CampusAcademicStyle.h.

Les icônes et leurs libellés restent associés. Les textes, valeurs et réactions des boutons viennent des systèmes existants. Aucun changement des règles de gestion, du calendrier ou du format de sauvegarde.

Le nouveau thème est séparé du style historique pour que sa mise au point ne modifie pas prématurément les menus et le planning. Les futurs écrans reprendront ces composants après validation.

## Vérifications

Contraste théorique de la correction opaque : six paires texte/fond vérifiées, résultats dans Saved/Tests/AcademicHUDOpaqueContrast.json. Ce calcul ne valide ni le dessin effectif des fonds ni les dimensions des textes en jeu.

La barre supérieure aligne les groupes horizontalement avec un espace extensible avant la date ; la fiche commence en dessous. Le dock peut revenir à la ligne et la fiche conserve son défilement. Les petits écrans et les soldes longs restent à observer dans le jeu.

## Essais pour valider le style en partie

1. Stop Play, Ctrl + Alt + F11, attendre le chargement réussi du patch, puis Play sur L_Campus_Work. Charger le campus avec F9 si nécessaire.
2. Vérifier la palette, la lisibilité des montants et des dates, les icônes, le survol des boutons et l'indication de la vitesse active.
3. Sélectionner le gymnase. Lire toute la fiche, faire défiler si nécessaire, ouvrir le planning et revenir. Tester sa croix et le lien vers les finances.
4. Tester construction, chemins, E/F/P, pause et ×1/×2/×4. Les clics sur l'interface ne doivent pas construire ou sélectionner sur le terrain.
5. Vérifier les messages de sauvegarde/chargement et les erreurs de placement. Reprendre la caméra : ZQSD, clic droit, molette.
6. Essayer 1280×720 et 1920×1080, puis redimensionner. Vérifier qu'aucun texte n'est coupé et qu'aucune commande n'est inaccessible. Les fenêtres du planning et du menu sont volontairement encore dans l'ancien style.

**Point d'arrêt : attendre la validation du style dans le jeu.** Ne pas appliquer le thème aux autres écrans avant cette validation.

Compilation du HUD universitaire : **Succeeded**, journal Saved/Logs/BuildAcademicHUD.log. Chargement Live Coding, affichage réel, redimensionnement et parcours des boutons restent à valider dans Unreal ; aucune validation visuelle automatique n’a été effectuée.

## Correction après capture utilisateur — 11 septembre 2026

La capture fournie révèle des fonds de fiche et de boutons absents. Remplacement des brosses arrondies par des brosses de couleur ; fiche et dock utilisent explicitement WhiteBrush, comme la barre visible dans la capture. Fiche marron opaque bordée, texte crème, boutons opaques avec états distincts, groupes supérieurs espacés et navigation agrandie. Le thème utilise une nouvelle instance pour éviter de réutiliser les styles statiques précédents après Live Coding.

La capture Windows a échoué deux fois : SetIsBorderRequired, interface non prise en charge (0x80004002). Le déclenchement distant de Live Coding n’a pas produit de nouveau patch confirmé. Aucune capture du nouveau rendu disponible ; validation visuelle et parcours fonctionnel en attente. Aucun mécanisme de surveillance de fichiers ajouté au jeu.
