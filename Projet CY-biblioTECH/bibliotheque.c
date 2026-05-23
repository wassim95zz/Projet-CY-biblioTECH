#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
 
/* ── Constantes ────────────────────────────────────────────────────────── */
#define MAX_TITRE       100
#define MAX_AUTEUR      100
#define MAX_CATEGORIE    50
#define MAX_LOGIN        50
#define MAX_MDP          50
#define MAX_LIVRES      100
#define MAX_USERS        50
#define MAX_EMPRUNTS    300
 
#define MAX_EMP_ETU       3
#define MAX_EMP_PROF      5
#define DUREE_ETU         2   /* minutes */
#define DUREE_PROF        3
 
#define FICHIER_LIVRES      "livres.txt"
#define FICHIER_USERS       "utilisateurs.txt"
#define FICHIER_EMPRUNTS    "emprunts.txt"
 
/* ── Structures ────────────────────────────────────────────────────────── */
typedef struct {
    int  id;
    char titre[MAX_TITRE];
    char auteur[MAX_AUTEUR];
    char categorie[MAX_CATEGORIE];
    int  disponible;        /* 1 = disponible, 0 = emprunte */
    int  emprunte_par_id;
} Livre;
 
typedef struct {
    int  id;
    char login[MAX_LOGIN];
    char mdp[MAX_MDP];
    int  role;              /* 0 = etudiant, 1 = professeur */
    int  livres_emp[MAX_EMP_PROF];
    int  heure_retour[MAX_EMP_PROF];
    int  min_retour[MAX_EMP_PROF];
    int  nb_emp;
} Utilisateur;
 
typedef struct {
    int id_user;
    int id_livre;
    int h_emp, m_emp;
    int h_ret, m_ret;
    int actif;
} Emprunt;
 
/* ── Utilitaires ───────────────────────────────────────────────────────── */
void lire_chaine(char *buf, int max) {
    fgets(buf, max, stdin);
    int n = strlen(buf);
    if (n > 0 && buf[n-1] == '\n') buf[n-1] = '\0';
}
 
void heure_actuelle(int *h, int *m) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    *h = tm->tm_hour;
    *m = tm->tm_min;
}
 
void afficher_heure(int h, int m) { printf("%02d:%02d", h, m); }
 
/* Met la chaine en minuscules dans buf_out (buf_out doit etre assez grand) */
void to_lower(const char *src, char *dst, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = tolower((unsigned char)src[i]);
    dst[i] = '\0';
}
 
int max_emp(int role)   { return role ? MAX_EMP_PROF : MAX_EMP_ETU; }
int duree_emp(int role) { return role ? DUREE_PROF   : DUREE_ETU;   }
 
/* ── Livres ────────────────────────────────────────────────────────────── */
int cmp_titre(const void *a, const void *b) {
    return strcmp(((Livre*)a)->titre, ((Livre*)b)->titre);
}
 
int trouver_livre(Livre *bib, int nb, int id) {
    for (int i = 0; i < nb; i++)
        if (bib[i].id == id) return i;
    return -1;
}
 
void afficher_livre(Livre *l) {
    printf("  [%d] %s - %s (%s) %s\n",
           l->id, l->titre, l->auteur, l->categorie,
           l->disponible ? "[DISPO]" : "[EMPRUNTE]");
}
 
/* Recherche generique : champ = 't' titre, 'a' auteur, 'c' categorie */
void rechercher(Livre *bib, int nb, char champ) {
    char req[MAX_TITRE], req_low[MAX_TITRE], champ_low[MAX_TITRE];
    int trouve = 0;
 
    const char *labels[] = {"titre", "auteur", "categorie"};
    int label_idx = (champ == 't') ? 0 : (champ == 'a') ? 1 : 2;
    printf("Recherche par %s : ", labels[label_idx]);
    lire_chaine(req, MAX_TITRE);
    to_lower(req, req_low, MAX_TITRE);
 
    for (int i = 0; i < nb; i++) {
        char *src = (champ == 't') ? bib[i].titre
                  : (champ == 'a') ? bib[i].auteur
                  :                  bib[i].categorie;
        to_lower(src, champ_low, MAX_TITRE);
        if (strstr(champ_low, req_low)) { afficher_livre(&bib[i]); trouve = 1; }
    }
    if (!trouve) printf("Aucun resultat.\n");
}
 
void afficher_disponibles(Livre *bib, int nb) {
    Livre dispo[MAX_LIVRES];
    int nd = 0;
    for (int i = 0; i < nb; i++)
        if (bib[i].disponible) dispo[nd++] = bib[i];
    if (!nd) { printf("Aucun livre disponible.\n"); return; }
    qsort(dispo, nd, sizeof(Livre), cmp_titre);
    printf("\n--- Livres disponibles (%d) ---\n", nd);
    for (int i = 0; i < nd; i++) afficher_livre(&dispo[i]);
}
 
void ajouter_livre(Livre *bib, int *nb) {
    if (*nb >= MAX_LIVRES) { printf("Bibliotheque pleine !\n"); return; }
    Livre *l = &bib[*nb];
    l->id = *nb + 1;
    l->disponible = 1;
    l->emprunte_par_id = -1;
    printf("Titre    : "); lire_chaine(l->titre,     MAX_TITRE);
    printf("Auteur   : "); lire_chaine(l->auteur,    MAX_AUTEUR);
    printf("Categorie: "); lire_chaine(l->categorie, MAX_CATEGORIE);
    (*nb)++;
    printf("Livre ajoute (ID %d).\n", l->id);
}
 
/* ── Fichiers Livres ───────────────────────────────────────────────────── */
void charger_livres(Livre *bib, int *nb) {
    FILE *f = fopen(FICHIER_LIVRES, "r");
    *nb = 0;
    if (!f) { printf("Fichier livres introuvable. Bibliotheque vide.\n"); return; }
    while (*nb < MAX_LIVRES &&
           fscanf(f, "%d|%99[^|]|%99[^|]|%49[^\n]\n",
                  &bib[*nb].id, bib[*nb].titre,
                  bib[*nb].auteur, bib[*nb].categorie) == 4) {
        bib[*nb].disponible = 1;
        bib[*nb].emprunte_par_id = -1;
        (*nb)++;
    }
    fclose(f);
    printf("%d livre(s) charge(s).\n", *nb);
}
 
void sauvegarder_livres(Livre *bib, int nb) {
    FILE *f = fopen(FICHIER_LIVRES, "w");
    if (!f) { printf("ERREUR: sauvegarde livres impossible.\n"); return; }
    for (int i = 0; i < nb; i++)
        fprintf(f, "%d|%s|%s|%s\n", bib[i].id, bib[i].titre, bib[i].auteur, bib[i].categorie);
    fclose(f);
}
 
/* ── Fichiers Utilisateurs ─────────────────────────────────────────────── */
void charger_utilisateurs(Utilisateur *users, int *nb) {
    FILE *f = fopen(FICHIER_USERS, "r");
    *nb = 0;
    if (!f) { printf("Fichier utilisateurs introuvable.\n"); return; }
    while (*nb < MAX_USERS &&
           fscanf(f, "%d|%49[^|]|%49[^|]|%d\n",
                  &users[*nb].id, users[*nb].login,
                  users[*nb].mdp, &users[*nb].role) == 4) {
        users[*nb].nb_emp = 0;
        for (int j = 0; j < MAX_EMP_PROF; j++) users[*nb].livres_emp[j] = -1;
        (*nb)++;
    }
    fclose(f);
    printf("%d utilisateur(s) charge(s).\n", *nb);
}
 
void sauvegarder_utilisateurs(Utilisateur *users, int nb) {
    FILE *f = fopen(FICHIER_USERS, "w");
    if (!f) { printf("ERREUR: sauvegarde utilisateurs impossible.\n"); return; }
    for (int i = 0; i < nb; i++)
        fprintf(f, "%d|%s|%s|%d\n", users[i].id, users[i].login, users[i].mdp, users[i].role);
    fclose(f);
}
 
/* ── Fichiers Emprunts ─────────────────────────────────────────────────── */
void charger_emprunts(Emprunt *emp, int *nb, Utilisateur *users, int nb_users,
                      Livre *bib, int nb_livres) {
    FILE *f = fopen(FICHIER_EMPRUNTS, "r");
    *nb = 0;
    if (!f) return;
    while (*nb < MAX_EMPRUNTS &&
           fscanf(f, "%d|%d|%d|%d|%d|%d|%d\n",
                  &emp[*nb].id_user, &emp[*nb].id_livre,
                  &emp[*nb].h_emp,   &emp[*nb].m_emp,
                  &emp[*nb].h_ret,   &emp[*nb].m_ret,
                  &emp[*nb].actif) == 7) {
        if (emp[*nb].actif) {
            /* Restaurer etat livre */
            int li = trouver_livre(bib, nb_livres, emp[*nb].id_livre);
            if (li != -1) { bib[li].disponible = 0; bib[li].emprunte_par_id = emp[*nb].id_user; }
            /* Restaurer etat utilisateur */
            for (int u = 0; u < nb_users; u++) {
                if (users[u].id == emp[*nb].id_user &&
                    users[u].nb_emp < MAX_EMP_PROF) {
                    int k = users[u].nb_emp;
                    users[u].livres_emp[k]  = emp[*nb].id_livre;
                    users[u].heure_retour[k] = emp[*nb].h_ret;
                    users[u].min_retour[k]   = emp[*nb].m_ret;
                    users[u].nb_emp++;
                }
            }
        }
        (*nb)++;
    }
    fclose(f);
}
 
void sauvegarder_emprunts(Emprunt *emp, int nb) {
    FILE *f = fopen(FICHIER_EMPRUNTS, "w");
    if (!f) { printf("ERREUR: sauvegarde emprunts impossible.\n"); return; }
    for (int i = 0; i < nb; i++)
        fprintf(f, "%d|%d|%d|%d|%d|%d|%d\n",
                emp[i].id_user, emp[i].id_livre,
                emp[i].h_emp,   emp[i].m_emp,
                emp[i].h_ret,   emp[i].m_ret,
                emp[i].actif);
    fclose(f);
}
 
/* ── Utilisateurs ──────────────────────────────────────────────────────── */
int utilisateur_existe(Utilisateur *users, int nb, const char *login) {
    for (int i = 0; i < nb; i++)
        if (strcmp(users[i].login, login) == 0) return 1;
    return 0;
}
 
Utilisateur *connexion(Utilisateur *users, int nb) {
    char login[MAX_LOGIN], mdp[MAX_MDP];
    printf("\n--- Connexion ---\n");
    printf("Login : "); lire_chaine(login, MAX_LOGIN);
    printf("MDP   : "); lire_chaine(mdp,   MAX_MDP);
    for (int i = 0; i < nb; i++)
        if (!strcmp(users[i].login, login) && !strcmp(users[i].mdp, mdp)) {
            printf("Bienvenue %s !\n", users[i].login);
            return &users[i];
        }
    printf("Login ou mot de passe incorrect.\n");
    return NULL;
}
 
void creer_compte(Utilisateur *users, int *nb) {
    if (*nb >= MAX_USERS) { printf("Nombre max d'utilisateurs atteint.\n"); return; }
    Utilisateur *u = &users[*nb];
    printf("\n--- Creation de compte ---\n");
    do {
        printf("Login : "); lire_chaine(u->login, MAX_LOGIN);
        if (utilisateur_existe(users, *nb, u->login))
            printf("Login deja pris, choisissez-en un autre.\n");
    } while (utilisateur_existe(users, *nb, u->login));
    printf("MDP   : "); lire_chaine(u->mdp, MAX_MDP);
    printf("Role (0=Etudiant, 1=Professeur) : ");
    scanf("%d", &u->role); while (getchar() != '\n');
    if (u->role != 0 && u->role != 1) { u->role = 0; printf("Role invalide, Etudiant par defaut.\n"); }
    u->id = *nb + 1;
    u->nb_emp = 0;
    for (int i = 0; i < MAX_EMP_PROF; i++) u->livres_emp[i] = -1;
    (*nb)++;
    printf("Compte cree (ID %d).\n", u->id);
}
 
/* ── Emprunts / Regles ─────────────────────────────────────────────────── */
int a_des_retards(Utilisateur *u, int h, int m) {
    int now = h * 60 + m;
    for (int i = 0; i < u->nb_emp; i++)
        if (now > u->heure_retour[i] * 60 + u->min_retour[i]) return 1;
    return 0;
}
 
void afficher_mes_emprunts(Utilisateur *u, Livre *bib, int nb, int h, int m) {
    if (!u->nb_emp) { printf("Aucun emprunt en cours.\n"); return; }
    int now = h * 60 + m;
    printf("\n--- Mes emprunts ---\n");
    for (int i = 0; i < u->nb_emp; i++) {
        int li = trouver_livre(bib, nb, u->livres_emp[i]);
        if (li != -1) {
            printf("  %s | retour avant ", bib[li].titre);
            afficher_heure(u->heure_retour[i], u->min_retour[i]);
            if (now > u->heure_retour[i] * 60 + u->min_retour[i])
                printf(" [EN RETARD !]");
            printf("\n");
        }
    }
}
 
void emprunter_livre(Utilisateur *u, Livre *bib, int nb,
                     int h, int m, Emprunt *emp, int *nb_emp) {
    if (u->nb_emp >= max_emp(u->role)) {
        printf("Quota atteint (%d/%d livres).\n", u->nb_emp, max_emp(u->role));
        return;
    }
    if (a_des_retards(u, h, m)) {
        printf("Retard detecte : rendez tous vos livres en retard avant d'emprunter.\n");
        return;
    }
 
    int choix;
    printf("\n1.Titre  2.Auteur  3.Categorie  4.Tous disponibles\nChoix : ");
    scanf("%d", &choix); while (getchar() != '\n');
    if (choix == 1)      rechercher(bib, nb, 't');
    else if (choix == 2) rechercher(bib, nb, 'a');
    else if (choix == 3) rechercher(bib, nb, 'c');
    else                 afficher_disponibles(bib, nb);
 
    printf("ID du livre a emprunter : ");
    int id; scanf("%d", &id); while (getchar() != '\n');
    int li = trouver_livre(bib, nb, id);
    if (li == -1)                  { printf("Livre introuvable.\n");  return; }
    if (!bib[li].disponible)       { printf("Livre deja emprunte.\n"); return; }
 
    /* Calcul heure de retour */
    int m_ret = m + duree_emp(u->role);
    int h_ret = h + m_ret / 60;
    m_ret %= 60;
    h_ret %= 24;
 
    /* Mise a jour livre */
    bib[li].disponible = 0;
    bib[li].emprunte_par_id = u->id;
 
    /* Mise a jour utilisateur */
    int k = u->nb_emp;
    u->livres_emp[k]   = id;
    u->heure_retour[k] = h_ret;
    u->min_retour[k]   = m_ret;
    u->nb_emp++;
 
    /* Enregistrement emprunt */
    if (*nb_emp < MAX_EMPRUNTS) {
        emp[*nb_emp] = (Emprunt){u->id, id, h, m, h_ret, m_ret, 1};
        (*nb_emp)++;
    }
 
    printf("Emprunt OK ! Retour avant "); afficher_heure(h_ret, m_ret); printf("\n");
}
 
void rendre_livre(Utilisateur *u, Livre *bib, int nb_bib,
                  Emprunt *emp, int nb_emp) {
    if (!u->nb_emp) { printf("Aucun livre a rendre.\n"); return; }
    afficher_mes_emprunts(u, bib, nb_bib, 0, 0);
 
    printf("ID du livre a rendre : ");
    int id; scanf("%d", &id); while (getchar() != '\n');
 
    /* Chercher dans les emprunts de l'utilisateur */
    int pos = -1;
    for (int i = 0; i < u->nb_emp; i++)
        if (u->livres_emp[i] == id) { pos = i; break; }
    if (pos == -1) { printf("Vous n'avez pas emprunte ce livre.\n"); return; }
 
    /* Decaler le tableau utilisateur */
    for (int j = pos; j < u->nb_emp - 1; j++) {
        u->livres_emp[j]    = u->livres_emp[j+1];
        u->heure_retour[j]  = u->heure_retour[j+1];
        u->min_retour[j]    = u->min_retour[j+1];
    }
    u->nb_emp--;
 
    /* Marquer livre disponible */
    int li = trouver_livre(bib, nb_bib, id);
    if (li != -1) { bib[li].disponible = 1; bib[li].emprunte_par_id = -1; }
 
    /* Marquer emprunt inactif */
    for (int i = 0; i < nb_emp; i++)
        if (emp[i].id_user == u->id && emp[i].id_livre == id && emp[i].actif)
            { emp[i].actif = 0; break; }
 
    printf("Livre rendu avec succes.\n");
}
 
/* ── Interface ─────────────────────────────────────────────────────────── */
void banniere(void) {
    printf("\n============================================================\n");
    printf("       BIBLIOTHEQUE UNIVERSITAIRE - CY TECH PreING1\n");
    printf("============================================================\n");
}
 
/* ── Main ──────────────────────────────────────────────────────────────── */
int main(void) {
    Livre       bib[MAX_LIVRES];
    Utilisateur users[MAX_USERS];
    Emprunt     emp[MAX_EMPRUNTS];
    int nb_bib = 0, nb_users = 0, nb_emp = 0;
 
    banniere();
    charger_livres(bib, &nb_bib);
    charger_utilisateurs(users, &nb_users);
    charger_emprunts(emp, &nb_emp, users, nb_users, bib, nb_bib);
 
    int running = 1;
    while (running) {
        printf("\n1.Se connecter  2.Creer un compte  3.Quitter\nChoix : ");
        int c; scanf("%d", &c); while (getchar() != '\n');
 
        if (c == 3) { running = 0; break; }
 
        if (c == 2) {
            creer_compte(users, &nb_users);
            sauvegarder_utilisateurs(users, nb_users);
            continue;
        }
 
        if (c != 1) { printf("Choix invalide.\n"); continue; }
 
        Utilisateur *u = connexion(users, nb_users);
        if (!u) continue;
 
        int h, m;
        heure_actuelle(&h, &m);
        afficher_mes_emprunts(u, bib, nb_bib, h, m);
 
        int connecte = 1;
        while (connecte) {
            printf("\n[%s | %s | %d/%d livres]\n",
                   u->login,
                   u->role ? "PROFESSEUR" : "ETUDIANT",
                   u->nb_emp, max_emp(u->role));
 
            if (u->role == 1)
                printf("1.Emprunter  2.Rendre  3.Mes emprunts  4.Ajouter livre  5.Deconnexion\n");
            else
                printf("1.Emprunter  2.Rendre  3.Mes emprunts  4.Deconnexion\n");
 
            printf("Choix : ");
            int ch; scanf("%d", &ch); while (getchar() != '\n');
            heure_actuelle(&h, &m);
 
            int deconn = u->role ? (ch == 5) : (ch == 4);
            if (deconn) { connecte = 0; break; }
 
            switch (ch) {
                case 1: emprunter_livre(u, bib, nb_bib, h, m, emp, &nb_emp); break;
                case 2: rendre_livre(u, bib, nb_bib, emp, nb_emp);            break;
                case 3: afficher_mes_emprunts(u, bib, nb_bib, h, m);          break;
                case 4:
                    if (u->role == 1) ajouter_livre(bib, &nb_bib);
                    else connecte = 0;
                    break;
                default: printf("Choix invalide.\n");
            }
 
            sauvegarder_livres(bib, nb_bib);
            sauvegarder_emprunts(emp, nb_emp);
        }
    }
 
    /* Sauvegarde finale */
    sauvegarder_livres(bib, nb_bib);
    sauvegarder_utilisateurs(users, nb_users);
    sauvegarder_emprunts(emp, nb_emp);
 
    printf("Au revoir !\n");
    return 0;
}
 