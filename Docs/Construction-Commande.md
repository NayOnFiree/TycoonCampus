# Commande commune de construction

Le parcours en jeu précédent (construction, chemins, séance, sauvegarde et panneaux) a été validé par le joueur. Cette étape stabilise la construction avant de poursuivre l'extraction des autres commandes.

## Changement

`FCampusConstructionService` est l'entrée commune pour l'aperçu et la confirmation. Le bouton Gymnase et la touche B activent déjà le même outil ; sa confirmation passe désormais par `Execute`, comme le test Unreal.

- `Evaluate` vérifie la présence d'un unique bâtiment, d'une horloge et du système de chemins initialisé ; la construction existante ; l'orientation ; la parcelle et l'entrée ; les chemins et obstacles ; le budget.
- `Execute` réévalue le monde courant, utilise l'heure de la session et applique l'achat. Aucun appelant ne fournit un ancien résultat de validation ni une heure arbitraire. Le bâtiment retourné n'est renseigné qu'en cas de succès.
- Le résultat est un enum explicite, traduit en message par `StatusText`. Le pawn conserve la projection du curseur, l'aperçu graphique et la sélection après achat.
- L'application sur l'acteur est privée (`ApplyConstruction`) et accessible au service. Le coût de 60 000 EUR est partagé par les vérifications et le débit via `FCampusOperations::GymCost`.

Ainsi, un emplacement qui était valide ne peut pas être acheté si un chemin apparaît ou si le budget devient insuffisant avant la confirmation. Une seconde exécution ne débite pas une seconde fois.

Le service s'exécute sur le thread de jeu et ne contient aucune opération asynchrone entre validation et application. Les règles existantes de géométrie et d'obstacles sont conservées. La grille de placement de 4 m reste calculée par l'outil ; les empreintes en grille de 2 m restent acceptées par la commande pour préserver les coordonnées existantes.

## Vérification

Les sept suites natives couvrent toujours la simulation et la sauvegarde. Le scénario Unreal `TycoonCampus.Runtime.BootAndPanels` utilise maintenant la commande réelle et vérifie en plus : hors parcelle, entrée hors parcelle, rotation invalide, empreinte incompatible, chemin apparu après l'aperçu, budget insuffisant, état inchangé sur refus et double achat interdit. Il conserve la vérification du menu et du planning.

Commandes depuis la racine du projet :

```powershell
pwsh -NoProfile -File Scripts/Test-Native.ps1
pwsh -NoProfile -File Scripts/Test-Unreal.ps1 -Isolated
```

Le test isolé ne recharge pas la DLL de l'éditeur du joueur. Pour utiliser cette version dans la session habituelle, fermer l'éditeur, compiler la cible Editor puis rouvrir le projet. Le test NullRHI vérifie les actions et les états ; il ne simule pas les gestes souris.

Résultats du 11 septembre 2026 : sept suites natives réussies et compilation Editor + scénario Unreal étendu réussis (1 succès, aucun échec). Rapports précis consignés dans Avancement.md. La vérification moteur a réutilisé la copie isolée de l'audit, avec Source/Config/Content synchronisés, pour conserver son cache de compilation et laisser l'éditeur du joueur ouvert.

## Suite

L'extraction des chemins est décrite ci-dessous. Prochain sous-jalon : centralisation des transitions entre outils. Le budget global du campus et le multi-bâtiment restent une étape ultérieure : le budget est encore porté par le modèle d'opérations du gymnase.

## Commande commune des chemins — 11 septembre 2026

Jalon actuel : stabilisation des jalons 9–12, extraction de l'achat et de la suppression des chemins.

`FCampusPathService::Evaluate/Execute` remplace `ACampusPaths::Commit` et le contrôle économique du pawn. Le service prend le monde et le mode achat/suppression ; sa requête est la sélection courante de la grille (`Segment`), produite par le rectangle de l'outil. `Evaluate` ne modifie pas cette sélection ni le monde et retourne un résultat explicite avec le devis. `Execute` revalide la sélection et le budget courants, puis utilise l'heure de l'horloge de la session pour le journal et le raccordement. Aucun devis ancien ni horaire arbitraire n'est accepté par l'exécution.

Le service exige un bâtiment, un système de chemins initialisé et une horloge uniques. Il délègue les règles géométriques à `FCampusPathGrid::Quote/Apply` : obstacles de la grille, chemin public protégé, achat atomique et prix des seules cellules neuves. Comme auparavant, les obstacles statiques sont importés au démarrage et l'empreinte du gymnase est ajoutée lors de sa construction. Aucun nouveau scan des acteurs n'est introduit.

Le débit, l'investissement et le journal sont coordonnés sur le thread de jeu, sans opération asynchrone entre validation et application. Seule une exécution acceptée rafraîchit les instances et le parcours. Une répétition d'achat reste un succès à zéro euro, sans nouvelle entrée au journal ; une répétition de suppression est également sans effet économique. La suppression reste sans remboursement, y compris lorsque le solde est négatif. Les coordonnées de 2 m, rectangles de 4 m, précision Maj, gestes, messages existants et format des sauvegardes sont conservés.

Besoin transversal coordonné avec l'intégration : uniquement les blocs d'aperçu et de confirmation des chemins de `CampusCameraPawn.cpp`, plus l'inclusion du service. Aucun changement du HUD, de `CampusBuilding`, des assets ou de `Docs/Avancement.md`.

Le scénario moteur `TycoonCampus.Runtime.BootAndPanels` teste désormais la commande des chemins : devis exact, budget devenu insuffisant après aperçu, obstacle ajouté à la grille après aperçu, suppression mêlant cellule achetée et chemin protégé, achat, suppression sans remboursement avec solde négatif, répétition des deux commandes et sélection vide. Les refus comparent toute la grille (sélection et route comprises), le modèle économique avec le journal complet, les points du parcours, le raccordement, sa date de disponibilité, sa distance, le nombre d'instances et la transformation du gymnase. Les succès contrôlent aussi le débit, l'investissement, le journal et le rafraîchissement des instances.

Validation locale : sept suites natives réussies via `Scripts/Test-Native.ps1`, rapport `Saved/Tests/Native/2b683c577654492b8abd8f7e087c2de4/results.json`. Compilation Editor réussie et scénario Unreal étendu réussi (1 succès, 0 échec) via `Scripts/Test-Unreal.ps1 -Isolated`, rapport `Saved/Tests/Unreal/1bc2b23dd31f439fb4adad9657c76213/index.json`. Les assets LFS du nouveau worktree et de sa copie isolée ont été hydratés avant le chargement de la carte ; aucun asset modifié.

Limites : aucun rendu ni geste souris observé pour cette extraction ; NullRHI valide les commandes et états uniquement. La session Unreal du joueur reste ouverte et sa DLL n'est pas remplacée. Prochain sous-jalon : centraliser les transitions entre outils, après intégration de cette PR.
