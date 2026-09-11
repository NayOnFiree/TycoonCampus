# Planning compact — proposition UI/UX, 11 septembre 2026

Jalon : stabilisation des jalons 9–12. Livraison documentaire uniquement, basée sur
`e061feb`. Aucun widget, règle économique, format de sauvegarde ou asset modifié.
La direction Heritage est retenue ; cette proposition ne vaut pas validation du rendu
Slate. Les dernières entrées d'Avancement priment sur les anciens guides.

Maquette autonome : [planning-compact.html](../UI-Concepts/planning-compact.html).
Données illustratives, sans lien avec une partie ; aucun enregistrement réel.
La référence `campus-studio.html` reste intacte.

## Audit du parcours (lecture du code, pas essai en jeu)

| Étape | Contrat existant et source | Risque / proposition |
|---|---|---|
| Construction | `CampusCameraPawn::UpdateConstruction/ConfirmConstruction` appelle `FCampusConstructionService::Evaluate/Execute`. Refus budgétaires et spatiaux explicites. | Conserver coût, motif et geste dans le message de placement. Tester texte long sur terrain clair/sombre et clic sur zone vide du HUD. |
| Raccordement | `UpdatePaths/FinishPaths` montre prix, suppression sans remboursement, obstacle et fonds insuffisants ; abandon au relâchement hors terrain. `OpenAlert` dirige vers chemins si déconnecté. | La consigne planning dit « avec C », mais `TogglePaths` refuse pendant un panneau modal. Proposer « Fermer [P], puis Chemins [C] » ; ne pas promettre une action C depuis le formulaire. |
| Planning | `SCampusPlanningPanel::Construct/BuildTimetable` : semaine type, préparation séparée, sélection de la réservation entière, formulaire défilant. | Priorité P1 : taille issue du viewport, navigation chargée et erreurs/actions en bas du défilement. Séparer en-tête, contenu et zone d'action persistante. |
| Exécution | Alerte sans encadrant enregistrable ; `GetSelectionText` indique que la séance commencée conserve ses conditions. Suivi réel dans Heritage. | Distinguer « Réservation enregistrée » de « Séance assurée ». Garder la règle des prochaines occurrences près du brouillon. Ne pas transformer l'alerte en interdiction. |
| Finances | `MakeFinance` distingue trésorerie, investissement, exploitation et semaine précédente ; journal des 128 dernières écritures. | Préserver ces distinctions. Tester montants longs et motifs retard/inaccessible sur plusieurs lignes. Éviter la confusion entre réservation et recette immédiate. |
| Sauvegarde | F5/F9/F10 dans `OnPreviewKeyDown`, boutons et statut dans le menu ; chargement réussi ferme le planning (`CampusSaveService`). | Brouillon non sauvegardé : rappel permanent dans le formulaire. F9/F10 remplacent la partie sans nouvelle confirmation. Garder ce comportement et son avertissement. Le vieux guide Jalon 11 situe encore les boutons dans Personnel/finances ; le code actuel les place au menu. |

## Focus, Échap et protection du monde

- `TogglePlanning` annule les outils, ouvre le panneau au Z=20 et lui donne le focus.
  `ClosePlanning` rend le focus au viewport. `SelectCell` abandonne le brouillon et
  réinitialise la confirmation ; `RefreshTimetable` reconstruit les boutons : retour
  de focus après création/suppression à vérifier, aucune garantie explicite sur la case.
- Le préprocesseur `FCampusMenuInput` intercepte Échap si viewport/interface a le focus.
  `ToggleMenu` ferme le planning, annule les outils et met en pause. Un deuxième Échap
  ferme le menu en restant en pause. P ferme seulement le planning. Conserver ce contrat,
  y compris l'abandon du brouillon, et l'expliquer ; ne pas proposer « Échap annule ».
- Espace et 1/2/3 sont interceptés avant les boutons du planning pour pause/vitesse.
  Tester aussi une liste déroulante ouverte : l'ordre réel de routage et le focus du
  popup ne sont pas prouvés par lecture. Les boutons du menu sont `IsFocusable(false)` :
  une navigation complète Tab/Entrée ne peut pas être annoncée comme acquise.
- `CampusCameraPawn::IsPlanningOpen` interroge **IsModalOpen** (planning OU menu).
  Sélection, rotation, zoom et Tick bloquent les actions monde sous ces panneaux.
  `CancelTools` efface mouvement et ancre de chemin. `FinishPaths` refuse hors terrain.
- Le HUD non modal consomme appuis/molette via `SCampusInterface` sur enfants visibles ;
  les chemins contrôlent le widget sous le curseur. Construction/sélection n'ont pas ce
  même contrôle local et reposent sur Slate pour les clics HUD. C'est un point de test,
  pas une fuite démontrée. Aucun acteur partagé modifié sans coordination d'intégration.

## Dimensions et DPI : ce que le code permet de conclure

À viewport 1280×720, les valeurs demandées sont 1180×656 unités Slate, bordure intérieure
20, formulaire 320, intervalle 14, grille demandée 780 (minimum 620). La grille reçoit
au plus 806 unités avant les détails de layout. Les 14 lignes font 448 unités plus
en-tête. Le résumé, besoins et titre consomment encore de la hauteur. Les deux axes de
défilement existent, mais les actions et le refus sont dans le scroll du formulaire.

Le diagnostic GymInspector documente la courbe héritée 720→0,666, 1080→1 ; la compensation
est locale à la fiche, absente du planning. Ne pas recopier l'inverse sans mesure.
Le viewport en pixels, l'espace alloué Slate, l'échelle applicative et le DPI du moniteur
ne sont pas interchangeables. Aucun facteur Windows effectif n'a été mesuré ici.

| Windows demandé | Cas de stress conceptuel si 1280×720 pixels / facteur OS | État réel |
|---|---|---|
| 100 % | 1280×720 unités de conception | Non observé dans Unreal |
| 125 % | 1024×576 | Non observé dans Unreal |
| 150 % | environ 853×480 | Non observé dans Unreal |

Ces quotients servent uniquement à éprouver la maquette. Ils ne prédisent pas la
géométrie Unreal et ne simulent ni la courbe DPI Unreal ni le DPI Windows réel.
Mesurer taille cliente (hors bordure), taille viewport, géométrie allouée et échelles
effectives à chaque essai ; préciser PIE intégré, fenêtre séparée ou standalone.

## Proposition limitée

Surfaces #FAFAF7 / #F1F0EC, texte #303735, secondaire #66706C, accent #82664E,
lignes #DDE1DB, Segoe UI locale avec repli sans-serif : valeurs Heritage révision 02.
Pas de nouvelle police, pas de nouveau décor. Les états portent des mots, pas seulement
une couleur. Corps proposé 13–14 unités de conception ; confirmer en pixels rendus.

- Marges 16 ; panneau occupant l'espace disponible, plafond 1248×688 dans la maquette.
  En-tête titre/fermeture puis trois onglets uniques Planning, Personnel, Finances.
  Retirer la duplication actuelle « Personnel et contrats » / « Personnel ».
- Au-dessus de 1100 unités disponibles : grille flexible + formulaire 304, espace 12.
  En dessous : une seule vue à la fois, Semaine / Détail ; la sélection passe au détail,
  retour explicite à la semaine. Aucun champ ni bouton réduit pour tenir artificiellement.
- Grille minimale 672, heures 56, sept jours 88 ; lignes 32, en-tête fixe et défilement
  vertical. Aux tailles inférieures, défilement horizontal local si nécessaire.
  Préparation distincte de l'activité ; libellé court dans le bloc, détail complet
  dans le formulaire (ne pas dépendre uniquement d'une infobulle).
- Formulaire : titre court, champs défilants, rappel des occurrences/brouillon ; refus
  et actions hors du scroll des champs. Refus complet avec retour à la ligne ; si un
  texte exceptionnel dépasse la réserve, zone de message défilante accessible au clavier.
  Valider, annuler et supprimer restent atteignables ; confirmation garde deux choix.
- Besoins hebdomadaires repliables, compteurs synthétiques persistants. Aucune nouvelle
  valeur simulée. Personnel/Finances et menu ne sont pas redessinés dans cette livraison.

Maquette : bascule des trois espaces de stress, sélection d'une case, vue Détail/Semaine,
états refus/avertissement/succès illustratifs et besoins dépliables. Les résultats ne sont
pas calculés et les onglets Personnel/Finances sont indiqués hors maquette.

## Recette à exécuter avant intégration Slate

Répéter à 1280×720 **client** et DPI Windows 100/125/150 %, puis 1920×1080 de contrôle.
Utiliser une copie de test et des sauvegardes isolées ; ne pas fermer la session joueur.

| Cas | Gestes et critères de réussite |
|---|---|
| Accès | Avant construction P refuse avec une solution lisible ; après achat P ouvre ; accès Chemins après fermeture sans placement accidentel. |
| Densité | Semaine vide puis pleine, dimanche 21–22, préparation à 08 h, titre long ; sept jours et dernière heure atteignables, champs/action/refus sans recouvrement. |
| Erreurs | Conflit activité/préparation, fin après 22 h, Camille indisponible, contrat non accepté ; refus entier visible et original intact. Sans encadrant reste une alerte enregistrable. |
| Focus | Tab/Maj+Tab, Entrée, dropdown, molette, clic sur préparation, création/modification/suppression ; focus visible et stable, retour clavier au viewport après P. |
| Échap | Depuis champ, bouton, dropdown et suppression à confirmer : menu ouvert une seule fois, brouillon abandonné, pause ; deuxième Échap ferme sans reprise. Espace/1/2/3 ne valident pas une case. |
| Monde | Clic gauche/droit, glisser commencé terrain puis relâché sur panneau, molette et ZQSD sur zones vides/scroll/barres ; aucune construction, sélection, rotation, zoom ou achat sous modal. Refaire sur HUD non modal. |
| Boucle complète | Construction → chemins → séance → exécution → journal → F5 → modification → F9/F10 ; reprise en pause, une seule écriture par paiement, succès/échec lisible. Sauvegarde joueur intacte. |

Captures attendues : chaque DPI, semaine pleine et refus long ; joindre mode de fenêtre,
taille cliente, DPI, commit, état de partie. Noter séparément les résultats clavier/souris.

## Validation de cette livraison

Audit statique seulement. Aucun lancement Unreal, capture Unreal, geste réel ni mesure DPI
Windows réalisé. Aucun test NullRHI ni compilation Unreal requis pour ces documents.
Sept suites natives réussies via `pwsh -NoProfile -File Scripts/Test-Native.ps1` ; rapport local : `Saved/Tests/Native/08ef237b917e4cc5a00dc93c0656a2f6/results.json`. CI suivie dans la PR. Aucun rendu navigateur ni interaction de maquette observé pendant cet audit. La maquette n'est pas une
capture du jeu et sa conformité visuelle dans un navigateur reste à distinguer du moteur.

Prochain sous-jalon : observer la matrice DPI et retenir les dimensions effectives, puis
implémenter uniquement le conteneur adaptatif et la zone de refus/actions du planning.
La centralisation future des commandes reste au domaine jouabilité : les appels directs
Schedule.Add/Update/Remove déjà présents dans Slate ne sont pas étendus ici.

