# Bible graphique Heritage — proposition 01

11 septembre 2026 · Domaine graphisme · Jalon actuel : stabilisation des jalons 9–12.
La direction de la maquette HUD est validée ; cette déclinaison 3D est une proposition à
valider. Elle prépare uniquement [le pilote gymnase](Pilote-Gymnase.md).

## Références et intention

Un campus européen fictif, semi-réaliste, chaleureux et entretenu : grandes masses
lisibles, matière mate, paysage calme et équipements sportifs contemporains.

| Référence du dépôt | À retenir |
|---|---|
| [Heritage révision 02](../Design/Heritage-HUD-Integration.md) et [maquette HTML](../UI-Concepts/campus-studio.html), CSS final | Crème neutre, tabac, finesse des détails ; seule référence actuelle de palette HUD |
| [Conception, §1](../Jalon-00-Conception.md) | Architecture européenne fictive, végétation, formes lisibles ; Cities: Skylines pour le développement, Planet Zoo pour la construction, Two Point Campus pour la lisibilité, Big Ambitions pour les tableaux |
| [Géométrie existante](../../Source/TycoonCampus/CampusGymGeometry.h) | Halle rectangulaire, grande ouverture axiale, terrain visible depuis la caméra aérienne |

Ces jeux sont des références d'intention consignées dans le projet, pas des banques
d'assets. Aucun visuel externe n'est copié. Le décor SVG de la maquette n'est pas un
plan architectural validé. Les anciennes palettes bleu nuit/menthe et marron/bordeaux
ne constituent pas la cible. L'intégration du HUD dans Unreal reste à observer.

## Palette

Valeurs hexadécimales en sRGB : repères de conception, pas couleurs finales garanties
sous l'éclairage Unreal. Les teintes 3D sont proposées ; les quatre premières reprennent
les variables de la maquette. Vérifier leur conversion à l'import et sous exposition fixe.

| Teinte | Valeur | Usage |
|---|---|---|
| Ivoire neutre | `#FAFAF7` | Référence HUD ; petites inscriptions claires |
| Papier chaud | `#F1F0EC` | Référence HUD ; rapprochement avec les enduits |
| Encre | `#303735` | Référence HUD ; caractères et ferronnerie sombre |
| Tabac | `#82664E` | Accent Heritage ; encadrement d'entrée, touches de bois |
| Pierre chaude | `#C8BBA4` | Masse des façades, socle légèrement plus sombre |
| Brique sourde | `#A17863` | Bandeau ou travées limitées, sans mur rouge dominant |
| Sauge | `#78846A` | Pelouse ; variation de valeur modérée |
| Feuillage | `#4F644E` | Houppiers et arbustes, deux variantes proches |
| Gravier clair | `#B7B3A5` | Chemins, séparés de la pelouse par la valeur |
| Bois sportif | `#AF8964` | Terrain intérieur mat ; lignes ivoire existantes |

Cible de composition : environ 70 % de tons neutres, 25 % de végétal et 5 % d'accents
sur le cadrage pilote. La végétation abondante est une intention à terme ; le pilote
reste peu peuplé. Préserver les couleurs fonctionnelles existantes de sélection,
placement et raccordement ; ne pas les employer comme grands aplats décoratifs.

## Silhouettes et architecture

Le gymnase reste une halle basse et longue. Une façade minérale rythmée par de larges
travées et un encadrement tabac suffit à lui donner une identité universitaire. Le
rythme de façade est décoratif : il ne déplace ni murs, ni ouverture, ni emprise.
Éviter flèches, tours, blasons réels, ornements fins et extension annexe.

L'entrée se reconnaît par un contraste clair/sombre et l'inscription « GYMNASE 01 ».
Conserver l'ouverture entièrement libre. Pour ce pilote, le dessus reste ouvert comme
dans le blockout : le terrain et les figurants priment. Le toit masquable évoqué à la
conception n'est pas implémenté par cette proposition et demanderait un contrat séparé.

## Matériaux et paysage

| Famille | Traitement proposé |
|---|---|
| Enduit et pierre | Roughness indicative 0,75–0,9 ; joints larges et peu contrastés, aucune géométrie de joint |
| Brique | Relief discret par texture éventuelle ; densité cohérente entre travées |
| Bois | Roughness 0,55–0,75 ; veinage peu visible en vue moyenne |
| Métal peint | Mat, petites surfaces ; aucune réflexion dominante |
| Sol et chemins | Roughness 0,8–0,95 ; variation légère, sans motifs pouvant simuler la grille active |
| Végétation | Houppiers regroupés, troncs simples ; feuilles opaques stylisées pour le premier essai |

Ces plages sont des points de départ artistiques, pas des mesures de matériaux réels.
Privilégier des aplats paramétrés avant les textures. Pas de verre transparent, de
parallaxe, de tessellation ou de vent animé dans le pilote. Bordures suggérées par la
matière ; aucun rebord traversant la porte ou altérant le cheminement.

Deux arbres de même famille à couronne arrondie et quatre arbustes au maximum suffisent.
Leur placement futur doit laisser visibles l'entrée, la case cyan et les figurants aux
angles de caméra retenus. Aucun nouvel obstacle de navigation ou de construction.

## Éclairage, signalétique et personnages

Proposer une lumière de journée légèrement chaude avec ombres lisibles et ciel neutre.
Commencer avec le soleil et le ciel existants ; comparer avant de modifier leurs valeurs.
Fixer les conditions d'exposition pour les captures. Pas de cycle jour/nuit, brume,
météo ou éclairage local supplémentaire dans cette tranche.

Signalétique : un seul cartouche, fond ivoire et texte encre, sans logo tiers. Le nom
complet peut se lire au zoom proche ; la forme contrastée de l'entrée doit suffire de
loin. Ne pas graver « raccordé », disponibilité ou effectif dans une texture statique.
La typographie 3D est à résoudre avec une ressource dont la redistribution est autorisée ;
ne pas exporter Segoe UI depuis Windows. Un panneau uni suffit au premier essai matière.

Personnages : conserver les huit représentants maximum et leurs animations procédurales,
leurs couleurs d'usage et leurs trajets. À terme, formes adultes simplifiées, vêtements
sportifs sans marque et variations sobres ; pas de doigts, visages détaillés ou simulation
individuelle. Le pilote ne remplace pas les personnages et ne recolore pas les états.

## Conventions de fabrication et licences

- Nouveaux assets futurs : `Content/TycoonCampus/Art/Heritage/`, sous-dossiers `Gym`,
  `Landscape`, `Materials`, `Textures`. Noms ASCII : `SM_HER_Gym_EntryTrim_01`,
  `M_HER_Surface`, `MI_HER_Stone`, `T_HER_Stone_BC`, suffixes `N` et `ORM` si nécessaires.
- Unités de livraison : centimètres, Z vertical ; assemblage gymnase centré au sol,
  entrée vers -X en orientation initiale. Modules de façade : pivot au centre de leur
  base ; végétation : au pied du tronc ; surface de chemin : centre XY à sa base.
  Appliquer les transformations et vérifier avec un cube témoin de 100 cm à l'import.
- Ne pas confondre le pivot artistique au sol avec la racine actuelle du bâtiment,
  décalée de 430 cm : l'adaptation relève de l'intégration, sans déplacement de la racine.
- Conserver les UV dans un espace documenté et une densité homogène ; pas de texture
  unique haute résolution par petit objet. Les collisions décoratives restent désactivées.
- Inventaire de cette livraison : uniquement deux fichiers Markdown originaux. Aucun
  média, modèle ou fichier de police tiers ajouté ; aucune licence d'asset tiers à acquérir.
  Les formes `/Engine/BasicShapes` sont déjà référencées dans le code et restent dans
  le projet Unreal ; ce document ne leur attribue pas une licence de redistribution libre.
- Avant un futur import tiers, consigner dans `Docs/Art/Licences.md` : asset, auteur,
  URL source, licence/version, date de consultation, preuve conservée, attribution et
  droits de modification, d'usage commercial et de distribution dans ce dépôt public.
  Une autorisation d'utiliser dans le jeu ne suffit pas pour publier ses sources.
- Les extensions `.uasset`, `.umap`, `.png`, `.fbx`, `.blend` et polices sont déjà sous
  LFS dans [.gitattributes](../../.gitattributes). Vérifier toute nouvelle extension.
  Verrouiller les binaires partagés et coordonner leur propriétaire avant modification.

Blender n'est pas requis : primitives et matériaux Unreal suffisent à tester ces masses.
Si un asset spécifique dépasse ces moyens, fournir avant fabrication son nom, son besoin
visuel, les limites constatées des assets actuels/Modeling Tools, son format source et
d'échange, dimensions, axes, pivot, UV, matériaux, LOD et licence. Aucun travail Blender
n'est lancé ni implicitement autorisé par cette bible.
