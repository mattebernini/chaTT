#include "./lib/all.h"

/*
********************** FUNZIONI FRONTEND ***********************
*/
// data una stringa restituisce il comando corrispondente
enum comandi_server quale_comando_serv(char* comando)
{
    if(strncmp(comando, "help", 4)==0)
        return help;
    if(strncmp(comando, "list", 4)==0)
        return list;
    if(strncmp(comando, "esc ", 3)==0)   
        return esc ;
    return invalid;
}

// mostra a schermo il menu dei comandi leggendolo dal file menu.txt
void mostra_comandi()
{
    char str[50] = "./server/menu.txt";
    printfile(str);
}

// Mostra una breve descrizione dei comandi.
void help_cmd()
{
    char str[50] = "./server/help.md";
    printfile(str);
}
// Mostra l’elenco degli utenti connessi alla rete, indicando  
// username, timestamp di connessione e numero di
// porta nel formato “username*timestamp*porta”.
void list_cmd()
{
    print_online();
}

// processo di interazione con l'utente
void frontend(char* comando, enum comandi_server cmd)
{
    mostra_comandi();

    // ciclo infinito che prende in input un comando ed esegue
    // l'azione corrispondente
    while(1)
    {
        fgets(comando, COMANDO_MAX_LEN, stdin);
        printf("\n");
        cmd = quale_comando_serv(comando);

        switch (cmd)
        {
            case help:
                help_cmd();
                break;
            case list:
                list_cmd();
                break;
            case esc:
                // uccido il processo figlio ed esco dal programma
                kill(0, SIGKILL);
                exit(0);
                break;
            default:
                printf("comando non valido...\n\n");
                break;
        } 
    }
}


/*
********************** FUNZIONI SERVER ***********************
*/

// avviso = 1(signup), 2(login), 3(logout)
void protocollo_avviso_server(int new_sd, int avviso)
{
    int ret;
    uint32_t lmsg;
    char* username;
    char* password;
    uint32_t porta_dev;
    // variabile che contiene l'inizio del protocollo di avviso
    // e le risposte di ak o no ak del server
    uint16_t byte_ak;

    // mando l'ak al device
    byte_ak = 1;
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto di ricevere la lunghezza dello username e 
    ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    username = (char*) malloc((lmsg+1)*sizeof(char));
    // subito dopo lo username
    ret = recv(new_sd, (void *)username, lmsg, 0);
    username[lmsg] = '\0';

    // mando la risposta, 0 se va tutto bene, 1 se c'è stato un problema
    byte_ak = controllo_username(username, lmsg, avviso);
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);
    if(byte_ak==1)
        return;

    // per il logout non ho bisogno ne della password ne della porta
    if(avviso == 3){
        registra_logout(username, porta_dev);
        return;
    }
    /* LOGOUT TERMINATO */

    // aspetto di ricevere lunghezza password
    ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    password = (char*) malloc((lmsg+1)*sizeof(char));
    // subito dopo la password
    ret = recv(new_sd, (void *)password, lmsg, 0);
    password[lmsg] = '\0';

    // mando la risposta, 0 se va tutto bene, 1 se c'è stato un problema
    byte_ak = controllo_password(username, password, lmsg, avviso);
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);
    if(byte_ak==1)
        return;

    // aspetto di ricevere la porta
    ret = recv(new_sd, (void *)&porta_dev, sizeof(uint16_t), 0);
    porta_dev = ntohs(porta_dev);

    // manda byte di ak = 0
    byte_ak = 0;
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    if(avviso == 1){
        salva_iscrizione(username, password, porta_dev);
    }
    /* ISCRIZIONE TERMINATA */
    if(avviso == 2)
        registra_login(username, porta_dev);
    /* LOGIN TERMINATO */
}

// manda al source la porta del dest a cui vuole inviare un msg
void protocollo_msg_server(int new_sd)
{
    int ret;
    uint32_t lmsg;
    char* dest;
    char* source;
    char* password;
    uint32_t porta_dev, porta_dest;
    // variabile che contiene l'inizio del protocollo di avviso
    // e le risposte di ak o no ak del server
    uint16_t byte_ak;
    char* pending_msg;

    // mando l'ak al device
    byte_ak = 1;
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto di ricevere la lunghezza dello username del dest 
    ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    dest = (char*) malloc((lmsg+1)*sizeof(char));
    // subito dopo lo username
    ret = recv(new_sd, (void *)dest, lmsg, 0);
    dest[lmsg-1] = '\0';

    // cerco la porta di dest (0 se non è online) e la invio
    porta_dest = porta_utente_online(dest);
    porta_dest = htons(porta_dest);
    ret = send(new_sd, (void*)&porta_dest, sizeof(uint16_t), 0);

    // aspetto il suo ak
    ret = recv(new_sd, (void *)&byte_ak, sizeof(uint16_t), 0);
    byte_ak = ntohs(byte_ak);

    // il destinatario è offline, salvo i messaggi 
    if(porta_dest==0){
        // ricevo source username
        ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
        lmsg = ntohs(lmsg);
        source = (char*) malloc((lmsg+1)*sizeof(char));
        ret = recv(new_sd, (void *)source, lmsg, 0);
        source[lmsg] = '\0';

        while(1){
            // aspetto di ricevere la lunghezza del msg
            ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
            lmsg = ntohs(lmsg);
            pending_msg = (char*) malloc((lmsg+1)*sizeof(char));
            // subito dopo il msg
            ret = recv(new_sd, (void *)pending_msg, lmsg, 0);
            if(strncmp(pending_msg, "\\q", 2) == 0)
                break;
            pending_msg[lmsg] = '\0';
            salva_msg_pendente(source, dest, pending_msg);
        }
    }
}
// tipo 1 -> hanging, 2 show
void protocolo_segreteria_server(int new_sd, int tipo)
{
    int ret;
    uint32_t lmsg;
    char* username;
    char* mittente;
    socklen_t len; 

    uint16_t byte_ak;
    char str[500];

    // mando l'ak al device
    byte_ak = 1;
    byte_ak = htons(byte_ak);
    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);

    // aspetto di ricevere la lunghezza dello username 
    ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
    lmsg = ntohs(lmsg);
    username = (char*) malloc((lmsg+1)*sizeof(char));
    // subito dopo lo username
    ret = recv(new_sd, (void *)username, lmsg, 0);
    username[lmsg] = '\0';

    if(tipo==1)
    {
        leggi_segreteria(username, str);

        // invio gli utenti che hanno lasciato un msg in segreteria
        len = strlen(str);
        lmsg = htons(len);
        ret = send(new_sd, (void*) &lmsg, sizeof(uint16_t), 0);
        ret = send(new_sd, (void*) str, len, 0);
    }
    else
    {
        // mando l'ak al device
        byte_ak = 1;
        byte_ak = htons(byte_ak);
        ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);

        // aspetto di ricevere la lunghezza dello username del mittente 
        ret = recv(new_sd, (void *)&lmsg, sizeof(uint16_t), 0);
        lmsg = ntohs(lmsg);
        mittente = (char*) malloc((lmsg+1)*sizeof(char));
        // subito dopo lo username
        ret = recv(new_sd, (void *)mittente, lmsg, 0);
        mittente[lmsg] = '\0';

        // invio i msg pendenti
        msg_pendenti(username, mittente, str);

        // invio gli utenti che hanno lasciato un msg in segreteria
        len = strlen(str);
        lmsg = htons(len);
        ret = send(new_sd, (void*) &lmsg, sizeof(uint16_t), 0);
        ret = send(new_sd, (void*) str, len, 0);

    }
}

// processo server vero e proprio 
void avvia_server(int porta)
{
    int ret, sd, new_sd;
    socklen_t len; 
    uint16_t lmsg;
    pid_t pid;
    struct sockaddr_in my_addr, cl_addr;
    // variabile che contiene l'inizio del protocollo di avviso
    // e le risposte di ak o no ak del server
    uint16_t byte_ak;

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(porta);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    /* Aggancio del socket */
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    ret = listen(sd, 10);
    if(ret < 0){
        perror("Errore in fase di bind: \n");
        exit(-1);
    }
    while(1)
    {
        len = sizeof(cl_addr);
        // Accetto nuove connessioni
        new_sd = accept(sd, (struct sockaddr*) &cl_addr, &len);
        // Creazione child process
        pid = fork();

        if( pid == 0 )
        {
            // figlio che gestisce scambio dati
            // Chiusura del listening socket 
            close(sd);

            //while(1)
            //{
                // ricevo byte che mi dice quale protocollo iniziare
                // 1, 2, 3 -> protocollo di avviso
                // 10 -> richiesta di inizio conversazione
                ret = recv(new_sd, (void *)&byte_ak, sizeof(uint16_t), MSG_WAITALL);
                if (ret < sizeof(uint16_t)){
                    perror("errore ricezione byte di avviso");
                    break;
                }
                byte_ak = ntohs(byte_ak);
                switch (byte_ak)
                {
                case 1:
                    protocollo_avviso_server(new_sd, 1);
                    break;
                case 2:
                    protocollo_avviso_server(new_sd, 2);
                    break;                
                case 3:
                    protocollo_avviso_server(new_sd, 3);
                    break;                
                case 10:
                    protocollo_msg_server(new_sd);
                    break;
                case 100:
                    protocolo_segreteria_server(new_sd, 1);
                    break;
                case 200:
                    protocolo_segreteria_server(new_sd, 2);
                    break;
                default:
                    byte_ak = 0;
                    byte_ak = htons(byte_ak);
                    ret = send(new_sd, (void*)&byte_ak, sizeof(uint16_t), 0);
                    break;
                }
            //}

            close(new_sd);
            exit(1);
        }
        else {
            // processo padre, torna ad ascoltare
            close(new_sd);
        }
    }
}

/*
********************** MAIN ***********************
*/
int main(int argc, char* argv[])
{
    int porta;
    char comando[COMANDO_MAX_LEN];
    enum comandi_server cmd;
    pid_t pid;

    // assegno il valore alla porta su cui comunica il server
    if(argv[1] == NULL)
        porta = 4242;
    else
        porta = atoi(argv[1]);

    pid = fork();
    // da ora in poi avrò due processi:
    if(pid == 0){
        // figlio che funge da server vero e proprio
        avvia_server(porta);
    }
    else{
        // padre per l'interazione con l'utente
        frontend(comando, cmd);
    }
}