// lunghezze
#define COMANDO_MAX_LEN 50
#define USERNAME_MAX_LEN 20
#define PASSWORD_MAX_LEN 8
#define MAX_CHAT_GROUP_MEMBERS 4
#define MAX_MSG_LEN 120
#define MAX_USERS 10
#define MAX_PENDING_MSG 10
#define MAX_FILE_CONTENT 1200
#define MAX_CONNECTIONS 10

// strutture
enum comandi_server {help, list, esc, invalid};
enum comandi_device {signup, in, hanging, show, chat, share, out, nonvalid};

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 4242

struct user
{
    char nome[USERNAME_MAX_LEN];
    int porta;
    int sd;
};
