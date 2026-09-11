# Campus — 01 / heritage : intégration du HUD

Référence exclusive : Docs/UI-Concepts/campus-studio.html, thème `heritage`, CSS de la révision 02 (prioritaire dans la cascade). Les palettes de la carte de sélection sont anciennes : le rendu effectif utilise #FAFAF7, #F1F0EC et l'accent #82664E. Les autres thèmes et le décor SVG ne sont pas intégrés.

## Composition

- Barre de ressources : gauche 16, haut 16, hauteur 48, marge interne horizontale 16.
- Fiche : droite 16, haut 80, largeur 252 ; en-tête 46, corps 15. Défilement si nécessaire ; suivi supplémentaire repliable.
- Navigation : gauche 16, bas 16, marge interne 4, boutons horizontaux avec icône 17, intervalle 2.
- Horloge : droite 16, bas 16, largeur 262, commandes 28 × 28, progression du jour 2.
- Objectif : gauche 18, haut 128, largeur 215, marge interne 14 × 12.
- Segoe UI / Segoe UI Semibold installées sur Windows ; tailles CSS converties en points Slate (×0,75). Repli sur la police moteur si absentes. Aucun fichier de police Windows redistribué.
- Fonds PNG neuf zones chargés dynamiquement par Slate, bordures fines, coins et ombres. Assets dans Content/UI/Heritage et copie au packaging configurée.

## Données et actions

Budget = trésorerie ; semaine = résultat opérationnel réel. Réservations et personnel remplacent les étudiants et la satisfaction fictifs, non simulés globalement. Participants actuels remplacent une capacité de 32 non définie par le jeu. Horloge = minutes du calendrier interne ; aucun faux mois ni saison ajoutés. Les vitesses ×1/×2/×4 reprennent la simulation si elle était en pause, comme dans la maquette ; le bouton pause bascule pause/reprise.

Objectif = gymnase construit, raccordement et première réservation. Occupation = heures réservées / 98. Les commandes construction, chemins, planning, personnel, finances, alertes et menu restent branchées. Le suivi détaillé contient activité, retard, figurants, trajet, préparation et pratique hebdomadaire. Messages de placement et sauvegarde conservés.

## Vérification

Compilation finale cible jeu Win64 Development : Succeeded (92,41 s), sans avertissement dans cette dernière passe. Journal : Saved/Logs/BuildHeritageHUD.log. Les fichiers PNG sont décodables ; ceci ne valide pas leur affichage dans Slate.

La capture Windows échoue deux fois avec `SetIsBorderRequired`, interface non prise en charge (0x80004002). Aucune capture du nouveau HUD en jeu obtenue ; rendu, survols, chargement des images et polices non validés visuellement. Ne pas présenter une maquette ou un assemblage comme une capture Unreal.

Essais après Stop, Ctrl+Alt+F11, patch réussi, Play :

1. À 1280×720 puis 1920×1080 : barre compacte et entièrement visible, navigation en bas à gauche, horloge en bas à droite.
2. Sélectionner le gymnase : vérifier coins, fonds, contraste et défilement ; ouvrir Activité et suivi.
3. Ouvrir planning / personnel / finances, revenir, fermer la fiche, accéder au menu Échap.
4. Pause puis ×1/×2/×4 ; vérifier horloge, état sélectionné et reprise.
5. Construire et tracer des chemins ; les clics et glissers sur les panneaux ne doivent pas agir sur le terrain. Vérifier ZQSD, rotation, zoom.
6. Vérifier zéro réservation, séance en cours, gymnase non raccordé, alertes, gros montant et messages longs.

Le style de la maquette est validé ; son intégration en jeu reste à faire valider. Attendre cette validation avant le planning, personnel, finances, contrats, menu Échap et confirmation. Le menu principal est absent et reste à créer à l'étape suivante. Inventaire complet : Docs/Design/Refonte-Universitaire.md.
