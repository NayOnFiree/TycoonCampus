# Diagnostic isolé du panneau Gymnase 01

## Éléments observés

La capture utilisateur montre le HUD heritage actif : mêmes libellés, fiche et section « Activité et suivi ». Le journal Saved/Logs/TycoonCampus.log confirme le chargement réel de Segoe UI / Semibold à 11:02:27 UTC, et signale un glyphe manquant pour la flèche U+2197 du bouton. Les fonds personnalisés ne sont pas visibles ; le fond natif de la section dépliable, lui, est dessiné.

## Causes confirmées par le code

1. La courbe DPI héritée de BaseEngine.ini (UIScaleRule=ShortestSide, 720→0,666 ; 1080→1) s'applique aux widgets AddViewportWidgetContent via SGameLayerManager. La largeur déclarée de 252 unités n'était donc pas 252 pixels sous 1080p. Le panneau seul reçoit maintenant la compensation inverse, marges incluses. Aucun réglage global modifié.
2. Le helper Text activait AutoWrapText(true) pour tous les textes, y compris les valeurs placées dans des slots AutoWidth. Le nouveau helper local conserve les valeurs sur une ligne et n'autorise le retour à la ligne que pour les descriptions.
3. La flèche du bouton ne possède pas de glyphe dans la police Semibold chargée. Le libellé fonctionnel est conservé sans ce glyphe de substitution.

La conversion CSS pixels ×0,75 est correcte : FontConstants::RenderDPI vaut 96 et FreeType applique RenderDPI/72. Elle est conservée. Le réglage de police UMG à 72 ne justifie pas de modifier arbitrairement cette conversion Slate.

## Fonds : diagnostic encore partiel

Les PNG sont décodables, les couleurs sources ont une opacité de 1 ; aucun échec de chargement des PNG heritage n'est enregistré dans le journal consulté. Cela ne prouve pas leur dessin effectif. La cause exacte de l'absence des fonds personnalisés à l'exécution n'est pas confirmée.

Le correctif isole la fiche de cette voie : brosse native WhiteBrush, opacité explicite, bordure claire et fond #FAFAF7, en-tête #F1F0EC, bouton principal #82664E. Styles des boutons et de la jauge détenus par les widgets, sans réutiliser le singleton de styles entre patches Live Coding. Une ligne « GymInspector repair v1 » journalise les propriétés de l'ancienne brosse et de la brosse native au prochain chargement. Les contours sont rectilignes dans cette étape diagnostique ; les coins arrondis de la maquette restent un écart à traiter avant validation visuelle finale.

## Périmètre

Seulement la fiche Gymnase 01. Barre de ressources, horloge, navigation, objectif, planning et menus inchangés. Toutes les actions de la fiche conservées. Son conteneur complet est masqué pendant construction/chemins afin de ne pas intercepter les clics dans une zone vide.

## Validation

Compilation finale cible jeu : Succeeded, journal Saved/Logs/BuildGymInspectorFix.log. Cela ne valide pas le chargement du patch dans l’éditeur ni le rendu.

Capture Windows toujours bloquée par SetIsBorderRequired / 0x80004002. Aucune capture du panneau corrigé, aucun nouveau rendu observé. Pas de validation visuelle ni de conformité pixel à pixel annoncée. Vérifier après patch et nouvelle session Play : 0 %, 100 %, horaires, boutons, fonds sur terrain clair/sombre, largeur à 720p et 1080p, section dépliable, fermeture et clics sur le terrain.
