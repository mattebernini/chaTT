
// gestione files
void printfile(char* percorso);
void salva_iscrizione(char* username, char* password, int porta_dev);
void registra_logout(char* username, int porta);
void registra_login(char* username, int porta);
void print_online();
int porta_utente_online(char* dest);
void salva_msg_pendente(char* source, char* dest, char* pending_msg);
void leggi_segreteria(char* username, char* ris);
void msg_pendenti(char* dest, char* mittente, char* ris);

// stringhe
char** str_split(char* a_str, const char a_delim);
int controllo_username(char* username, int len, int tipo);
int controllo_password(char* username, char* password, int lmsg, int tipo);
