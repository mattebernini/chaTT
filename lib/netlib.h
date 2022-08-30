int inizializza_sd_server(int porta);
int connessione_con_host(int porta);
int cerca_connessione_con_host(int porta);

int ricevi_int(int sd);
void invia_int(int sd, int bytes);

void invia_stringa(int sd, char* str);
int ricevi_stringa(int sd, char* str);
