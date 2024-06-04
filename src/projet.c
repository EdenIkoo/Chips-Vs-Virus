#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <MLV/MLV_all.h>

/*Chips = Les tourelles*/
typedef struct chips{
    int type; /*A/R/P/X/F*/
    int life; /*Points de vie*/
    int line; /*Ligne sur laquelle elle est placée*/
    int position; /*Position horizontale*/
    int price; /*Le prix qu'elle coûte*/
    struct chips* next; /*Adresse de la prochaine tourelle*/ /*SUIVANT*/
}Chips;

/*Virus = Les méchants*/
typedef struct virus{
    int type; /*E/V/S/H/B*/
    int life; /*Points de vie*/
    int line; /*Ligne sur laquelle elle est placée*/
    int position; /*Position horizontale*/
    int speed; /*Vitesse à laquelle il avance*/
    int turn; /*A quel tour il apparait*/
    struct virus* next; /*adresse du prochain méchant*/
    struct virus* next_line; /*adresse de la prochaine ligne*/
    struct virus* prev_line; /*adresse de la précédente ligne*/
}Virus;

typedef struct game{
    Virus* virus; /*liste chainée des méchants*/
    Chips* chips; /*liste chainée des tourelles*/
    int turn; /*numéro du tour*/
    int money; /*montant d'argent*/
}Game;

/*[INDICATIF] Affichage des listes*/

void AfficheListeVirus(Virus* v){
    /*Affiche la liste des virus*/
    if(v == NULL)
        return;
    for(; v!= NULL; v = v->next)
        printf("%d %d %c %p\n", v->line, v->turn, v->type, v->prev_line);
}

void AfficheListeChips(Chips* c){
    /*Affiche la liste des chips*/
    if(c == NULL)
        return;
    for(; c!= NULL; c = c->next)
        printf("%d %d %d %d %c\n", c->line, c->position, c->price, c->life, c->type);
}

void AfficheLigneVirus(Virus * v){
    /*Affiche la liste des virus sur une ligne*/
    for (; v != NULL; v = v->next_line){
        printf("%d %d %c %p\n", v->turn, v->line, v->type, v->prev_line);
    }
}

void ListeVirusParLigne(Virus * v){
    /*Affiche la liste des virus dans l'ordre des lignes*/
    int i;
    for (i = 0; i<7; i++, v = v->next){
        AfficheLigneVirus(v);
    }
}

/*Détection des arguments*/

int verif_arguments(int argc, char *argv[]){

	if(argc>2){printf("too many arguments were given\n"); exit(1);}
	else if(argc<2){printf("too less arguments were given\n"); exit(1);}

    if (argv[1][0] == '-'){ 
        if (argv[1][1] == 'a')
            return 1;
        else if (argv[1][1] == 'g')
            return 2;
        else {
            printf("invalid argument given\n");
            exit(1);
        }
    }
    else {
        printf("invalid argument given\n");
        exit(1);
    }
    return 0;
}


/*Recherche de maximums dans la liste virus*/

int max_line(Virus* v){
    /*Détermine le nombre de ligne du jeu*/
    int max = 0;
    for(; v != NULL; v = v->next){
        if(v->line > max) max = v->line;
    }
    return max;
}

int max_turn(Virus* v){
    /*Détermine le tour auquel apparait le dernier virus*/
    int max = 0;
    for(; v != NULL; v = v->next){
        if(v->turn > max) max = v->turn;
    }
    return max;
}


/*Spécifités des types*/

/*Virus*/

void spe_virus(char type_virus, Virus* v){
    /*Attribue vie et vitesse à un nouveau virus en fonction de son type*/
    if (type_virus == 'E'){ /*Virus de base*/
        v->life = 4;
        v->speed = 2;
    }
    else if (type_virus == 'V'){ /*Plus rapide, plus fragile*/
        v->life = 3;
        v->speed = 3;
    }
    else if (type_virus == 'S'){ /*Plus lent, plus solide*/
        v->life = 6;
        v->speed = 1;
    }
    else if (type_virus == 'H'){ /*Healer*/
        v->life = 2;
        v->speed = 2;
    }
    else if (type_virus == 'B'){ /*Booster*/
        v->life = 2;
        v->speed = 2;
    }
}

int capacite_virus(char type_virus){
    /*Détection de la capacité du virus en fonction de son type */
    if (type_virus == 'E' || type_virus == 'V') /*Base et rapide; tapent à 1*/
        return 1;
    else if (type_virus == 'S') /*Lent; tape à 2*/
        return 2;
    else if (type_virus == 'H') /*Healer; soigne et tape à 1*/
        return 0;
    else if (type_virus == 'B') /*Booster; accélère et tape à 1*/
        return -1;
    return 0;
}

/*Chips*/

void spe_chips(char type_chips, Chips* c){
    /*Attribue vie et prix à un nouveau chips en fonction de son type*/
    if (type_chips == 'A'){ /*Tourelle de base*/
        c->life = 3;
        c->price = 100;
    }
    else if (type_chips == 'R'){ /*Ralentisseur*/
        c->life = 2;
        c->price = 75;
    }
    else if (type_chips == 'M'){ /*Mine*/
        c->life = 1;
        c->price = 50;
    }
    else if (type_chips == 'X'){ /*Gros canon*/
        c->life = 3;
        c->price = 500;
    }
    else if (type_chips == 'W'){ /*Mur*/
        c->life = 6;
        c->price = 150;
    }
}

int capacite_chips(char type_chips){
    /*Détection de quel chips agit*/
    if (type_chips == 'A'){
        return 1; /*Tourelle; tape à 1*/
    }
    else if (type_chips == 'R'){
        return -1; /*Ralentisseur; met vitesse à 1*/
    }
    else if (type_chips == 'M'){
        return 2; /*Mine; tue*/
    }
    else if (type_chips == 'X'){
        return 3; /*Gros canon; tape à 3 sur 3 lignes*/
    }
    else if (type_chips == 'W'){
        return 0; /*Mur; beaucoup de vie*/
    }
    return 0;
}


/*Insertion dans les liste chaînées*/

void insertion_virus(Virus* v, Virus* creation_virus){
    /*Ajoute un virus à la liste chaînée du jeu*/
    if(v != NULL){
        for(; v->next != NULL; v = v->next){}/*Tant qu'on est pas au bout de la liste*/
        v->next = creation_virus; /*Prend l'adresse du nouveau virus*/
    }
    else {
        v = creation_virus; /*Si liste vide*/
    }
}

void insertion_chips(Chips* chips, Chips* creation_chips){
    /*Ajoute un chips à la liste chaînée du jeu*/
    if(chips != NULL){
        for(;chips->next != NULL;)/*Tant qu'on est pas au bout de la liste*/
            chips = chips->next;
        chips->next = creation_chips; /*Prend l'adresse du nouveau chips*/
    }
    else {
        chips = creation_chips; /*Si liste vide*/
    }
}


/*Allocation de mémoire*/

Virus * alloue_cellule_virus(int num_tour, int num_ligne, char type_virus){
    /*Allocation de mémoire pour la création d'un virus*/
    Virus * tmp = (Virus *)malloc(sizeof(Virus)); /*Initialisation d'un virus*/
    if(tmp != NULL){
        /*Si l'allocation a réussie*/
        /*Initialisation de chaque attributs*/
        tmp->type = type_virus;
        spe_virus(type_virus,tmp);
        tmp->line = num_ligne;
        tmp->position = num_tour; /*Change quand il avance*/
        tmp->turn = num_tour;
        tmp->next = NULL;
        tmp->next_line = NULL;
        tmp->prev_line = NULL;
    }
    return tmp;
}

Chips * alloue_cellule_chips(char type_chips, int num_ligne, int num_colonne){
    /*Allocation de mémoire pour la création d'un chips*/
    Chips * tmp = (Chips *) malloc(sizeof(Chips)); /*Initialisation d'un chips*/
    if(tmp != NULL){
        /*Si l'allocation a réussie*/
        /*Initialisation de chaque attributs*/
        tmp->type = type_chips;
        spe_chips(type_chips, tmp);
        tmp->line = num_ligne;
        tmp->position = num_colonne;
        tmp->next = NULL;
    }
    return tmp;
}


/*Recherche tourelle*/

int recherche(Chips* c, int nb_ligne, int nb_colonne){
    /*Recherche la présence d'un chips à la position nb_ligne et nb_colonne*/
    for(; c != NULL; c = c->next){
        if(c->line == nb_ligne && c->position == nb_colonne){
            return 1;
        }
    }
    return 0; /*Pas de tourelle à cet emplacement*/
}


/*Création des listes*/

void init_liste_virus(FILE* fichier, Game *jeu, Virus* virus){
    /*Crétion de la liste des virus par lecture du fichier de niveau*/
    int num_tour, num_ligne;
    char type_virus;

    while((fscanf(fichier,"%d %d %c",&num_tour, &num_ligne, &type_virus)) != EOF){
        Virus * nouv_virus = alloue_cellule_virus(num_tour, num_ligne, type_virus); /*Créé le virus*/

        if (nouv_virus == NULL) {
            printf("Erreur d'allocation !\n");
            exit(1);
        }
        insertion_virus(virus, nouv_virus);/*Fonction d'insertion*/
    }
    jeu->virus = virus->next; /*L'ajoute à la liste chainée du jeu*/
}

void init_position_virus(Virus* v, int nb_colonnes){
    /*Attribution de la position maximale à tous les virus*/
    for(; v != NULL; v = v->next){
        v->position = nb_colonnes;
    }
}

void Pose_tourelles(Game *jeu, Chips* chips, int nb_lignes, int nb_colonnes, int *score){
    /*Création de la liste des chips par saisie*/
    char type_chips;
    int num_l = 0, num_c = 0;
    do{
        do{
            printf("Quelle tour déployer ?\n");
            printf("Vous avez %d pièces\n", jeu->money);
            printf("A : Tourelle (100)\nR : Ralentisseur (75)\nM : Mine (50)\nX : Gros canon (500)\nW : Mur(150)\nq : no more\nVotre choix ? : ");
            scanf(" %c", &type_chips);
        } while(type_chips!= 'A' && type_chips!= 'R' && type_chips!= 'M' && type_chips!= 'X' && type_chips!= 'W' && type_chips!= 'q');
        
        if(type_chips == 'q')
            continue;

        do {
            printf("Où la placez-vous ? (n°ligne n°colonne) : ");
            scanf("%d %d", &num_l, &num_c);
        } while(num_l < 1 || num_l > nb_lignes || num_c < 1 || num_c > nb_colonnes - 1 || (recherche(chips->next, num_l, num_c) == 1));
        
        Chips * nouv_chips = alloue_cellule_chips(type_chips, num_l, num_c);

        if(jeu->money - nouv_chips->price < 0){ /*Pas assez d'argent*/
            printf("Pas assez d'argent !\n");
            printf("Il vous reste %d pièces\n", jeu->money);
            continue;
        }

        if (nouv_chips == NULL) {
            printf("Erreur d'allocation !\n");
            exit(1);
        }

        insertion_chips(chips, nouv_chips);
        jeu->money -= nouv_chips->price; /*Retrait du prix de la tour*/
        *score -= nouv_chips->price/10;

        printf("\n");

    } while(type_chips != 'q');
    jeu->chips = chips->next; /*Ajout à la liste des virus du jeu*/
}
 
char selec_type_chips(int x, int y){
    if(x>=30 && x<=80 && y>=35 && y<=75)
        return 'A';
    else if(x>=100 && x<=150 && y>=35 && y<=68)
        return 'M';
    else if(x>=170 && x<=201 && y>=15 && y<=75)
        return 'R';
    else if(x>=230 && x<=268 && y>=15 && y<=75)
        return 'W';
    else if(x>=300 && x<=346 && y>=20 && y<=80)
        return 'X';
    return 'O';
}

void Pose_tourelles_graph(Game *jeu, Chips* chips, char type_chips, int num_colonne, int num_ligne, int nb_lignes, int nb_colonnes, int *score){

    int hauteur_case = 600/nb_lignes;
    int largeur_case = 1050/nb_colonnes;
	num_ligne = round((num_ligne-110)/(float)hauteur_case);
	num_colonne = round((num_colonne-110)/(float)largeur_case);

    Chips * nouv_chips = alloue_cellule_chips(type_chips, num_ligne, num_colonne);

    if(jeu->money - nouv_chips->price < 0){ /*Pas assez d'argent*/
            MLV_draw_text(50, 120, "Pas assez d'argent !", MLV_COLOR_WHITE);
            return;
        }

    if (recherche(chips->next, num_ligne, num_colonne) == 1)
        return;

    if (nouv_chips == NULL) {
        printf("Erreur d'allocation !\n");
        exit(1);
    }

    insertion_chips(chips, nouv_chips);
    jeu->money -= nouv_chips->price; /*Retrait du prix de la tour*/
    *score -= nouv_chips->price/10;
    jeu->chips = chips->next; /*Ajout à la liste des virus du jeu*/
}


/*Chainage next_line et prev_line virus*/

void chainage_ligne(Virus* v, int nb_lignes){
    /*Chaïnage next_line et prev_line des virus sur une ligne*/
    Virus* tab[nb_lignes]; /*Tableau vide de taille nb_lignes*/
    memset(tab, 0, nb_lignes*sizeof(Virus*)); /*Rempli tableau de 0*/
    for(; v != NULL; v = v->next){ /*Parcours de la liste virus*/
        if (tab[(v->line)-1] != 0){ /*Si pas premier virus de sa ligne*/
            tab[(v->line)-1]->next_line = v; /*Virus courant devient next_line de celui du tableau*/
        }
        v->prev_line = tab[(v->line)-1]; /*prev_line virus courant devient virus du tableau*/
        tab[(v->line)-1] = v; /*virus courant devient virus du tableau*/
    }
}


/*[Graphique] Affichage du plateau*/

void affiche_plateau_graph(int nb_lignes, int nb_colonnes){
    /*Affichage du plateau de jeu en mode graphique*/
    int hauteur_case = 600/nb_lignes;
    int largeur_case = 1050/nb_colonnes;
    int l, c;

    MLV_draw_filled_rectangle(0, 0, 1200, 800, MLV_COLOR_GRAY10);
    MLV_draw_rectangle(610, 10, 200, 100, MLV_COLOR_WHITE);
    MLV_draw_text(665, 50, "Voir la vague", MLV_COLOR_WHITE);
    MLV_draw_rectangle(830, 10, 200, 100, MLV_COLOR_WHITE);
    MLV_draw_text(885, 50, "Commencer", MLV_COLOR_WHITE);
    MLV_draw_rectangle(1050, 10, 140, 100, MLV_COLOR_WHITE);
    MLV_draw_text(1095, 50, "Quitter", MLV_COLOR_WHITE);

    for(l = 0; l<nb_lignes; l++){
        for(c = 0; c < nb_colonnes; c++){
            MLV_draw_filled_circle(c*largeur_case + 150, l*hauteur_case + 200, hauteur_case/10, MLV_COLOR_ALICE_BLUE);
        }
    }
}

void affichage_boutique(int money){
    MLV_draw_rectangle(10, 10, 400, 100, MLV_COLOR_WHITE);
    MLV_Image *mousse, *nougamine, *chalentisseur, *wall, *triplechat;

    mousse = MLV_load_image("images/mousse.png");
    MLV_draw_image(mousse, 30, 35);
    MLV_draw_text(40, 80, "100", MLV_COLOR_WHITE);
    nougamine = MLV_load_image("images/nougamine.png");
    MLV_draw_image(nougamine, 100, 35);
    MLV_draw_text(110, 80, "50", MLV_COLOR_WHITE);
    chalentisseur = MLV_load_image("images/chalentisseur.png");
    MLV_draw_image(chalentisseur, 170, 15);
    MLV_draw_text(180, 80, "75", MLV_COLOR_WHITE);
    wall = MLV_load_image("images/wall.png");
    MLV_draw_image(wall, 230, 15);
    MLV_draw_text(240, 80, "150", MLV_COLOR_WHITE);
    triplechat = MLV_load_image("images/triplechat.png");
    MLV_draw_image(triplechat, 300, 20);
    MLV_draw_text(310, 80, "500", MLV_COLOR_WHITE);
    MLV_draw_text(360, 50, "%d $", MLV_COLOR_YELLOW3, money);

}

/*Affichage de la vague*/

void affiche_vague(Virus* virus, int nb_lignes, int nb_colonnes) {
    /*Affichage de la vague*/
    printf("Vague en approche...\n");
    int i, j;
    Virus* v = virus; /*Pour parcourir la liste des virus*/
    char plateau[nb_lignes][nb_colonnes*2]; /*Pour parcourir la liste des virus*/

    for (i = 0; i < nb_lignes; i++) {
        plateau[i][0] = (char) (i+1 + '0');
        plateau[i][1] = '|';
        for (j = 2; j < nb_colonnes*2; j++) {
            plateau[i][j] = ' ';
        }
        plateau[i][nb_colonnes*2-1] = '\0';
    }

    for(; v != NULL; v = v->next) {
        plateau[v->line - 1][v->turn * 2 + 1] = v->type;
    }

    printf("\n");
    for (i = 0; i < nb_lignes; i++) {
        printf("%s\n", plateau[i]);
    }
    printf("\n");
}

void affiche_vague_graphique(Virus* v, int nb_lignes, int nb_colonnes){
    int hauteur_case = 600/nb_lignes;
    int largeur_case = 1050/nb_colonnes;

    MLV_Image *sidney, *levrier, *stbernard, *healer, *booster;

    sidney = MLV_load_image("images/sidney.png");
    levrier = MLV_load_image("images/levrier.png");
    stbernard = MLV_load_image("images/stbernard.png");
    healer = MLV_load_image("images/healer.png");
    booster = MLV_load_image("images/booster.png");
    for(; v != NULL; v = v->next){

        if(v->type == 'E')
            MLV_draw_image(sidney, v->turn*largeur_case + 85 , v->line*hauteur_case + 85);
        else if(v->type == 'V')
            MLV_draw_image(levrier, v->turn*largeur_case + 85 , v->line*hauteur_case + 80);
        else if(v->type == 'S')
            MLV_draw_image(stbernard, v->turn*largeur_case + 85, v->line*hauteur_case + 80);
        else if(v->type == 'H')
            MLV_draw_image(healer, v->turn*largeur_case + 85, v->line*hauteur_case + 80);
        else if(v->type == 'B')
            MLV_draw_image(booster, v->turn*largeur_case + 85, v->line*hauteur_case + 80);

        MLV_draw_text(v->turn*largeur_case + 105, v->line*hauteur_case + 140, "%d", MLV_COLOR_GREEN, v->life);
    }

}


/*Action des virus et chips*/

void action_chips(Chips* c, Virus* v, int tour){
    /*Action des chips en foncttion de leur type sur les virus */
    Chips* tc = c;
    for(; tc != NULL; tc = tc->next){
        Virus* tv = v;
        for(; tv != NULL; tv = tv->next){
            if(tv->line == tc->line && tv->prev_line == NULL && tv->turn <= tour){
                if(capacite_chips(tc->type) == -1) /*Ralentisseur*/
                    tv->speed = 1;
                else if(capacite_chips(tc->type) == 1) /*Tourelle de base*/
                    tv->life -= 1;
                else if(capacite_chips(tc->type) == 2 && tv->position == tc->position+1){ /*Mine*/
                    tv->life = 0;
                    tc->life = 0;
                }
            }
            if(capacite_chips(tc->type) == 3 && tv->turn <= tour){ /*Grosse tourelle*/
                if(tv->prev_line == NULL &&
                  (tv->line == tc->line-1 || tv->line == tc->line || tv->line == tc->line+1))
                    tv->life -= 3;
                    
            
            }
        }
    }
}

void action_virus(Chips* c, Virus* v, int tour){
    /*Action des virus en foncttion de leur type sur les chips */
    Virus* tv = v; /*Liste des virus*/
    Virus* tb = v; /*/*Liste des virus, pour le booster*/
    for(; tv != NULL; tv = tv->next){ /*Parcours les virus*/

        if(capacite_virus(tv->type) == 0){ /*Healer*/
            if(tv->prev_line != NULL && tv->prev_line->position == tv->position-1)
                if(tv->prev_line->life < 9)
                    tv->prev_line->life += 1;
            if(tv->next_line != NULL && tv->next_line->position == tv->position+1)
                if(tv->next_line->life < 9 && tv->next_line->turn <= tour)
                    tv->next_line->life += 1;
        }

        if(capacite_virus(tv->type) == -1 && tv->turn <= tour){ /*Booster*/
            for(; tb != NULL && tb->line <= tv->line+1 ; tb = tb->next){ /*Reparcours de liste*/
                if(tb->prev_line == NULL &&
                  (tb->line == tv->line-1 || tb->line == tv->line || tb->line == tv->line+1)){
                    tb->speed = 3;
                }
            }
        }

        Chips* tc = c; /*Liste des tourelles*/
        for(; tc != NULL; tc = tc->next){ /*Parcours les tourelles*/
            if(recherche(c,tv->line,tv->position-1)==1 && tv->line == tc->line && tv->position == tc->position+1 && tv->turn <= tour){
                /*Si la position du virus est un cran avant celle de la tourelle*/
                if(capacite_virus(tv->type) == 1){ /*Normal ou rapide*/
                    tc->life -= 1;
                }
                else if(capacite_virus(tv->type) == 2){ /*Gros virus*/
                    tc->life -= 2;
                }
                else if(capacite_virus(tv->type) == 0 || capacite_virus(tv->type) == -1){ /*Healer ou Booster*/
                    tc->life -= 1;
                }
            }
        }
    }
}

void avance_virus(Virus* v, Chips* c, int tour){
    /*Fait avancer les virus sur le plateau en fonction de leur vitesse*/
    for(; v != NULL; v = v->next){ /*parcours de la liste des virus*/
        if(v->turn <= tour){ /*si le virus est sur le plateau*/
            v->position -= v->speed; /*avance de speed cases*/
            if(v->prev_line != NULL){
                if(v->prev_line->position >= v->position) /*dépassement*/
                    v->position = v->prev_line->position+1;
            }
            if(recherche(c, v->line, v->position) == 1){
               /*Soit il est au mm endroit*/
               /*Donc il fait un pas en arrière */
                v->position = v->position+1;
            }
            else if(recherche(c, v->line, v->position+1) == 1){
                /*Soit il est avant*/
                /*Donc il recule de 2*/
                v->position = v->position+2;
            }
            else if(recherche(c,v->line,v->position+2) == 1){
                /*Soit il est 2 avant*/
                /*Donc il recule de 3*/
                v->position = v->position+3;
            }
            if(v->position <= 1) /*Si il est arrivé au bout*/
                v->position = 1;
            
        }
    }
}

Virus* supprime_virus(Virus* v, int *score){
    /*Supprime de la liste les virus morts*/
    if(v == NULL)
        return NULL;

    Virus* prev = v;
    while(prev->life <= 0 && prev->next != NULL){
        v = v->next;
        if(prev->next_line != NULL)
                prev->next_line->prev_line = NULL;
        *score += prev->position;
        free(prev);
        
        prev = v;
    }

    Virus* tmp = v->next;

    if(tmp == NULL){
        if(prev->life <= 0){
            *score += prev->position;
            free(prev);
            return NULL;
        }
    }

    while(tmp != NULL){
        if(tmp->life <= 0){
            prev->next = tmp->next;
            if(tmp->next_line != NULL)
                tmp->next_line->prev_line = NULL;
            *score += tmp->position;
            free(tmp);
            tmp = prev->next;
        }
        else {
            prev = tmp;
            tmp = tmp->next;
        }
    }
    return v;
}

Chips* supprime_chips(Chips* c){
    /*Supprime de la liste les chips morts*/
    if(c == NULL)
        return NULL;
    Chips* prev = c;

    while(prev->life <= 0 && prev->next != NULL){
        c = c->next;
        free(prev);
        prev = c;
    }

    Chips* tmp = c->next;

    if(tmp == NULL){
        if(prev->life <= 0){
            free(prev);
            return NULL;
        }
    }

    while(tmp != NULL){ /*tant que c'est pas le dernier*/
        if(tmp->life <= 0){
            prev->next = tmp->next;
            free(tmp);
            tmp = prev->next;
        }
        else {
            prev = tmp;
            tmp = tmp->next;
        }
    }
    return c;
}


/*Affichage du tour*/

void affichage_tour(Game* jeu, int nb_lignes, int nb_colonnes, int tour) {
    /*Affichage du plateau de jeu en fonction du tour*/
    int i, j;
    Chips* c = jeu->chips;
    Virus* v = jeu->virus;
    char plateau[nb_lignes][nb_colonnes * 3 + 4];

    for (i = 0; i < nb_lignes; i++) {
        plateau[i][0] = (char) (i+1 + '0');
        plateau[i][1] = '|';
        for (j = 2; j < nb_colonnes * 3 + 4; j++) {
            plateau[i][j] = ' ';
        }
        plateau[i][nb_colonnes * 3 + 3] = '\0';
    }
    
    for(; c != NULL; c = c->next) {
        plateau[c->line - 1][c->position * 3 + 1] = c->type;
        plateau[c->line - 1][c->position * 3 + 2] = (char) (c->life + '0');
    }

    for(; v != NULL; v = v->next) {
        if(v->turn <= tour){
            plateau[v->line - 1][v->position * 3 + 1] = v->type;
            plateau[v->line - 1][v->position * 3 + 2] = (char) (v->life + '0');
        }
    }

    printf(" |");
    for (i = 1; i <= nb_colonnes; i++) {
        printf(" %2d", i);
    }
    printf(" |");
    
    printf("\n ");
    for (i = 0; i < nb_colonnes * 3 + 3; i++) {
        printf("-");
    }
    printf("\n");
    
    for (i = 0; i < nb_lignes; i++) {
        printf("%s\n", plateau[i]);
    }
    printf("\n");
}

void affiche_chips_graphique(Chips *c, int nb_lignes, int nb_colonnes, int tour){
    int hauteur_case = 600/nb_lignes;
    int largeur_case = 1050/nb_colonnes;

    MLV_Image *mousse, *nougamine, *chalentisseur, *wall, *triplechat;

    mousse = MLV_load_image("images/mousse.png");
    nougamine = MLV_load_image("images/nougamine.png");
    chalentisseur = MLV_load_image("images/chalentisseur.png");
    wall = MLV_load_image("images/wall.png");
    triplechat = MLV_load_image("images/triplechat.png");

    for(; c != NULL; c = c->next){
        if(c->type == 'A')
            MLV_draw_image(mousse, c->position*largeur_case + 85 , c->line*hauteur_case + 90);
        else if(c->type == 'M')
            MLV_draw_image(nougamine, c->position*largeur_case + 90 , c->line*hauteur_case + 95);
        else if(c->type == 'R')
            MLV_draw_image(chalentisseur, c->position*largeur_case + 95, c->line*hauteur_case + 80);
        else if(c->type == 'W')
            MLV_draw_image(wall, c->position*largeur_case + 95, c->line*hauteur_case + 80);
        else if(c->type == 'X')
            MLV_draw_image(triplechat, c->position*largeur_case + 90, c->line*hauteur_case + 80);

        MLV_draw_text(c->position*largeur_case + 105, c->line*hauteur_case + 140, "%d", MLV_COLOR_GREEN, c->life);
    }
}

void affiche_tour_graphique(Virus* v, Chips *c, int nb_lignes, int nb_colonnes, int tour){
    int hauteur_case = 600/nb_lignes;
    int largeur_case = 1050/nb_colonnes;

    MLV_Image *sidney, *levrier, *stbernard, *healer, *booster;

    sidney = MLV_load_image("images/sidney.png");
    levrier = MLV_load_image("images/levrier.png");
    stbernard = MLV_load_image("images/stbernard.png");
    healer = MLV_load_image("images/healer.png");
    booster = MLV_load_image("images/booster.png");

    for(; v != NULL; v = v->next){
        if(v->turn <= tour){
            if(v->type == 'E')
                MLV_draw_image(sidney, v->position*largeur_case + 85 , v->line*hauteur_case + 85);
            else if(v->type == 'V')
                MLV_draw_image(levrier, v->position*largeur_case + 85 , v->line*hauteur_case + 80);
            else if(v->type == 'S')
                MLV_draw_image(stbernard, v->position*largeur_case + 85, v->line*hauteur_case + 80);
            else if(v->type == 'H')
                MLV_draw_image(healer, v->position*largeur_case + 85, v->line*hauteur_case + 80);
            else if(v->type == 'B')
                MLV_draw_image(booster, v->position*largeur_case + 85, v->line*hauteur_case + 80);

            MLV_draw_text(v->position*largeur_case + 105, v->line*hauteur_case + 140, "%d", MLV_COLOR_GREEN, v->life);
        }
    }
    affiche_chips_graphique(c, nb_lignes, nb_colonnes, tour);
}

/*Fonction de délai*/

void delay(int nb_ms){
	/*Crée un délai*/
	int nb = 1000*nb_ms;
	time_t start = clock();
	while(clock() < start+nb);
}


/*Condition de victoire virus*/

int condition_victoire(Virus* v){
    /*Détecte si un virus a atteint l'arrivée*/
    for(; v!= NULL; v = v->next){
        if (v->position == 1)
            return 1;
    }
    return 0;
}


/*Suppression des listes*/

void supprime_listes(Chips* c, Virus* v){
    /*Vide les listes chainees du jeu*/
    if(c != NULL) {
        Chips* tmpc = c;

        for(; tmpc != NULL;){
            c = c->next;
            free(tmpc);
            tmpc = c;
        }
    }

    if(v != NULL) {
        Virus* tmpv = v;

        for(; tmpv != NULL; ){
            v = v->next;
            free(tmpv);
            tmpv = v;
        }
    }
}


/*Jeu*/

int main(int argc, char *argv[]){

    int mode = verif_arguments(argc, argv);

    /*Initialisation du jeu*/
    Game jeu;
    Virus virus;
    Chips chips;
    jeu.virus = NULL;
    jeu.chips = NULL;
    virus.next = NULL;
    chips.next = NULL;

    int m;
    for(m = 1; m <= 5 ; m++){
        jeu.turn = 1; /*Premier tour*/
        int score = 0;

        /*Ouverture du fichier*/
        FILE* fichier; /*Pointeur vers le fichier*/
        char nom_fichier[10];
        char buffer[2];
        strcpy(nom_fichier,"level");
        sprintf(buffer,"%d",m);
        strcat(nom_fichier,buffer);
        strcat(nom_fichier,".txt");
        fichier = fopen(nom_fichier, "r");

        /*Lecture du fichier*/
        if (fichier != NULL){
            /*Initialisation de la monnaie*/
            int monnaie;
            fscanf(fichier,"%d",&monnaie);
            jeu.money = monnaie;
        }

        /*Chaînage des virus*/
        init_liste_virus(fichier, &jeu, &virus); /*création de la liste chaînée des virus*/

        /*Fermeture du fichier*/
        fclose(fichier);

        int nb_lignes = max_line(jeu.virus); /*nombre de lignes du plateau*/
        int nb_colonnes = 3*max_turn(jeu.virus); /*nombre de colonnes du plateau*/

        chainage_ligne(jeu.virus, nb_lignes); /*chaînage par lignes des virus*/
        init_position_virus(jeu.virus, nb_colonnes); /*attribution des positions*/


        /*[ASCII] Affichage de la vague*/
        if (mode == 1){

            affiche_vague(jeu.virus, nb_lignes, nb_colonnes);


            /*Déploiement des tourelles*/
            Pose_tourelles(&jeu, &chips, nb_lignes, nb_colonnes, &score);

            for( ; jeu.virus != NULL && condition_victoire(jeu.virus) != 1; jeu.turn++){
                printf("Tour %d :\n", jeu.turn);
                printf("Score : %d\n", score);
                affichage_tour(&jeu, nb_lignes, nb_colonnes, jeu.turn);
                action_chips(jeu.chips, jeu.virus, jeu.turn);
                action_virus(jeu.chips, jeu.virus, jeu.turn);
                avance_virus(jeu.virus, jeu.chips, jeu.turn);
                jeu.virus = supprime_virus(jeu.virus, &score);
                jeu.chips = supprime_chips(jeu.chips);
                delay(1000);
            }
            affichage_tour(&jeu, nb_lignes, nb_colonnes, jeu.turn);

            if(jeu.virus == NULL){
                printf("Le joueur a gagné Mashallah bien ouej martinx\n");
                printf("Score : %d\n", score);
            }
            else
                printf("Les voris ont gagné, t'es éclatax mon reuf\n");
        }

        else if(mode == 2){ /*graphique*/

            int x, y;
            char selec_chips = 'O'; /*Garder la sélection*/

            MLV_create_window("Projet", "Projet",1200, 800);

            affiche_plateau_graph(nb_lignes, nb_colonnes);
            affichage_boutique(jeu.money);
            MLV_actualise_window();
            for(;;){
                MLV_wait_mouse(&x,&y); /*Attend un clic souris*/
                if(x>=10 && x<=510 && y>=10 && y<=110){ /*Boutique*/
                    selec_chips = selec_type_chips(x, y);
                }

                else if (x>=120 && x<=1150 && y>=140 && y<=750 && selec_chips != 'O'){ /*Plateau*/
                    MLV_clear_window(MLV_COLOR_WHITE);
                    affiche_plateau_graph(nb_lignes, nb_colonnes);
                    Pose_tourelles_graph(&jeu, &chips, selec_chips, x, y, nb_lignes, nb_colonnes, &score);
                    affiche_chips_graphique(jeu.chips, nb_lignes, nb_colonnes, jeu.turn);
                    affichage_boutique(jeu.money);
                    MLV_actualise_window();
                }
                else if(x>=610 && x<=810 && y>=10 && y<=110){ /*Voir la vague*/
                    MLV_clear_window(MLV_COLOR_WHITE);
                    affiche_plateau_graph(nb_lignes, nb_colonnes);
                    affichage_boutique(jeu.money);
                    affiche_vague_graphique(jeu.virus, nb_lignes, nb_colonnes);
                    MLV_actualise_window();
                    for(;;){
                        MLV_wait_mouse(&x,&y);
                        if (x>=610 && x<=810 && y>=10 && y<=110){
                            MLV_clear_window(MLV_COLOR_WHITE);
                            affiche_plateau_graph(nb_lignes, nb_colonnes);
                            affichage_boutique(jeu.money);
                            affiche_chips_graphique(jeu.chips, nb_lignes, nb_colonnes, jeu.turn);
                            MLV_actualise_window();
                            break;
                        }
                        else if(x>=1050 && x<=1190 && y>=10 && y<=110){ /*quitter*/
                            supprime_listes(jeu.chips, jeu.virus);
                            MLV_free_window();
                        }
                    }
                }
                else if(x>=1050 && x<=1190 && y>=10 && y<=110){ /*Quitter*/
                    supprime_listes(jeu.chips, jeu.virus);
                    MLV_free_window();
                }
                else if(x>=830 && x<=1030 && y>=10 && y<=110){ /*Commencer*/
                    
                    for( ; jeu.virus != NULL && condition_victoire(jeu.virus) != 1; jeu.turn++){
                        MLV_clear_window(MLV_COLOR_WHITE);
                        affiche_plateau_graph(nb_lignes, nb_colonnes);
                        affichage_boutique(jeu.money);
                        MLV_draw_text(20, 120, "Tour %d", MLV_COLOR_WHITE, jeu.turn);
                        MLV_draw_text(20, 140, "Score : %d", MLV_COLOR_WHITE, score);

                        
                        affiche_tour_graphique(jeu.virus, jeu.chips, nb_lignes, nb_colonnes, jeu.turn); /**/
                        
                        action_chips(jeu.chips, jeu.virus, jeu.turn);
                        action_virus(jeu.chips, jeu.virus, jeu.turn);
                        avance_virus(jeu.virus, jeu.chips, jeu.turn);
                        jeu.virus = supprime_virus(jeu.virus, &score);
                        jeu.chips = supprime_chips(jeu.chips);
                        MLV_actualise_window();
                        delay(1000);
                    }
                    MLV_clear_window(MLV_COLOR_WHITE);
                    affiche_plateau_graph(nb_lignes, nb_colonnes);
                    affichage_boutique(jeu.money);
                    affiche_tour_graphique(jeu.virus, jeu.chips, nb_lignes, nb_colonnes, jeu.turn); /**/


                    if(jeu.virus == NULL){
                        MLV_draw_text(20, 120, "Le joueur a gagné Mashallah bien ouej martinx", MLV_COLOR_WHITE);
                        MLV_draw_text(20, 140, "Score : %d", MLV_COLOR_WHITE, score);
                    }
                    else{
                        MLV_draw_text(20, 120, "Les voris ont gagné, t'es éclatax mon reuf", MLV_COLOR_WHITE);
                    }

                    MLV_actualise_window();
                }
            }
        }

        supprime_listes(jeu.chips, jeu.virus);
    }

    return 0;
}