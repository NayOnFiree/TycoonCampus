# Planning adaptatif — stabilisation des jalons 9–12

Suite limitée de la proposition `Planning-Compact-Audit.md`. Branche
`codex/ui-ux-planning-adaptive`, empilée sur `codex/ui-ux-planning-compact`.
La PR documentaire reste inchangée. Aucun changement de direction artistique :
couleurs, polices et blocs de réservation existants sont conservés ; ce sous-jalon
porte sur la disposition. L'application du style Heritage au planning reste distincte.

## Comportement

Le conteneur mesure l'espace local réellement alloué par Slate à chaque Tick,
au lieu de mélanger dimensions du viewport en pixels et unités de disposition.
Il conserve 16 unités de marge par bord et plafonne à 1248 × 740.
Il ne compense pas une seconde fois le DPI et ne modifie pas la configuration globale.

À partir de 1100 unités de largeur de panneau, semaine et formulaire de 320 unités
coexistent. En dessous, le bouton Voir détail / Voir semaine alterne les vues.
Sélectionner une case révèle son détail et transfère le focus vers le champ Jour
si la grille devient masquée. Revenir à la semaine conserve le brouillon ; sélectionner
une autre case conserve le comportement existant d'abandon de ce brouillon.

Les champs ont leur défilement propre, avec suivi immédiat du focus. Enregistrer,
annulation, suppression et confirmation restent hors de ce défilement. Le message
de refus/alerte conserve son texte complet dans une zone indépendante, plafonnée
à 84 unités et défilante si nécessaire. Les besoins sont repliables, avec une zone
plafonnée à 56 unités. Les deux zones textuelles acceptent le focus.

La navigation Planning / Personnel / Finances ne duplique plus l'accès Personnel.
Les pages de gestion gardent leur contenu. La consigne de raccordement indique
désormais de fermer avec P avant d'utiliser Chemins (C), conformément au blocage modal.

## Périmètre et contrats

- Modifications dans SCampusPlanningPanel.h/.cpp uniquement pour la présentation,
  plus la politique indépendante CampusPlanningLayout.h et son test natif.
- Aucun changement aux fonctions CreateReservation/DeleteReservation,
  OnKeyDown/OnPreviewKeyDown, aux commandes, à l'économie ou aux sauvegardes.
- Aucun changement à CampusHUD, CampusCameraPawn, CampusSaveService ni aux assets.
  Les blocs de transitions réservés à la jouabilité sont conservés.
- P ferme le planning ; Échap ouvre le menu et met en pause selon le contrat actuel.
  L'absence d'action monde dépend toujours des protections modales existantes.

## Validation et limites

Test natif ciblé : espace alloué 1280 × 720, 1024 × 576, 853,33 × 480 ; bornes,
marges, plafond, seuil exact et largeur disponible pour les sept jours en mode large.
Le test ne prétend pas valider l'arbre Slate, le rendu des polices ou les gestes.
Ces quotients servent au stress de disposition ; ce ne sont pas des mesures Windows.

Huit suites natives réussies via `pwsh -NoProfile -File Scripts/Test-Native.ps1`.
Rapport local : `Saved/Tests/Native/0facc3efefef42c6b4a1146d7733dfb2/results.json`.
Premier essai `Scripts/Test-Unreal.ps1 -Isolated` : compilation Editor réussie,
puis timeout runtime à 180 secondes pendant l'initialisation
TurnkeySupport/ValidatePlatforms, avant rapport de scénario. Journal :
`Saved/Tests/Unreal/5909378d8f8540ffaf0e11a61c051313/runtime.log`.
Cet essai ne valide pas le runtime.

Compilation Editor finale réussie (62,81 s) dans la même copie isolée :
`Saved/Tests/Unreal/5909378d8f8540ffaf0e11a61c051313/build-final.log`.
Le journal confirme la recompilation de SCampusPlanningPanel.cpp et la liaison
de la DLL ; le fichier source de la copie a la même empreinte SHA-256 que celui livré.
Le lancement suivant a terminé l'initialisation mais refusé la carte : le worktree
contenait encore le pointeur LFS de 130 octets de L_Campus_Work.umap. Journal :
`Saved/Tests/Unreal/5909378d8f8540ffaf0e11a61c051313/Project/Saved/Tests/Unreal/3e377f66c0a2428f91fd0ff3057a5df7/runtime.log`.
Les assets versionnés ont ensuite été hydratés par `git lfs pull` et recopiés dans
la copie de test (carte de 45111 octets). Aucun changement d'asset dans le diff.
Essai final après hydratation, seul, avec `Test-Unreal.ps1 -SkipBuild` exécuté
depuis la copie isolée : **1 succès, 0 échec** (BootAndPanels).
Rapport : `Saved/Tests/Unreal/5909378d8f8540ffaf0e11a61c051313/Project/Saved/Tests/Unreal/7b181f4e18b146fb9cc0c0d744546c92/index.json`.
Le scénario vérifie le démarrage, la construction et l'ouverture/fermeture des panneaux,
pas leur géométrie ni les vrais gestes. Aucun succès visuel n'est déduit de NullRHI.

Observation Windows tentée par Computer Use : `get_window_state` de la fenêtre
« TycoonCampus - Unreal Editor » répond que la fenêtre est minimisée et demande une
activation. Aucun rendu du nouveau planning n'a été obtenu. La session du joueur
n'a pas été activée, fermée, patchée ni utilisée pour tester ; aucune sauvegarde joueur
modifiée. Aucun changement des DPI système effectué. Pas de capture livrée.

La lisibilité effective, le focus visible, les listes déroulantes, les longs refus,
les confirmations et les vrais gestes souris/clavier restent à observer à 1280 × 720
client, Windows 100/125/150 %, puis 1920 × 1080. La présence des actions hors du scroll
des champs est implémentée ; leur accessibilité réelle à ces DPI reste à confirmer.
Les espaces nettement inférieurs à 853 × 480 unités allouées sont hors recette de ce
sous-jalon ; aucun minimum de panneau ne force toutefois un débordement du viewport.

Prochain sous-jalon : exécuter la matrice visuelle de l'audit sur cette version,
notamment semaine pleine, dimanche 21 h, besoins dépliés et refus long avec suppression
à confirmer ; corriger seulement les écarts observés avant validation du style.
