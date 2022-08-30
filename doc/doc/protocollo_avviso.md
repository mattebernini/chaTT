protocollo binary usato per comunicare login, logout e iscrizione al server
binary perchè senno le password sono in chiaro

- dev: invia 1, 2, 3 a seconda del messaggio (iscrizione, login, logout)
- serv: riceve il byte e sa cosa aspettarsi dopo, invia ak
- dev: invia lo username (massimo 20 caratteri)
- serv: invia 0 se è andato tutto bene, 1 se qualcosa è andato storto 
- dev: se riceve 1 da errore e termina (se sta facendo il logout non da errore), altrimenti invia la password (max 8 char)
- serv: invia 1 se la password va bene, 2 altrimenti
- dev: invia la porta a cui è disponibile
- serv: invia 1 se tutto va bene, 0 altirmenti

 
************************************************************************
Binary Protocol: invio
…
struct temp{
uint32_t a;
uint8_t b;
};
struct temp t;
…
// Convertire in network order prima dell’invio
t.a = htonl(t.a);
// Spedire i campi sul socket ‘new_sd’
ret = send(new_sd, (void*)&t.a, sizeof(uint32_t), 0);
…
ret = send(new_sd, (void*)&t.b, sizeof(uint8_t), 0);
…
************************************************************************
Binary Protocol: ricezione
…
struct temp t;
…
ret = recv(new_sd, (void *)&t.a, sizeof(uint32_t), 0);
if (ret < sizeof(uint32_t)){
// Gestione errore
}
// Convertire in host order il campo ‘a’
t.a = ntohl(t.a);
ret = recv(new_sd, (void *)&t.b, sizeof(uint8_t), 0);
if (ret < sizeof(uint8_t)){
// Gestione errore
}
…
************************************************************************



