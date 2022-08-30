
// gestione files
void printfile(char* percorso);
// stringhe
char** str_split(char* a_str, const char a_delim);
int controllo_username(char* username, int tipo);
int controllo_password(char* username, char* password, int lmsg, int tipo);
void pulisci_stringa(char* str);


// server
void salva_iscrizione(char* username, char* password, int porta_dev);
void registra_logout(char* username);
void registra_login(char* username, int porta);
void set_tutti_offchat();
void set_tutti_logout();
void print_list();
int porta_utente(char* dest);
void get_utenti_online(char* str);

void salva_msg_pendente(char* source, char* dest, char* pending_msg);
void leggi_segreteria(char* username, char* ris);
void msg_pendenti(char* dest, char* mittente, char* ris);

int offchat(char* source, char* dest);
int set_onchat(char* source, char* dest);

// device
enum comandi_device quale_comando_dev(char* comando);
void mostra_comandi();

void salva_chat(char* other, char* owner, char* msg, int asterischi);
void carica_chat(char* other, char* owner);
void get_file_content(char* path, char* ris);
void put_file_content(char* path, char* content);