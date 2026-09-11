# Jalon 3.1 — Vue hebdomadaire du gymnase

## Commandes et périmètre

**P** ouvre ou ferme le planning. Le bouton **Fermer [P]** ferme également la fenêtre. Les sept colonnes vont du lundi au dimanche et les quatorze lignes de 08:00–09:00 à 21:00–22:00 : 98 créneaux d'une heure.

Le planning représente une semaine type récurrente, pas un calendrier daté. Toutes les cases sont libres dans cette étape. Cliquer sur une case la met en évidence et affiche son jour, son heure de début et son heure de fin. Une infobulle fournit aussi ces informations. Cela ne crée pas encore de réservation.

L'interface utilise une fenêtre Slate du jeu, avec défilement pour les petites résolutions. Les clics et la molette sont réservés à la fenêtre lorsqu'elle est ouverte. La caméra et la sélection du campus sont bloquées jusqu'à sa fermeture. La simulation continue dans son état précédent : **Espace** et les touches de vitesse restent actifs, même après avoir sélectionné une case. L'ouverture ne change pas automatiquement la pause.

## Organisation

`SCampusPlanningPanel` contient la présentation et l'inspection des cases. `CampusHUD` gère l'ouverture, la fermeture et le retrait du panneau à la fin de Play. La caméra respecte l'état du panneau. Les modules Slate et SlateCore sont explicitement déclarés.

La sélection d'une case est provisoire et remise à zéro quand on rouvre le panneau. Aucun horaire universitaire, contrat ou revenu n'est inventé dans cette vue vide. Création et conservation des réservations arrivent au jalon 3.2.

## Essai en jeu

Après compilation, relancer Play, appuyer sur P, vérifier les sept jours et la dernière ligne 21:00–22:00. Cliquer sur des cases, notamment lundi 08:00 et dimanche 21:00. Vérifier l'information en bas. Tester Espace et les vitesses après un clic de case. Fermer avec P puis avec le bouton, et vérifier que caméra et sélection refonctionnent. Vérifier qu'un clic dans le planning ne sélectionne pas le bâtiment situé derrière. Réduire la fenêtre pour vérifier le défilement.

La compilation ne valide pas à elle seule le rendu ni les interactions. La prochaine étape sera une première réservation, à partir d'une case choisie.
