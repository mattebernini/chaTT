#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "costanti.h"

// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**) malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

// scrive nello stdoutput il contenuto di un file
void printfile(char* percorso)
{
    char c;
    FILE* fd;

    fd = fopen(percorso, "r" );
    if (fd == NULL) 
        return;

    c = fgetc(fd);
    while (c != -1)
    {
        printf("%c", c);
        c = fgetc(fd);
    }
  
    fclose(fd);
}

// scrive nel file utenti.txt username e password 
// aggiorna registro.txt con i dati del nuovo utente
void salva_iscrizione(char* username, char* password, int porta_dev)
{
    FILE* fd;
    int t = (int) time(NULL);

    fd = fopen("./server/utenti.txt", "a");
    if (fd == NULL) 
        return;
    fprintf(fd, "%s,%s", username, password);
    
    fd = fopen("./server/registro.txt", "a");
    if (fd == NULL) 
        return;
    fprintf(fd, "%s,%d,%d,%d\n", username, porta_dev, t, 0);
    
    fclose(fd);
}

// effettua controlli sullo username in base alla variabile tipo
// se 1 controllo per iscrizione, 2 login, 3 logout
int controllo_username(char* username, int lmsg, int tipo)
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;

    if(lmsg > USERNAME_MAX_LEN || lmsg == 1)
            return 1;

    fd = fopen("./server/utenti.txt", "r");
    if (fd == NULL) 
        return 1;

    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        // se lo username è contenuto in questo file 
        // in iscrizione non è valido
        if(strcmp(*(parole+0), username) == 0 && tipo == 1)
            return 1;
        // invece in login e logout va bene
        if(strcmp(*(parole+0), username) == 0 && tipo != 1)
            return 0;
    }
    // se non è stata trovata una corrispondenza in login e logout errore
    if(tipo != 1)
        return 1;   
    fclose(fd);
    return 0;
}

int controllo_password(char* username, char* password, int lmsg, int tipo)
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;

    // la password in fase di iscrizione deve essere lunga
    // massimo PASSWORD_MAX_LEN e minimo 3
    if(lmsg > PASSWORD_MAX_LEN || lmsg < 3)
        return 1;
    if(tipo == 1)
        return 0;

    // controlli su login e logout
    fd = fopen("./server/utenti.txt", "r");
    if (fd == NULL) 
        return 1;

    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        // trovo lo username corrispondente alla password
        if(strcmp(*(parole+0), username) == 0){
            // se la password corrisponde va bene
            if(strncmp(*(parole+1), password, lmsg) == 0){
                return 0;
            }
        }
    }
    // se non è stata trovata una corrispondenza in login e logout errore
    fclose(fd);
    return 1;   
}

void scrivi_registro(char* username, int porta, int tipo)
{
    FILE* fd1;
    FILE* fd2;
    char new_line[100];
    char* line = NULL;
    size_t len = 0;
    ssize_t read;    
    char** parole;
    int t = (int) time(NULL);
    int count, line_target;

    fd1 = fopen("./server/registro.txt", "r");
    fd2 = fopen("./server/registro.tmp", "a"); 
    if (fd1 == NULL || fd2 == NULL) 
        return;
    
    line_target = 0;
    while ((read = getline(&line, &len, fd1)) != -1) 
    {
        line_target++;
        parole = str_split(line, ',');
        // trovo lo username corrispondente
        if(strncmp(*(parole+0), username, strlen(username)) == 0)
            break;
    }    

    fclose(fd1);
    fd1 = fopen("./server/registro.txt", "r");
    count = 0;
    while ((read = getline(&line, &len, fd1)) != -1) 
    {
        count++;
        if (count != line_target)
            fprintf(fd2, line);
        else
        {
            parole = str_split(line, ',');
            if(tipo==1)
            // login -> cambio porta e ts_login
                sprintf(new_line, "%s,%d,%d,%s", *(parole+0), porta, t, *(parole+3));
            else
            // logout -> cambio ts_logout
                sprintf(new_line, "%s,%s,%s,%d\n", *(parole+0), *(parole+1), *(parole+2), t);
            fprintf(fd2, new_line);
        }
    }
    fclose(fd1);
    fclose(fd2);

    // scambio il tmp con l'originale
    remove("./server/registro.txt");
    rename("./server/registro.tmp", "./server/registro.txt");
}

// scrive nel campo login_timestamp di username nel file registro.txt
void registra_login(char* username, int porta)
{
    scrivi_registro(username, porta, 1);
}   
// scrive nel campo logout_timestamp di username nel file registro.txt
void registra_logout(char* username, int porta)
{
    scrivi_registro(username, porta, 2);
}
// stampa “username*timestamp*porta” degli utenti online
void print_online()
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;
    time_t logout_ts, login_ts;
    int count = 0;

    struct tm  ts;
    char       timestamp[80];

    fd = fopen("./server/registro.txt", "r");
    if (fd == NULL) 
        return;

    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        // condizione per essere online:
        // logout_ts < login_ts
        logout_ts = (time_t) atoi(*(parole+3));
        login_ts = (time_t) atoi(*(parole+2));
        if(logout_ts < login_ts)
        {
            ts = *localtime(&login_ts);
            strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", &ts);
            printf("%s*%s*%s\n",
                    *(parole+0),
                    timestamp,
                    *(parole+1));
            count++;
        }
    }
    if(count == 0)
        printf("nessun utente online\n");
    printf("\n");
    fclose(fd);
}

// restituisce la porta relativa a dest, 0 se dest è offline
int porta_utente_online(char* dest)
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;
    time_t logout_ts, login_ts;
    char* username;

    fd = fopen("./server/registro.txt", "r");
    if (fd == NULL) 
        return -1;


    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        // condizione per essere online:
        // logout_ts < login_ts
        logout_ts = (time_t) atoi(*(parole+3));
        login_ts = (time_t) atoi(*(parole+2));
        username = (char*) malloc(sizeof(char)*(strlen(*(parole+0)))); 
        strcpy(username, *(parole+0));
        if(logout_ts < login_ts && strncmp(username, dest, strlen(username))==0){
            fclose(fd);
            return atoi(*(parole+1));
        }
    }
    fclose(fd);
    return 0;
}
// salva il messaggio in pending_msg.txt
// i dati relativi in pending_data.txt
// aggiorna segreteria.txt
void salva_msg_pendente(char* source, char* dest, char* pending_msg)
{
    FILE* fd;
    FILE* fd2;
    char new_line[100];
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;
    int t = (int) time(NULL);
    int count = 1, i = 0, quanti;

    fd = fopen("./server/pending_msg.txt", "a");
    if (fd == NULL) 
        return;
    fprintf(fd, "%s\n", 
        pending_msg);
    fclose(fd);

    fd = fopen("./server/pending_data.txt", "a");
    if (fd == NULL) 
        return;
    fprintf(fd, "%s,%s,%d\n", 
        source, dest, t);
    fclose(fd);

    fd = fopen("./server/segreteria.txt", "r");
    if (fd == NULL) 
        return;

    // controllo se esiste già una riga con questi source e dest
    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        if(strncmp(*(parole+1), dest, strlen(*(parole+1)))==0
           && strncmp(*(parole+0), source, strlen(*(parole+0)))==0){
            // metto count a 0 così so che devo modificare quella riga
            count = 0;
            break;
        }
        i++;
    }
    if(count == 0){
        // modifico il valore della riga i
        fd = fopen("./server/segreteria.tmp", "a");
        fd2 = fopen("./server/segreteria.txt", "r");
        if (fd == NULL) 
            return;
        while ((read = getline(&line, &len, fd2)) != -1) 
        {
            if (count != i)
                fprintf(fd, line);
            else
            {
                parole = str_split(line, ',');
                quanti = atoi(*(parole+2));
                sprintf(new_line, "%s,%s,%d,%d\n", *(parole+0), *(parole+1), ++quanti, t);
                fprintf(fd, new_line);
            }
            count++;
        }
        remove("./server/segreteria.txt");
        rename("./server/segreteria.tmp", "./server/segreteria.txt");
    }
    else {
        // aggiungo una riga
        fd2 = fopen("./server/segreteria.txt", "a");
        fprintf(fd2, "%s,%s,%d,%d\n", source, dest, 1, t);
        fclose(fd2);
    }
    fclose(fd);
}

// restituisce una stringa riassuntiva con scritto:
// gli utenti che hanno lasciato msg pendenti ad username,
// il numero di tali messaggi e l'ultimo timestamp
void leggi_segreteria(char* username, char* ris)
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;
    struct tm  ts;
    char       timestamp[80];
    time_t last_msg_ts;

    fd = fopen("./server/segreteria.txt", "r");
    if (fd == NULL) 
        return;

    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        // le righe in cui username è dest
        if(strncmp(*(parole+1), username, strlen(*(parole+1)))==0){
            last_msg_ts = (time_t) atoi(*(parole+3));
            ts = *localtime(&last_msg_ts);
            strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", &ts);
            
            sprintf(ris, "%s: %s messaggi, %s\n", 
                    *(parole+0), *(parole+2), timestamp);
        }
    }
    
    fclose(fd);
}

void delete_line(char* file_path, int line_target)
{
    FILE* fd;
    FILE* tmp;
    char buffer[MAX_MSG_LEN];
    int c = 0;

    fd = fopen(file_path, "r");
    tmp = fopen("delete.tmp", "a");
    if (fd == NULL || tmp == NULL) 
        return;

    while ((fgets(buffer, MAX_MSG_LEN, fd)) != NULL){
        if (line_target != c)
            fputs(buffer, tmp);
        c++;
    }
    remove(file_path);
    rename("delete.tmp", file_path);
    fclose(fd);
    fclose(tmp);
}

// mette in ris tutti i msg pendenti 
void msg_pendenti(char* dest, char* mittente, char* ris)
{
    FILE* fd;   
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** parole;
    int righe[MAX_PENDING_MSG];
    int conta, i, deleted;

    fd = fopen("./server/pending_data.txt", "r");
    if (fd == NULL) 
        return;

    // raccolgo le righe a cui sono i messaggi
    conta = 0;
    i = 0;
    deleted = 0;
    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        if(strncmp(*(parole+0), mittente, strlen(*(parole+0)))==0
           && strncmp(*(parole+1), dest, strlen(*(parole+1)))==0){
            righe[i++] = conta;
            delete_line("./server/pending_data.txt", conta-deleted);
            deleted++;
        }
        conta++;
    }
    fclose(fd);

    fd = fopen("./server/pending_msg.txt", "r");
    if (fd == NULL) 
        return;
    
    // prelevo i messaggi a quelle righe
    conta = 0;
    i = 0;
    deleted = 0;
    while ((read = getline(&line, &len, fd)) != -1) {
        if(righe[i] == conta){
            i++;
            // aggiungo riga a ris
            sprintf(ris, "%s%s", ris, line);
            // cancello la riga
            delete_line("./server/pending_msg.txt", conta-deleted);
            deleted++;
        }
        conta++;
    }
    fclose(fd);

    fd = fopen("./server/segreteria.txt", "r");
    if (fd == NULL) 
        return;

    // svuoto segreteria
    conta = 0;
    deleted = 0;
    while ((read = getline(&line, &len, fd)) != -1) {
        parole = str_split(line, ',');
        if(strncmp(*(parole+0), mittente, strlen(*(parole+0)))==0
           && strncmp(*(parole+1), dest, strlen(*(parole+1)))==0){
            delete_line("./server/segreteria.txt", conta-deleted);
            deleted++;
        }
        conta++;
    }
    fclose(fd);
}