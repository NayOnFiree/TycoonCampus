# Matrice reproductible de captures UI — jalons 9–12

Branche `codex/ui-visual-capture-matrix`, empilée sur le planning adaptatif.
Le harnais ajoute seulement `Scripts/Capture-UI.ps1` et le test dédié
`TycoonCampus.Visual.CaptureUI`. Il ne modifie ni widgets, routage, commandes,
économie, sauvegarde, configuration partagée ni assets.

## Exécution

Coordonner un créneau Unreal unique avec les autres tâches, puis :

```powershell
git lfs pull
pwsh -NoProfile -File Scripts/Capture-UI.ps1
```

Le script crée un projet jetable sous `Saved/Tests/UIVisual/<identifiant>/Project`,
compile Editor et lance six processus successifs avec un vrai RHI et
`-RenderOffscreen` : 1280×720 et 1920×1080, chacune avec échelle UI effective demandée
1, 1,25 et 1,5. Chaque cas possède son propre `-UserDir`, rapport, journal et dossier
de captures. Aucun fichier Saved ou sauvegarde joueur n'est copié ni chargé.
Le test construit le gymnase par la commande existante, le sélectionne, conserve
le temps en pause et capture le HUD puis le planning vide. Aucun appel de sauvegarde.
La caméra est centrée sur le gymnase. Les infobulles sont désactivées temporairement
et restaurées en fin de test pour éviter une infobulle dépendant de la position du
curseur réel ; aucun mouvement de curseur natif n'est injecté.

Pour limiter une première vérification :

```powershell
pwsh -NoProfile -File Scripts/Capture-UI.ps1 -Resolution 1280x720 -Scales 1
```

`-BuildProject` permet de réutiliser un projet jetable déjà présent **dans le
Saved/Tests de ce worktree**. Les chemins redirigés sont refusés ; le script copie
les sources/configurations/assets du checkout, ne supprime rien et refuse les anciens
fichiers source supplémentaires dans le cache. `-SkipBuild` n'est accepté qu'après
une compilation du harnais réussie, avec les mêmes empreintes Source/Config/uproject
dans le checkout et la copie. Les sources modifiées sont datées après la copie pour
éviter de réutiliser un objet compilé plus récent qu'un fichier remplacé.

Les assets LFS non hydratés provoquent un refus avant le lancement. Le délai par cas
est borné (240 s par défaut) ; seul le processus lancé par le script est arrêté en
cas de dépassement. Aucun processus tiers ni session éditeur n'est fermé.

## Nature des captures et limites DPI

`FScreenshotRequest` capture l'UI Slate avec le viewport, via le renderer moteur.
Le test refuse explicitement `-nullrhi` et un moteur sans rendu. Il vérifie la présence
du widget planning, les dimensions exactes du viewport et du PNG, l'existence de pixels
non uniformes et l'écriture des fichiers. Cela ne prouve pas l'absence d'un défaut
visuel : les PNG doivent être examinés. Il attend au moins cinq secondes de rendu et
la fin de la compilation des shaders, avec un délai borné. Aucun test de référence pixel à pixel ajouté,
aucune référence visuelle validée remplacée.

Les échelles 100/125/150 % modifient uniquement
`UUserInterfaceSettings::ApplicationScale` en mémoire, restauré à la fin du test.
Le test mesure d'abord la courbe de résolution à multiplicateur 1, puis applique
`ApplicationScale = échelle demandée / courbe mesurée`. Ainsi, 125 % demande réellement
1,25 unité de rendu par unité UI, au lieu de 0,666 × 1,25 à 720p. La courbe initiale,
le multiplicateur appliqué et le résultat effectif sont tous consignés.
Ce n'est **pas** un changement du DPI Windows. Dans UE 5.8, `SGameLayerManager`
retire l'échelle plateforme avant d'appliquer la courbe UI ; modifier l'échelle globale
de Slate ne constitue donc pas une émulation fiable du DPI système. Le test consigne
le DPI réel de sa fenêtre, l'échelle globale Slate, le résultat de la courbe UI et
la géométrie allouée au planning. Il ne modifie aucune préférence Windows.

Nom : `1280x720_ui125_hud.png`, `1280x720_ui125_planning.png` et JSON correspondants.
Chaque JSON distingue dimensions demandées/capturées, multiplicateur UI, échelle
effective, DPI de fenêtre et état de partie. `matrix.json` conserve le commit,
l'état modifié du checkout, les empreintes source, le moteur, les heures UTC et
le résultat de chaque cas et le RHI/GPU du rapport moteur. Les avertissements sont
conservés dans le statut `passed-with-warnings` et dans le manifeste ; ils ne deviennent
ni un faux échec de capture ni un succès sans réserve. Les captures restent sous Saved,
hors versionnement.

Le rendu hors écran ne prouve pas les gestes souris, le focus visible en fenêtre
normale, les listes ouvertes ou les DPI système 125/150 %. La fixture de cette première
matrice est vide : semaine pleine, refus long et suppression à confirmer restent des
extensions distinctes. Aucun succès NullRHI n'est présenté comme succès visuel.

## Résultats de cette livraison

Huit suites natives réussies :
`Saved/Tests/Native/36bdb12b6ae84752b0eb8f4d8d61bca7/results.json`.
Parsing PowerShell, refus d'un BuildProject égal au checkout et refus de SkipBuild
sans compilation préalable vérifiés.
Compilation Editor finale réussie (19,43 s), puis **six cas RHI réussis et douze PNG
examinés**. Cinq cas sont `passed-with-warnings` : glyphe U+2161 introuvable pour le
bouton pause, conservé dans les rapports. Le cas 1080p UI100 est `passed` sans
avertissement enregistré pendant le scénario. Aucune erreur de capture ou de géométrie.

Sorties locales : `Saved/Tests/UIVisual/f8bbf6817887457d9c8c26cf0914c1aa/`.
Ce dossier contient `build.log`, `matrix.json`, puis six dossiers de cas avec leurs
deux PNG, JSON et `index.json` Unreal. Matériel déclaré par le moteur : GTX 1660,
D3D12 SM6, Windows ; captures hors écran. DPI de fenêtre mesuré : 1 pour les six cas.

| Résolution PNG | Échelle UI effective | Espace planning mesuré | Vue observée |
|---|---|---|---|
| 1280×720 | 1 | 1280×720 | Grille et détail côte à côte |
| 1280×720 | 1,25 | 1024×576 | Semaine compacte, bouton Voir détail |
| 1280×720 | 1,5 | 853,33×480 | Semaine compacte, bouton Voir détail |
| 1920×1080 | 1 | 1920×1080 | Grille et détail côte à côte |
| 1920×1080 | 1,25 | 1536×864 | Grille et détail côte à côte |
| 1920×1080 | 1,5 | 1280×720 | Grille et détail côte à côte |

Constats visuels (pas de correction du HUD dans ce sous-jalon) :

- Planning : sept en-têtes de jours visibles ; fermeture/navigation dans le panneau.
  En compact, les dernières heures demandent un défilement. Aucun geste de défilement
  ni ouverture de détail compact n'est déduit de ces images statiques.
- HUD : « CAMPUS » se coupe avant son S à UI125/150 ; le symbole EUR du résultat
  hebdomadaire revient à la ligne en 1080p UI125/150. Le bouton pause affiche un glyphe
  de remplacement. À 720p UI150, la fiche et l'horloge sont presque jointives.
- La fiche de gymnase garde une taille physique stable tandis que les autres zones
  du HUD grossissent, conformément à sa compensation locale préexistante.

Le premier essai exploratoire 720p avait déjà produit deux images mais le script
ne reconnaissait pas `succeededWithWarnings`. La version finale conserve ce statut
explicitement. Une infobulle dépendant du curseur observée lors de cet essai a motivé
sa suppression temporaire dans le harnais final. Les douze images ci-dessus proviennent
de la version finale, sans infobulles.

Étape suivante : intégration de la pile UI par la tâche d'intégration. Aucun nouveau
sous-jalon lancé. Les écarts HUD et l'extension de la fixture aux refus/semaine dense
restent proposés pour une demande ultérieure.
