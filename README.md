# TycoonCampus

Prototype de gestion de campus avec Unreal Engine **5.8**, C++ et interface Slate.

## Démarrage

Prérequis : Git + Git LFS, Unreal Engine 5.8, outils C++ Visual Studio et SDK Windows
(voir `.vsconfig`), PowerShell 7. Pour les tests de modèles : g++ avec C++17.

```powershell
git clone https://github.com/NayOnFiree/TycoonCampus.git
cd TycoonCampus
git lfs install --local
git lfs pull
```

Générer les fichiers Visual Studio depuis `TycoonCampus.uproject`, compiler la cible
`TycoonCampusEditor` en Development, puis ouvrir le projet. Carte : `L_Campus_Work`.
Les solutions, DLL, sauvegardes et caches sont régénérés localement et exclus de Git.

## Tests

```powershell
pwsh -NoProfile -File Scripts/Test-Native.ps1
pwsh -NoProfile -File Scripts/Test-Unreal.ps1 -Isolated
```

Le second script accepte `-EngineRoot 'X:/Epic/UE_5.8'`. L'isolation conserve l'éditeur
du joueur ouvert. Résultats dans `Saved/Tests`. La CI `Native tests` exécute les mêmes
suites C++ sur GitHub à chaque PR et push sur main. Le workflow Unreal est distinct et
nécessite un runner Windows portant le label `ue-5.8` ; voir le guide de collaboration.

## Travail en parallèle

Lire [AGENTS.md](AGENTS.md) et [Docs/Collaboration.md](Docs/Collaboration.md).
Trois domaines : jouabilité, UI/UX et graphisme, chacun dans un worktree distinct.
`main` reste l'intégration ; une PR et les contrôles précèdent chaque fusion.

État : [Avancement](Docs/Avancement.md). Architecture : [audit](Docs/Audit-Architecture-Qualite.md).
Dernière extraction : [commande de construction](Docs/Construction-Commande.md).
