#include "./lib/all.h"

/*
********************** variabili globali ***********************
*/
// info di questo device
char* username;
char* password; 
int porta;

// socket per la ricezione
fd_set sockets;
int sd_ascolto;
// socket connesso al server
int sd_srv;

// thread
pthread_t th_chat_gruppo;
pthread_t th_ascolto_server, th_invio_msg;
pthread_t th_ricezione, th_invio;

// variabili di stato
int invitato_qualcuno = 0;  // 1 se hai invitato un utente nella chat
int chat_finita = 0;
int dest_online = 0;
int fine_msg_pendenti = 0;
int chat_di_gruppo = 0;     // se fai parte di una chat di gruppo

/*************** info utenti collegati al device ********************/
struct user user_info[MAX_CHAT_GROUP_MEMBERS];
int primo_user = 0;
int n_chat_group_members = 0;

void azzera_user_info()
{
    int i;
    for (i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
        user_info[i].sd = -1;    
}
void elimina_user(int sd)
{
    int i;
    for(i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
            if(user_info[i].sd == sd)
                break;
    user_info[i].sd = -1;
    user_info[i].porta = 0;
    n_chat_group_members--;
    strcpy(user_info[i].nome, "");
}
int first_free_spot()
{
    int i;
    for(i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
        if(user_info[i].sd == -1)
                return i;
    return -1;
}
void salva_user(char* n, int p, int s)
{
    int first_free;
    first_free = first_free_spot();
    strncpy(user_info[first_free].nome, n, strlen(n));
    user_info[first_free].porta = p;
    user_info[first_free].sd = s;
    FD_SET(user_info[first_free].sd, &sockets);
    n_chat_group_members++;
}
int get_user_pos(char* nome, int p, int sd)
{
    int i;
    for(i = 0; i<MAX_CHAT_GROUP_MEMBERS; i++)
        if(user_info[i].sd == sd || 
                user_info[i].porta == p || 
                strncmp(user_info[i].nome, nome, strlen(user_info[i].nome)) == 0)
            return i;
    return -1;
}
void stampa_user_info()
{
    int i;
    for(i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
        printf("%s,%d,%d\n",
                user_info[i].nome, user_info[i].porta, user_info[i].sd);
    printf("chat members = %d\n", n_chat_group_members); 
}

/******************************************************/


// dichiarazione funzioni chiamate nel main
void protocollo_avviso_dev(int avviso);
void protocollo_segreteria_dev_hanging();
void protocollo_segreteria_dev_show(char* source, int stampa);
void protocollo_msg_dev(char* dest);
void mostra_comandi();
void p2p(int srv);
void* ascolta_chat_gruppo(void* arg);
/*
********************** MAIN ***********************
*/
int main(int argc, char* argv[])
{
    char comando[COMANDO_MAX_LEN];
    enum comandi_device cmd;
    char* arg1;
    char** comando_splitted;
    int len;

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
        pulisci_stringa(comando);
        
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
        
        printf("username: %s\npassword: %s\n",username, password);

        // mi connetto al server
        sd_srv = connessione_con_host(SERVER_PORT);

        switch (cmd)
        {
            case signup:
                protocollo_avviso_dev(1);
                mostra_comandi();
                break;
            case in:
                protocollo_avviso_dev(2);
                mostra_comandi();
                break;
            default:
                printf("comando non valido...\n\n");
                break;
        } 
    } while(cmd != signup && cmd != in);
    
    close(sd_srv);
    azzera_user_info();
    // ciclo infinito che prende in input un comando ed esegue
    // l'azione corrispondente
    while(1)
    {
        //pthread_create(&th_chat_gruppo, NULL, ascolta_chat_gruppo, NULL);
        
        fgets(comando, COMANDO_MAX_LEN, stdin);
        pulisci_stringa(comando);
        printf("\n");
        cmd = quale_comando_dev(comando);

        //pthread_cancel(th_chat_gruppo);

        // estraggo la seconda parola da comando che chiamo dest
        // ma in realtà può essere sia l'username destinatario che il filename del file da inviare
        comando_splitted = str_split(comando, ' ');
        if(*(comando_splitted+1) != NULL){
            len = strlen(*(comando_splitted+1));
            arg1 = (char*) malloc(sizeof(char)*(len+1));
            strcpy(arg1, *(comando_splitted+1));
        }

        // mi connetto al server
        sd_srv = connessione_con_host(SERVER_PORT);

        switch (cmd)
        {
            case out:
                protocollo_avviso_dev(3);
                close(sd_srv);
                exit(0);
                break;
            case chat:
                protocollo_msg_dev(arg1);
                break;
            case hanging:
                protocollo_segreteria_dev_hanging();
                break;
            case show:
                protocollo_segreteria_dev_show(arg1, 1);
                break;
            default:
                printf("comando non valido...\n\n");
                break;
        } 

        close(sd_srv);
        printf("\n");
    }
}

// mostra a schermo il menu dei comandi leggendolo dal file menu.md
void mostra_comandi()
{
    char str[50] = "./device/menu.md";
    printf("***************************** LOGIN EFFETTUATO ********************************\n");
    printf("***************************** username: %s *********************************\n", username);
    printfile(str);
}
                
void protocollo_avviso_dev(int avviso)
{
    int sd;
    int bytes;

    sd = sd_srv;

    // dico al server cosa ho intenzione di fare
    // 1 iscrizione, 2 login, 3 logout
    printf("rivelo al server le mie intenzioni\n");
    invia_int(sd, avviso);

    // aspetto che abbia capito
    bytes = ricevi_int(sd);

    // invio il mio username
    printf("invio username\n");
    invia_stringa(sd, username);

    // ricevo risposta
    bytes = ricevi_int(sd);
    if(bytes==1){
       printf("username non valido\n");
        exit(1);
    }
    else
        printf("username valido\n");

    // logout
    if(avviso == 3){
        printf("logout eseguito con successo\n");
        return;
    }
    /* LOGOUT TERMINATO */

    // invio la mia password
    printf("invio password\n");
    invia_stringa(sd, password);

    // ricevo risposta
    bytes = ricevi_int(sd);
    if(bytes==1){
       printf("password non corretta\n");
        exit(1);
    }
    else
        printf("password corretta\n");

    // invio la porta
    printf("invio porta\n");
    invia_int(sd, porta);

    // aspetto ack finale
    bytes = ricevi_int(sd);

    if(avviso==1 && bytes==0)
        printf("iscrizione andata a buon fine\n");
    /** FINE ISCRIZIIONE */
    /** FINE LOGIN */
}

// prende dal server le informazioni relative alla segreteria e le stampa
void protocollo_segreteria_dev_hanging()
{
    int sd, len;
    char hanging_str[200];

    sd = sd_srv;

    // dico al server cosa ho intenzione di fare
    invia_int(sd, 100);

    // aspetto che abbia capito
    ricevi_int(sd);

    // invio il mio username
    invia_stringa(sd, username);

    // ricevo la lista dei messaggi lasciati in segreteria
    len = ricevi_stringa(sd, hanging_str);
    pulisci_stringa(hanging_str);

    if(len > 1)
        printf("\n***SEGRETERIA***\n%s\n", hanging_str);
    else
        printf("\nsegreteria vuota...\n");
}

// legge i messaggi lasciati in segreteria da source
void protocollo_segreteria_dev_show(char* source, int stampa)
{
    int sd, len;
    char pending_msg[200];

    sd = sd_srv;

    // dico al server cosa ho intenzione di fare
    invia_int(sd, 200);

    // aspetto che abbia capito
    ricevi_int(sd);

    // invio il mio username
    invia_stringa(sd, username);

    // aspetto che abbia capito
    ricevi_int(sd);

    // invio username di chi voglio leggere i messaggi pendenti
    invia_stringa(sd, source);

    // ricevo i messaggi pendenti
    len = ricevi_stringa(sd, pending_msg);
    pending_msg[len] = '\0';

    if(strncmp(pending_msg, "null", 4) != 0)
    {
        if(stampa == 1)
            printf("\n***MESSAGGI DA: %s***\n%s\n", source, pending_msg);
        salva_chat(source, username, pending_msg, 0);
    }
    else
        printf("\nnessun messaggio pendente da %s\n", source);

}

void* ascolto_server(void* arg);
void* lascio_msg_segreteria(void* arg);
void p2p(int srv);

// invia messaggi a dest
void protocollo_msg_dev(char* dest)
{
    int sd;
    int porta_dest;

    sd = sd_srv;

    // dico al server cosa ho intenzione di fare
    invia_int(sd, 10);

    // aspetto che abbia capito
    ricevi_int(sd);

    // invio il mio username
    invia_stringa(sd, username);

    // aspetto che abbia capito
    ricevi_int(sd);

    // invio username del destinatario
    invia_stringa(sd, dest);

    // ricevo la porta, 0 oppure 1
    porta_dest = ricevi_int(sd);

    if(porta_dest == 0){
        printf("il destinatario '%s' non esiste...\n", dest);
        return;
    }

    // destinatario esiste
    strncpy(user_info[primo_user].nome, dest, strlen(dest));
    user_info[primo_user].porta = porta_dest;
    // prima di avviare la chat salvo i messaggi pendenti
    protocollo_segreteria_dev_show(dest, 0);

    if(porta_dest == 1){
        // il destinatario è offline
        // lascio i messaggi al server
        printf("il destinatario è offline\n");

        sleep(1);
        system("clear");
        printf("chat con %s:\n", dest);
        carica_chat(dest, username);   

        // lascio questo thread in ascolto sul server 
        // che mi avverte se dest diventa onlin
        pthread_create(&th_ascolto_server, NULL, ascolto_server, NULL);
        // questo thread invece invia i messaggi da lasciare in
        // segreteria a dest sul server
        pthread_create(&th_invio_msg, NULL, lascio_msg_segreteria, NULL);

        while(dest_online == 0 && fine_msg_pendenti == 0)
            ;

        pthread_cancel(th_ascolto_server);
        pthread_cancel(th_invio_msg);

        // sono uscito dal while perchè dest è diventato online
        if(dest_online == 1)
        {
            p2p(1);
        }
        // sono uscito dal while perchè ho finito di inviare msg pendenti
        if(fine_msg_pendenti == 1)
        {
            system("clear");
            mostra_comandi();
        }
        fine_msg_pendenti = 0;
        dest_online = 0;
    }
    else {
        // il destinatario è online
        printf("il destinatario è online, porta %d\n", (int)porta_dest);
        p2p(0);
    }
}

/************ THREADS *********************/
void invia_file();
void ricevi_file();
void aggiungi_utente();

// rimane in ascolto sul socket sd connesso al server
void* ascolto_server(void* arg)
{
    int bytes;
    int sd = sd_srv;

    bytes = ricevi_int(sd);

    if(bytes > 1)
        dest_online = 1;

    user_info[primo_user].porta = bytes;
    pthread_exit(NULL);
}

// invia i msg pendenti al server 
void* lascio_msg_segreteria(void* arg)
{
    int sd = sd_srv;
    char messaggio_inviato[MAX_MSG_LEN];

    while(1)
    {
        fgets(messaggio_inviato, MAX_MSG_LEN, stdin);
        pulisci_stringa(messaggio_inviato);
        invia_stringa(sd, messaggio_inviato);
        if(strncmp(messaggio_inviato, "\\q", 2) == 0)
            break;
        if(strncmp(messaggio_inviato, "share ", 6) == 0)
            printf("non puoi inviare un file se l'utente è offline\n");
        salva_chat(user_info[primo_user].nome, username, messaggio_inviato, 1);
    }
    fine_msg_pendenti = 1;
    pthread_exit(NULL);
}

// invia messsaggi al peer connesso al socket sd
void* invia_msg(void* arg)
{
    char msg[MAX_MSG_LEN];
    int i;

    while(1)
    {
        fgets(msg, MAX_MSG_LEN, stdin);
        pulisci_stringa(msg);
        for(i = 0; i<MAX_CHAT_GROUP_MEMBERS; i++)
            if(user_info[i].sd > 0)
                invia_stringa(user_info[i].sd, msg);

        if(strncmp(msg, "\\q", 2) == 0)
            break;
        if(strncmp(msg, "share ", 6) == 0)
            invia_file(msg);
        if(strncmp(msg, "\\u ", 2) == 0)
            aggiungi_utente();
        
        if(chat_di_gruppo == 0)
            salva_chat(user_info[primo_user].nome, username, msg, 2);
    }   
    printf("sei uscito dalla chat\n");
    chat_finita = 1;
    pthread_exit(NULL);
}
// riceve messsaggi dai peer connessi
void* ricevi_msg(void* arg)
{
    char msg[MAX_MSG_LEN];
    char nuovo_usr[USERNAME_MAX_LEN] = {0};
    int len, i, j, porta_nuovo, mittente;
    fd_set ready_sockets;
    int new_sd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // inizializzo il set col socket che ascolta e
    // quello collegato al primo user
    FD_ZERO(&sockets);
    FD_SET(sd_ascolto, &sockets);
    for(i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
        if(user_info[i].sd != -1)
            FD_SET(user_info[i].sd, &sockets);

    while(1)
    {
        // utilizzo ready_sockets come set di appoggio
        ready_sockets = sockets;

        if(select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
            chat_finita = 1;
        
        for(i = 0; i < FD_SETSIZE; i++)
        {
            if(FD_ISSET(i, &ready_sockets))
            {
                // ho una richiesta di collegamento sul socket di ascolto
                if(i == sd_ascolto)
                {
                    strcpy(nuovo_usr, "");
                    new_sd = accept(sd_ascolto,
                                    (struct sockaddr *)&address,
                                    (socklen_t *)&addrlen);   
                    // se accetto qui è una chat di gruppo
                    invia_int(new_sd, 1);  
                    // ricevo presentazione del nuovo arrivato
                    ricevi_stringa(new_sd, nuovo_usr);
                    porta_nuovo = ricevi_int(new_sd);
                    // rispondo col numero di altri membri
                    invia_int(new_sd, n_chat_group_members);
                    
                    salva_user(nuovo_usr, porta_nuovo, new_sd);
                    
                    // se l ho invitato io gli mando gli altri membri
                    if(invitato_qualcuno)
                    {
                        for(j = 0; j<MAX_CHAT_GROUP_MEMBERS; j++)
                            if(user_info[j].sd != -1 && strcmp(user_info[j].nome, nuovo_usr)!=0)
                            {
                                invia_stringa(new_sd, user_info[j].nome);
                                invia_int(new_sd, user_info[j].porta);
                            }
                        invitato_qualcuno--;
                    }
                    printf("\n\n%s(%d) join the chat!\n\n", nuovo_usr, porta_nuovo);
                    break;
                }
                // un user della chat mi ha inviato un msg
                else
                {
                    mittente = get_user_pos("", 0, i);
                    len = ricevi_stringa(i, msg);
                    pulisci_stringa(msg);
                    msg[len] = '\0';

                    // share
                    if(strncmp(msg, "share ", 6) == 0)
                        ricevi_file(i);
                    // richiesta uscita
                    if(strncmp(msg, "\\q", 2) == 0)
                    {
                        // questo user si disconnette dalla chat
                        printf("%s è uscito dalla chat\n", user_info[mittente].nome);
                        chat_finita = 2;
                        pthread_exit(NULL);
                    }
                    // aggiunta nuovo membro
                    if(strncmp(msg, "\\u", 2) == 0)
                    {
                        printf("\n%s ha invitato un nuovo membro\n\n", user_info[mittente].nome);
                        chat_di_gruppo = 1;
                        continue;
                    }
                    printf("%s)\t%s\n", user_info[mittente].nome, msg);
                    if(chat_di_gruppo == 0)
                        salva_chat(user_info[primo_user].nome, username, msg, 0);
                }
            }
        }
    }
}

/* **************** P2P *************************/

// implementa una connessione p2p con gli utenti facenti parte della chat
void p2p(int srv)
{
    int sd = -1, new_sd;
    struct sockaddr_in cl_addr;
    socklen_t len_s;
    int i, porta_nuovo, quanti_nuovi;
    char nuovo_usr[USERNAME_MAX_LEN] = {0};
    // connessione a peer

    sd_ascolto = inizializza_sd_server(porta);

    if(srv == 1)
    {
        // il peer stabilisce la connessione come server
        len_s = sizeof(cl_addr);
        sd = accept(sd_ascolto, (struct sockaddr*) &cl_addr, &len_s);

        user_info[primo_user].sd = sd;
        // dico che non è una chat di gruppo
        invia_int(sd, 0);
    }
    if(srv == 0)
    {
        // il peer stabilisce la connessione come client
        sd = cerca_connessione_con_host(user_info[primo_user].porta);
        if(sd == -1){
            printf("%s non risponde...", user_info[primo_user].nome);
            azzera_user_info();
            return;
        }

        user_info[primo_user].sd = sd;
        FD_SET(sd, &sockets);
        chat_di_gruppo = ricevi_int(sd);

        if(chat_di_gruppo == 1)
        {
            // stai entrando in una chat di gruppo, devi presentarti
            printf("mi presento alla chat di gruppo\n");
            invia_stringa(sd, username);
            invia_int(sd, porta);
            // ricevo quanti altri utenti sono nella chat
            quanti_nuovi = ricevi_int(sd); 
            // mi connetto con questi
            for(i = 0; i < quanti_nuovi; i++){

                ricevi_stringa(sd, nuovo_usr);
                porta_nuovo = ricevi_int(sd);

                new_sd = cerca_connessione_con_host(porta_nuovo);
                printf("mi connetto a %s\n",nuovo_usr);
                salva_user(nuovo_usr, porta_nuovo, new_sd);
                //stampa_user_info();
                // mi presento 
                invia_stringa(new_sd, username);
                invia_int(new_sd, porta);
                ricevi_int(new_sd);
            }
        }

    }
    n_chat_group_members++;

    //stampa_user_info();
    //getchar();
    sleep(3);
    system("clear");
    printf("chat live con %s:\n", user_info[primo_user].nome);
    carica_chat(user_info[primo_user].nome, username); 
    
    // avvio un thread per ricevere messaggi
    pthread_create(&th_ricezione, NULL, ricevi_msg, NULL);
    pthread_create(&th_invio, NULL, invia_msg, NULL);
    
    while(chat_finita == 0)
        sleep(1);
    printf("chat finita\n");

    pthread_cancel(th_ricezione);
    pthread_cancel(th_invio);

    close(sd_ascolto);
    sd = -1;
    for(i = 0; i < MAX_CHAT_GROUP_MEMBERS; i++)
        if(user_info[i].sd != -1)
            close(user_info[i].sd);
    azzera_user_info();

    chat_finita = 0;

    sleep(1);
    system("clear");
    mostra_comandi();
}

void invia_file(char* msg)
{
    char nome_file[MAX_MSG_LEN];
    char path_file[MAX_MSG_LEN+10];
    char file_content[MAX_FILE_CONTENT] = {0};
    int i;
    FILE* fd;
    char** parole;

    parole = str_split(msg, ' ');
    strcpy(nome_file, *(parole+1));
    sprintf(path_file, "./device/%s", nome_file);

    // controllo che il file esista
    fd = fopen(path_file, "r");
    if (fd == NULL){
        printf("il file non esiste...\n");
        // invio *** per dire che non esiste
        for(i = 0; i<MAX_CHAT_GROUP_MEMBERS; i++)
            if(user_info[i].sd > 0)
                invia_stringa(user_info[i].sd, "***");
        return;
    }

    printf("invio %s\n", nome_file);

    // invio nome
    for(i = 0; i<MAX_CHAT_GROUP_MEMBERS; i++)
            if(user_info[i].sd > 0)
                invia_stringa(user_info[i].sd, nome_file);

    printf("preparo il contenuto da inviare...\n");
    get_file_content(path_file, file_content);

    for(i = 0; i<MAX_CHAT_GROUP_MEMBERS; i++)
            if(user_info[i].sd > 0)
                invia_stringa(user_info[i].sd, file_content);
    printf("il file %s è stato inviato con successo\n", nome_file);
}

void ricevi_file(int sd)
{
    char nome_file[MAX_MSG_LEN];
    char path_file[MAX_MSG_LEN+20];
    char file_content[MAX_FILE_CONTENT] = {0};

    printf("ricevo un file\n");

    ricevi_stringa(sd, nome_file);
    if(strncmp(nome_file, "***", 3) == 0){
        printf("il file che dovevi ricevere non esiste...\n");
        return;
    }
    printf("pronto per ricevere: %s\n", nome_file);

    sprintf(path_file, "./device/%s-%s", username, nome_file);

    ricevi_stringa(sd, file_content);
    printf("copia del file in corso...\n");

    put_file_content(path_file, file_content);
    printf("file disponibile: %s\n", path_file);

}

void aggiungi_utente()
{
    int sd;
    char lista_attivi[200] = {0};
    char scelta[USERNAME_MAX_LEN];
    int porta_scelta;

    printf("aggiungo un utente\n\n");

    sd_srv = connessione_con_host(SERVER_PORT);
    sd = sd_srv;

    invia_int(sd, 20);

    ricevi_stringa(sd, lista_attivi);
    printf("\nscegli l'utente da aggiungere tra:\n%s\n", lista_attivi);

    fgets(scelta, USERNAME_MAX_LEN, stdin);
    scelta[strlen(scelta)-1] = '\0';

    invia_stringa(sd, scelta);

    porta_scelta = ricevi_int(sd);
    if(porta_scelta == 0){
        printf("utente scelto non valido...\n");
        return;
    }

    invia_stringa(sd, username);
    invitato_qualcuno++;
    chat_di_gruppo = 1;

    printf("invito inoltrato\n\n");
    
    close(sd_srv);
}
