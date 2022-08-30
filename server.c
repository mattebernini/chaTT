#include "./lib/all.h"

/*
********************** MAIN ***********************
*/
void avvia_server(int porta);
void frontend();

int main(int argc, char* argv[])
{
    int porta;
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
        set_tutti_logout();
        set_tutti_offchat();
        avvia_server(porta);
    }
    else{
        // padre per l'interazione con l'utente
        frontend();
    }
}
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
void mostra_comandi_server()
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
    print_list();
}

// processo di interazione con l'utente
void frontend()
{
    char comando[COMANDO_MAX_LEN];
    enum comandi_server cmd;

    mostra_comandi_server();

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
void protocollo_avviso_server(int sd, int avviso);
void protocollo_msg_server(int sd);
void protocollo_segreteria_server(int sd, int tipo);
void protocollo_group_chat(int sd);
void uscita_chat_serv(int sd);

// processo server vero e proprio 
void avvia_server(int porta)
{
    int sd, new_sd;
    struct sockaddr_in cl_addr;
    socklen_t len; 
    pid_t pid;
    uint16_t bytes;

    sd = inizializza_sd_server(porta);
    // pthread_create(&th_routine, NULL, routine, NULL);

    while(1)
    {
        len = sizeof(cl_addr);
        new_sd = accept(sd, (struct sockaddr*) &cl_addr, &len);

        pid = fork();

        // processo figlio gestisce scambio di dati
        if(pid == 0)
        {   
            close(sd);

            bytes = ricevi_int(new_sd);
            
            switch(bytes)
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
                case 20:
                    protocollo_group_chat(new_sd);
                    break;
                case 30:
                    uscita_chat_serv(new_sd);
                    break;
                case 100:
                    protocollo_segreteria_server(new_sd, 1);
                    break;
                case 200:
                    protocollo_segreteria_server(new_sd, 2);
                    break;
                default:
                    break;
            }
            close(new_sd);
            exit(0);
        }
        else {
            // processo padre, torna ad ascoltare
            close(new_sd);
        }
    }
}

// implementa prot. avviso in base alla variabile avviso:
// avviso = 1(signup), 2(login), 3(logout)
void protocollo_avviso_server(int sd, int avviso)
{
    int bytes;
    char username[USERNAME_MAX_LEN];
    char password[PASSWORD_MAX_LEN];
    int porta_dev;
    int len;

    // dico al device che ho capito 
    invia_int(sd, 1);

    // device mi manda il suo username
    len = ricevi_stringa(sd, username);

    // mando 0 se tutto ok, 1 altrimenti
    bytes = controllo_username(username, avviso);
    invia_int(sd, bytes);
    if(bytes==1)
        return;

    // per il logout non ho bisogno ne della password ne della porta
    if(avviso == 3){
        registra_logout(username);
        set_onchat(username, "0");
        close(sd);
        exit(0);
    }
    /* LOGOUT TERMINATO */

    // device mi manda la sua password
    len = ricevi_stringa(sd, password);
    password[len] = '0';
    // mando 0 se tutto ok, 1 altrimenti
    bytes = controllo_password(username, password, len, avviso);
    invia_int(sd, bytes);
    if(bytes==1)
        return;

    // ricevo porta e dico che ho capito
    porta_dev = ricevi_int(sd);
    invia_int(sd, 0);

    if(avviso == 1){
        salva_iscrizione(username, password, porta_dev);
    }
    /* ISCRIZIONE TERMINATA */
    if(avviso == 2)
        registra_login(username, porta_dev);
    /* LOGIN TERMINATO */
}

// implementa protocollo di segreteria in base al tipo
// tipo 1 -> hanging, 2 show
void protocollo_segreteria_server(int sd, int tipo)
{
    char username[USERNAME_MAX_LEN];
    char source[USERNAME_MAX_LEN];
    char str[500] = {0};

    // dico al device che ho capito 
    invia_int(sd, 1);

    // device mi manda il suo username
    ricevi_stringa(sd, username);

    if(tipo == 1)
    {
        // hanging
        leggi_segreteria(username, str);
        invia_stringa(sd, str);
    }
    else if(tipo == 2)
    {
        // show

        // dico al device di prosguire
        invia_int(sd, 1);

        // ricevo username dell'utente del quale device vuole 
        // leggere i messaggi
        ricevi_stringa(sd, source);

        // gli rispondo con i messaggi pendenti
        msg_pendenti(username, source, str);
        if(strlen(str) == 0)
            invia_stringa(sd, "null");
        else
            invia_stringa(sd, str);
    }
}


// fa lasciare in segreteria i messaggi a device oppure lo fa collegare 
// direttamente col destinatario
void protocollo_msg_server(int sd)
{
    char destinatario[USERNAME_MAX_LEN];
    char mittente[USERNAME_MAX_LEN];
    char pending_msg[MAX_MSG_LEN];
    int porta_dest;
    int len;
    pid_t pid;

    // dico al device che ho capito 
    invia_int(sd, 1);

    // device mi manda il suo username
    len = ricevi_stringa(sd, mittente);
    mittente[len] = '\0';
    
    // dico al device che ho capito 
    invia_int(sd, 1);

    // device mi manda username destinatario
    ricevi_stringa(sd, destinatario);

    set_onchat(mittente, destinatario);

    // se dest è onchat mando la porta, 
    // se non esiste 0, se offchat 1
    porta_dest = (offchat(destinatario, mittente)==1)? 
                                1 : porta_utente(destinatario);
    invia_int(sd, porta_dest);

    // dest non esiste
    if(porta_dest == 0)
        return;

    // prima di avviare la chat invio msg pendenti
    ricevi_int(sd);
    protocollo_segreteria_server(sd, 2);

    // il destinatario è offchat, salvo i messaggi 
    if(porta_dest == 1)
    {
        pid = fork();

        if(pid == 0)
        {
            // controllo se dest diventa onchat
            while(1)
            {
                sleep(1);
                porta_dest = (offchat(destinatario, mittente)==1)? 
                                1 : porta_utente(destinatario);
                if(porta_dest != 1)
                    break;
            }
            // se dest diventa onchat glie lo dico
            invia_int(sd, porta_dest);
            // se è diventato onchat termino processo padre
            set_onchat(mittente, "0");
            kill(getppid(), SIGKILL);
        }
        else
        {
            // ricevo i messaggi pendenti 
            while(1){
                len = ricevi_stringa(sd, pending_msg);
                pulisci_stringa(pending_msg);
                pending_msg[len] = '\0';
                if(strncmp(pending_msg, "\\q", 2) == 0){
                    set_onchat(mittente, "0");
                    break;
                }
                salva_msg_pendente(mittente, destinatario, pending_msg);
            }
            // se ha finito di inviarmi messaggi termino il processo
            // che controllava se dest diventa onchat
            kill(pid, SIGKILL);
        }
    }
    else
    {
        // destinatario onchat
        sleep(1);
        set_onchat(mittente, "0");
    }    
}

void protocollo_group_chat(int sd)
{
    char lista_online[200] = {0};
    char scelta[USERNAME_MAX_LEN];
    char source[USERNAME_MAX_LEN];
    int porta_scelta;
    int len;

    // mando lista utenti attivi (online)
    get_utenti_online(lista_online);
    invia_stringa(sd, lista_online);

    len = ricevi_stringa(sd, scelta);
    scelta[len] = '\0';
    porta_scelta = porta_utente(scelta);

    invia_int(sd, porta_scelta);
    if(porta_scelta == 0)
        return;
    
    ricevi_stringa(sd, source);

    set_onchat(source, scelta);
}

void uscita_chat_serv(int sd)
{
    char utente_uscito[USERNAME_MAX_LEN];

    invia_int(sd, 1);

    ricevi_stringa(sd, utente_uscito);
    set_onchat(utente_uscito, "0");
}
