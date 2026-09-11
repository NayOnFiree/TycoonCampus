# Jalon 2.1 — Horloge et pause

Une petite barre en haut à gauche affiche jour, heure, semestre, semaine et état de pause. La partie commence le lundi à 08:00, en pause. **Espace** alterne pause et vitesse normale. Il s'agit d'une pause de simulation : déplacement, rotation, zoom et sélection restent disponibles.

Le temps est détenu par `CampusClock`, séparément de la caméra et de l'affichage. `CampusHUD` ne fait que le lire. À la demande du joueur, la base est désormais quatre fois plus rapide que la proposition initiale : 288 secondes simulées par seconde réelle en x1, soit une journée complète en 5 minutes réelles. Le calendrier compte sept jours par semaine et quatre semaines par semestre, sans encore simuler cours, vacances ou examens.

L'affichage est provisoire, sans interaction souris. La programmation d'événements arrive ensuite. La sauvegarde n'est pas encore créée : relancer Play remet le temps à lundi 08:00.

Essai : laisser en pause quelques secondes ; appuyer sur Espace et observer l'heure avancer ; appuyer de nouveau et vérifier l'arrêt. Vérifier caméra et sélection dans les deux états. Une minute de jeu dure environ 0,208 seconde réelle en x1. La compilation ne remplace pas cet essai en jeu.

## Jalon 2.2 — Accélération

Touches 1, 2 et 3 : x1, x2 et x4, sur la rangée supérieure ou le pavé numérique. Les touches AZERTY sans Maj (&, é, ") sont aussi affectées explicitement. Choisir une vitesse démarre ou reprend la simulation. Espace met en pause puis reprend à la dernière vitesse choisie. La vitesse affichée entre parenthèses en pause est la vitesse de reprise.

Seul le temps simulé est accéléré. La caméra et la sélection restent à vitesse normale. Un jour complet dure 5 minutes en x1, 2 min 30 en x2 et 1 min 15 en x4. Un intervalle réel de 10 secondes doit avancer respectivement de 48, 96 ou 192 minutes simulées. En pause, aucune minute ne doit avancer, puis une reprise à x4 doit conserver x4. Le démarrage reste en pause à 08:00 ; le nouveau rythme de base ne change pas ce choix.

## Jalon 2.3 — Événements du calendrier

Chaque heure franchie déclenche `OnHourStarted`. À minuit suivent `OnDayStarted`, puis `OnWeekStarted` le lundi et `OnSemesterStarted` tous les 28 jours, dans cet ordre. Les indices transmis sont absolus et commencent à zéro, contrairement aux numéros affichés au joueur qui commencent à un.

Si une seule mise à jour franchit plusieurs heures, chacune est traitée une fois dans l'ordre chronologique. Pendant les notifications, l'horloge indique l'heure exacte de la frontière traitée ; elle rejoint ensuite l'heure finale de la mise à jour. Pause et changement de vitesse demandés par un abonné affectent la prochaine mise à jour. Une avance récursive est ignorée. Au démarrage, l'horloge est déjà à 08:00 : aucun événement de franchissement de 08:00 n'est inventé. Le futur planning devra aussi évaluer les réservations actives lors de son initialisation.

Les événements constituent le support du futur planning ; aucun cours n'est encore simulé et aucun faux événement d'activité n'est ajouté à l'interface. Le HUD affiche déjà les passages de jour, semaine et semestre.

## Vérification automatisée

`Tests/CampusTimeModelTests.cpp` teste directement le modèle C++ utilisé par l'acteur Unreal. Cas couverts : durées x1/x2/x4, pause/reprise, plusieurs heures franchies dans une mise à jour, minuit, quatre semaines et semestre, valeurs invalides, changements de vitesse et cohérence entre une grande avance et mille petites avances.

Un écart d'arrondi observé par le test de fréquence d'images a été corrigé par sommation compensée. Tous ces tests passent avec MSVC. L'intégration et l'essai dans Unreal restent distingués de ces tests du modèle.
