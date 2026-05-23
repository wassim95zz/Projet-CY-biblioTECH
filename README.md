# Projet-CY-biblioTECH
# CY-biblioTECH

Projet de gestion de bibliothèque universitaire - PreING1 CY Tech 2025-2026

## Installation et lancement

* Télécharger le depot Git du projet.
* Ouvrir l'invite de commande et entrer la commande :
  wsl --install
* Une fois la machine redémarré, lancer WSL, puis entrer dans le dossier du projet avec la commande :
  cd /nom_dossier (attention, si il y a un espace dans le nom, utiliser \ suivi d'un espace)
* Compiler le projet avec la commande suivante :
  gcc -o bibliotheque bibliotheque.c
* Lancer le programme :
  ./bibliotheque
* Bonne utilisation !

## Fichiers nécessaires

Placer ces fichiers dans le même dossier que l'exécutable :

- bibliotheque.c   : le code source
- livres.txt       : liste des livres 
- utilisateurs.txt : liste des utilisateurs 
- emprunts.txt     : historique des emprunts (cree automatiquement)

## Format des fichiers

### livres.txt
id|titre|auteur|categorie
Exemple :
1|L'Odyssee|Homere|Litterature

### utilisateurs.txt
id|login|mot_de_passe|role
role : 0 = etudiant, 1 = professeur
Exemple :
1|alice|alice123|0
2|prof_martin|martin789|1

## Regles de la bibliotheque

- Etudiant  : 3 livres maximum, duree 2 minutes
- Professeur: 5 livres maximum, duree 3 minutes
- En cas de retard : impossible d'emprunter tant que tous les livres en retard ne sont pas rendus
