
#include "./lib/all.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{

    int ret, sd;
    uint16_t lmsg;    
    socklen_t len; 

    struct sockaddr_in srv_addr;
    char buffer[BUFFER_SIZE];

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);

    /* Creazione indirizzo del server */
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(-1);
    }
    printf("connessione effettuata");
    
    //while(1){

        // Attendo input da tastiera
        // scanf("%s", buffer); scanf recupera solo una parola
        //fgets(buffer, BUFFER_SIZE, stdin);

        // Calcolo la dimensione del messaggio (compreso '\0')
        //len = strlen(buffer) + 1;

        // Gestione endianness (converto in 'network order')
        lmsg = htons(1);

        // Invio al server la dimensione del messaggio
        ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);

        // Invio il messaggio
        //ret = send(sd, (void*) buffer, len, 0);
    //}

    // Chiusura del socket connesso
    close(sd);

}