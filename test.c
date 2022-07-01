#include "./lib/all.h"

int main()
{
    char username[10];
    char password[10];
    char str1[150], str2[150];
    pid_t pid, child_pid;

    strcpy(username, "andre");
    strcpy(password, "abc");

    salva_msg_pendente("matte", username, "cacca culo");
    salva_msg_pendente("matte", username, "cacca merda");
    salva_msg_pendente("matte", username, "cacca cazzo");
    
    leggi_segreteria(username, str1);
    printf("%s", str1);

    msg_pendenti(username, "matte", str2);
    printf("%s", str2);

    return 0;
}
