# Automatisation des entrées des outils

Jalon : stabilisation des jalons 9–12, sous-jalon empilé sur les transitions entre outils (PR #5). Aucun changement de geste, de rendu, d'économie ou de sauvegarde.

## Frontières exercées

Le nouveau scénario `TycoonCampus.Runtime.ToolInput` vérifie les bindings réellement installés sur le pawn. Il injecte des événements pressé/relâché avec `APlayerController::InputKey`, puis traite le composant d'entrée du pawn via `UPlayerInput::ProcessInputStack`. Il couvre B, C, P et Échap, en contrôlant le mode exclusif, l'état des fenêtres et le masquage du curseur à chaque étape. Il ne remplace pas les bindings par des appels directs aux actions pour ces séquences.

Les événements P et Échap passent aussi par `FSlateApplication::ProcessKeyDownEvent/ProcessKeyUpEvent`, avec le focus du viewport ou du planning/menu réel. Le scénario vérifie qu'Échap maintenu n'effectue pas une seconde transition et que P/Échap ferment le panneau focalisé. Le préprocesseur et les callbacks Slate de production participent à ce parcours.

Pour les clics, la frontière privée `HandleWorldPress(const FWidgetPath&)` est appelée par le vrai gestionnaire du clic, immédiatement après `LocateWindowUnderMouse`. Le prédicat `IsTerrainHit` est aussi partagé par les vérifications de curseur de production. Le scénario ajoute temporairement un `SBorder` au viewport, obtient son chemin de widgets réel avec `GeneratePathToWidgetUnchecked`, et le transmet à cette frontière. Il vérifie également qu'un chemin vers le viewport est reconnu comme terrain et qu'un chemin vide est refusé.

Dans chacun des cinq modes, le clic de panneau suivi d'un événement relâché envoyé au binding Unreal doit conserver la grille, le modèle économique/journal et l'acteur sélectionné, tout en annulant le geste provisoire. Le panneau temporaire, les touches et le mode sont nettoyés en sortie. Le scénario peut préparer son gymnase avec la commande de construction lorsqu'il est exécuté seul ; il ne dépend pas d'une sauvegarde joueur.

## Périmètre

Modification coordonnée de `CampusCameraPawn.h/.cpp` : surcharge privée et prédicat de classement du chemin Slate, avec accès du test. Aucun changement du HUD, du modèle de transitions, des commandes économiques, du codec, du format des sauvegardes, des assets ou des configurations. Les PR #4/#5 restent inchangées ; la nouvelle PR cible `codex/tool-mode-transitions`.

## Validation et limites

Huit suites natives réussies via `Scripts/Test-Native.ps1`, rapport `Saved/Tests/Native/8691989c796943c489eb1bbe3003d303/results.json`. Compilation Editor et deux scénarios Unreal réussis (BootAndPanels et ToolInput, 2 succès, 0 échec) via `pwsh -NoProfile -File Scripts/Test-Unreal.ps1 -Isolated -TimeoutSeconds 600`, après libération du créneau par l'intégration. Rapport : `Saved/Tests/Unreal/413723780303403cbe8bae20bdd26960/index.json`. Sortie code 0 et processus terminé. La carte du worktree et de sa copie est hydratée sous LFS (45 111 octets), aucun asset modifié. Le fichier du nouveau test dans la copie compilée est identique à celui livré (SHA-256 vérifié).

La frontière clavier Unreal, les bindings et le routage clavier Slate sont testés avec des événements synthétiques. La souris n'est pas déplacée au niveau Windows. Le parcours de widgets du panneau est réel, mais sa résolution géométrique par `LocateWindowUnderMouse`, la capture native, le glisser physique et le rendu ne sont pas observés sous NullRHI. Il ne s'agit donc pas d'une validation de gestes souris natifs. Les gestes manuels listés dans `Outils-Transitions.md` restent à observer.

Prochain sous-jalon : validation interactive du hit-test et des gestes de terrain après intégration des PR empilées.
