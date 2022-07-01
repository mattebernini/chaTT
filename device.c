#include "./lib/all.h"

// data una stringa restituisce il comando corrispondente
enum comandi_device quale_comando_dev(char* comando)
{
    if(strncmp(comando, "signup", strlen("signup"))==0)
        return signup;
    if(strncmp(comando, "in", strlen("in"))==0)
        return in;
    if(strncmp(comando, "hanging", strlen("hanging"))==0)
        return hanging;
    if(strncmp(comando, "show", strlen("show"))==0)
        return show;
    if(strncmp(comando, "chat", strlen("chat"))==0)
        return chat;
    if(strncmp(comando, "share", strlen("share"))==0)
        return share;
    if(strncmp(comando, "out", strlen("out"))==0)
        return out;
    return nonvalid;
}

// implementazione del protocollo di avviso lato client
void protocollo_avviso_dev(int avviso, char* username, char* password, int porta)
{
    int ret, sd;
    uint16_t lmsg;    
    socklen_t len; 
    struct sockaddr_in srv_addr;

    uint16_t porta_dev = (uint16_t) porta;
    // variabile da inviare ad inizio protocollo
    // che conterrà le risposte di ak del server
    uint16_t byte_ak;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(1);
    }
    printf("connessione al server effettuata\n");
    
    // dico al server cosa ho intenzione di fare
    // 1 iscrizione, 2 login, 3 logout
    byte_ak = avviso;
    printf("mando byte di ak = %d\n", byte_ak);
    byte_ak = htons(byte_ak);
    ret = send(sd, (void*)&byte_ak, sizeof(uint16_t), 0);
    
    // aspetto il suo ak
    ret = recv(sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // invio username mandando prima la lunghezza del msg
    printf("invio username\n");
    len = strlen(username);
    //printf("username contiene: %s\n", username);
    //printf("len username = %d\n", len);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) username, len, 0);

    // ricevo risposta su username
    // flag MSG_DONTWAIT fa attendere la ricezione 120 secondi
    byte_ak = -1;
    ret = recv(sd, (void*)&byte_ak, sizeof(uint16_t), MSG_DONTWAIT);
    if(byte_ak == -1){
        // nessuna risposta dal server
        printf("nessuna risposta dal server\n");
        exit(1);
    }
    byte_ak = ntohs(byte_ak);
    // ricevo risposta validità username
    if(byte_ak==1){
        printf("username non valido\n");
        exit(1);
    }
    printf("username valido\n");
    // logout
    if(avviso == 3){
        printf("logout eseguito con successo\n");
        return;
    }
    /* LOGOUT TERMINATO */

    // invio la password preceduta dalla sua lunghezza
    printf("invio password\n");
    len = strlen(password);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) password, len, 0);

    // ricevo risposta su password
    ret = recv(sd, (void*)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);
    if(byte_ak == 1){
        printf("password non corretta\n");
        exit(1);
    }
    printf("password corretta\n");

    // invio la porta
    printf("invio porta\n");
    porta_dev = htons(porta_dev);
    ret = send(sd, (void*) &porta_dev, sizeof(uint16_t), 0);

    // aspetto ak finale
    ret = recv(sd, (void*)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);
    if(avviso==1 && byte_ak==0)
        printf("iscrizione andata a buon fine\n");
    /** FINE ISCRIZIIONE */
    /** FINE LOGIN */
    // Chiusura del socket connesso
    close(sd);

}
// prende dal server le informazioni relative alla segreteria
void protocollo_segreteria_dev_hanging(char* username, int porta)
{
    int ret, sd;
    uint16_t lmsg;    
    socklen_t len; 
    struct sockaddr_in srv_addr;
    struct sockaddr_in dest_addr;

    uint16_t porta_dev = (uint16_t) porta;
    uint16_t byte_ak;

    char* hanging_str;


    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(1);
    }
    printf("connessione al server effettuata\n");

    // faccio capire al server che voglio iniziare una conversazione
    byte_ak = 100;
    printf("mando byte di ak = %d\n", byte_ak);
    byte_ak = htons(byte_ak);
    ret = send(sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto il suo ak
    ret = recv(sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // invio mio username mandando prima la lunghezza del msg
    printf("invio username\n");
    len = strlen(username);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) username, len, 0);

    // ricevo la stringa da stampare
    ret = recv(sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    hanging_str = (char*) malloc((lmsg+1)*sizeof(char));
    ret = recv(sd, (void *)hanging_str, lmsg, 0);
    hanging_str[lmsg-1] = '\0';

    if(strlen(hanging_str) > 3)
        printf("\n***SEGRETERIA***\n%s\n", hanging_str);
    else
        printf("\nsegreteria vuota...\n");
}
// legge i messaggi lasciati in segreteria da source
void protocollo_segreteria_dev_show(char* username, char* source, int porta)
{
    int ret, sd;
    uint16_t lmsg;    
    socklen_t len; 
    struct sockaddr_in srv_addr;
    struct sockaddr_in dest_addr;

    uint16_t porta_dev = (uint16_t) porta;
    uint16_t byte_ak;

    char* pending_msg;


    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(1);
    }
    printf("connessione al server effettuata\n");

    // faccio capire al server che voglio iniziare una conversazione
    byte_ak = 200;
    printf("mando byte di ak = %d\n", byte_ak);
    byte_ak = htons(byte_ak);
    ret = send(sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto il suo ak
    ret = recv(sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // invio mio username mandando prima la lunghezza del msg
    printf("invio mio username\n");
    len = strlen(username);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) username, len, 0);

    // aspetto il suo ak
    ret = recv(sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // invio username del source mandando prima la lunghezza del msg
    printf("invio username mittente\n");
    len = strlen(source);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) source, len, 0);
    
    // ricevo la stringa da stampare con i msg pendenti
    ret = recv(sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    pending_msg = (char*) malloc((lmsg+1)*sizeof(char));
    ret = recv(sd, (void *)pending_msg, lmsg, 0);
    pending_msg[lmsg-1] = '\0';

    source[strlen(source)-1] = '\0';
    if(strlen(pending_msg) > 1)
        printf("\n***MESSAGGI DA: %s***\n%s\n", source, pending_msg);
    else
        printf("\nnessun messaggio pendente da %s\n", source);

    // avvisare source che abbiamo letto...
}

// inizia una conversazione con l'utente dest
// se questo è online avviene uno scambio di msg p2p
// altrimenti i msg vengono lasciati al server
void protocollo_msg_dev(char* source, char* dest, int porta_source)
{
    int ret, sd;
    uint16_t lmsg;    
    socklen_t len; 
    struct sockaddr_in srv_addr;
    struct sockaddr_in dest_addr;

    uint16_t porta_dev = (uint16_t) porta_source, porta_dest;
    // variabile da inviare ad inizio protocollo
    // che conterrà le risposte di ak del server
    uint16_t byte_ak;

    char* messaggio_ricevuto;
    char messaggio_inviato[MAX_MSG_LEN];


    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(1);
    }
    printf("connessione al server effettuata\n");
    
    // faccio capire al server che voglio iniziare una conversazione
    byte_ak = 10;
    printf("mando byte di ak = %d\n", byte_ak);
    byte_ak = htons(byte_ak);
    ret = send(sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto il suo ak
    ret = recv(sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // invio username del destinatario mandando prima la lunghezza del msg
    printf("invio username dest\n");
    len = strlen(dest);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send(sd, (void*) dest, len, 0);

    // mi risponde con la porta del destinatario
    ret = recv(sd, (void *)&porta_dest, sizeof(uint16_t), 0);
    porta_dest = ntohs(porta_dest);

    // invio un ak
    byte_ak = 0;
    byte_ak = htons(byte_ak);
    ret = send(sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    if(porta_dest == 0){
        // il destinatario è offline
        // lascio i messaggi al server
        printf("il destinatario è offline\n");
        printf("invio al server il mio username\n");
        len = strlen(dest);
        lmsg = htons(len);
        ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
        ret = send(sd, (void*) source, len, 0);
        printf("ora il server sa chi sono, posso lasciare i messaggi a %s\n", dest);

        while(1)
        {
            fgets(messaggio_inviato, MAX_MSG_LEN, stdin);
            len = strlen(messaggio_inviato);
            messaggio_inviato[len-1] = '\0';
            lmsg = htons(len);
            ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
            ret = send(sd, (void*) messaggio_inviato, len, 0);
            if(strncmp(messaggio_inviato, "\\q", 2) == 0)
                break;
        }
    }
    else {
        // il destinatario è online
        // instauro una connessione con lui
        printf("il destinatario è online, porta %d\n", porta_dest);
    }
    close(sd);
}

// mostra a schermo il menu dei comandi leggendolo dal file menu.txt
void mostra_comandi()
{
    char str[50] = "./device/menu.txt";
    printfile(str);
}

int main(int argc, char* argv[])
{
    int porta;
    char comando[COMANDO_MAX_LEN];
    enum comandi_device cmd;
    char* username;
    char* password;
    char* dest;
    char** comando_splitted;
    int len;
    pid_t pid, child_pid;

    // assegno il valore alla porta su cui comunica il server
    if(argv[1] == NULL){
        printf("inserire la porta...");
        exit(1);
    }
    else
        porta = atoi(argv[1]);

    // fase di avvio
    do{
        printf("signup -> per iscriversi \nin -> per il login\n\n");
        fgets(comando, COMANDO_MAX_LEN, stdin);
        printf("\n");
        cmd = quale_comando_dev(comando);
        // estraggo username e password dalla stringa comando
        comando_splitted = str_split(comando, ' ');

        len = strlen(*(comando_splitted+1));
        username = (char*) malloc(sizeof(char)*(len+1));
        strcpy(username, *(comando_splitted+1));

        len = strlen(*(comando_splitted+2));
        password = (char*) malloc(sizeof(char)*(len+1));
        strcpy(password, *(comando_splitted+2));
        
        switch (cmd)
        {
            case signup:
                printf(username);
                protocollo_avviso_dev(1, username, password, porta);
                mostra_comandi();
                break;
            case in:
                protocollo_avviso_dev(2, username, password, porta);
                mostra_comandi();
                break;
            default:
                printf("comando non valido...\n\n");
                break;
        } 
    } while(cmd != signup && cmd != in);
    

    // ciclo infinito che prende in input un comando ed esegue
    // l'azione corrispondente
    while(1)
    {
        fgets(comando, COMANDO_MAX_LEN, stdin);
        printf("\n");
        cmd = quale_comando_dev(comando);

        // estraggo la seconda parola da comando che chiamo dest
        // ma in realtà può essere sia l'username destinatario che il filename del file da inviare
        comando_splitted = str_split(comando, ' ');
        if(*(comando_splitted+1) != NULL){
            len = strlen(*(comando_splitted+1));
            dest = (char*) malloc(sizeof(char)*(len+1));
            strcpy(dest, *(comando_splitted+1));
        }

        switch (cmd)
        {
            case out:
                // avviso il server che esco
                protocollo_avviso_dev(3, username, password, porta);
                exit(0);
                break;
            case chat:
                protocollo_msg_dev(username, dest, porta);
                break;
            case hanging:
                protocollo_segreteria_dev_hanging(username, porta);
                break;
            case show:
                protocollo_segreteria_dev_show(username, dest, porta);
                break;
            default:
                printf("comando non valido...\n\n");
                break;
        } 
        printf("\n");
    }
}