#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "costanti.h"

// variabile di debugging (se a 1 mostra printf)
int print_info = 0;

// inizializza un server socket su una data porta fino alla listen()
int inizializza_sd_server(int porta)
{
    int ret, sd, new_sd;
    uint16_t lmsg;
    struct sockaddr_in my_addr;

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(porta);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    /* Aggancio del socket */
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    if(ret < 0){
        perror("Errore in fase di bind: \n");
        return -1;
    }
    ret = listen(sd, MAX_CONNECTIONS);
    if(ret < 0){
        perror("Errore in fase di listen: \n");
        return -1;
    }

    return sd;
}

int connessione_con_host(int porta)
{
    int ret, sd;
    struct sockaddr_in srv_addr;
    
    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(porta);
    inet_pton(AF_INET, LOCALHOST, &srv_addr.sin_addr);

    /* Connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
        //perror("Errore in fase di connessione: \n");
        return -1;
    }
    if(print_info == 1)
        printf("connessione a %d effettuata\n", porta);

    return sd;
}

int cerca_connessione_con_host(int porta)
{
    int sd = -1;
    int count = 0;

    while(sd == -1){
        // cerco di connettermi
        sd = connessione_con_host(porta);
        sleep(1);
        count++;
        if(count > 10)
            return -1;
    }
    return sd;
}

int ricevi_int(int sd)
{
    int ret;
    uint32_t ris;

    ret = recv(sd, (void *)&ris, sizeof(uint32_t), 0);
    if (ret < sizeof(uint32_t)){
        // perror("errore ricezione bytes");
        return -1;
    }
    ris = ntohl(ris);
    if(print_info == 1)
        printf("(%d)(%d) ricevuto %d\n", getpid(), sd, ris);
    return (int) ris;
}

void invia_int(int sd, int bytes)
{
    int ret;
    uint32_t x;

    x = (u_int32_t) bytes;
    x = htonl(x);
    ret = send(sd, (void*)&x, sizeof(uint32_t), 0);
    if (ret < sizeof(uint32_t)){
        perror("errore invio bytes");
    }
    x = ntohl(x);
    if(print_info == 1) 
        printf("(%d)(%d) inviato %d\n", getpid(), sd, x);
}

int ricevi_stringa(int sd, char* str)
{
    int ret;
    uint32_t lmsg;

    // ricezione lunghezza
    ret = recv(sd, (void *)&lmsg, sizeof(uint32_t), 0);
    lmsg = ntohs(lmsg);
    //str = (char*) malloc((lmsg+1)*sizeof(char));
    // ricezione stringa
    ret = recv(sd, (void *)str, lmsg, 0);
    if(print_info == 1)
        printf("(%d)(%d) ricevuto %s di dim %d\n", getpid(), sd, str, lmsg);
    return lmsg;
}

void invia_stringa(int sd, char* str)
{
    int ret;
    uint32_t lmsg;    
    socklen_t len; 

    len = strlen(str);
    lmsg = htons(len);
    ret = send(sd, (void*) &lmsg, sizeof(uint32_t), 0);
    ret = send(sd, (void*) str, len, 0);
    if(print_info == 1)
        printf("(%d)(%d) invio %s di dim  %d\n", getpid(), sd, str, len);
}