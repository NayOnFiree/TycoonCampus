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

Même extraction pour l'achat/suppression des chemins, puis centralisation des transitions entre outils. Le budget global du campus et le multi-bâtiment restent une étape ultérieure : le budget est encore porté par le modèle d'opérations du gymnase.
