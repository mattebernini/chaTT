1 server
n device
1 utente per ogni device

comunicare con gli altri utenti e condividere file

funzionalità:
- signup
- login e logout
- invio di messaggi online
- invio di messaggi offline
- chat di gruppo
- invio di file diretto o di gruppo

protocolli:
- iscrizione, login, logout
- scambio messaggi
- invio di file

server:
ad ogni cambio di stato dei device si passa dal server perchè
- login, logout, signup chiediamo al server
- in caso di disconnessione catastrofica il server se ne accorge perchè device non risponde
server concorrente

***************************************
Supporre che i device possano disconnettersi improvvisamente senza effettuare out. Spiegare brevemente
nella documentazione la politica utilizzata per gestire le disconnessioni improvvise e le conseguenze che
comportano.
***************************************
in caso di mancato logout (server disconnesso) il server si accorge sempre che l'utente non è online perchè questo non gli risponde.
in fase di accensione del server potrei fargli controllare se gli utenti sono sempre online.


*online* = connesso al server oppure connesso ad altri devices

./serv -> due processi
- frontend: permette di eseguire 3 comandi (help, list, esc) a chi ha avviato e gestisce il server
- server: server concorrente che accetta richieste e le fa gestire da processi figli

./dev -> due processi
- processo principale: getsisce login, logout e le interazioni client
- processo server (p2p): viene creato al login per ricevere messaggi dai membri della chat ed instaurare connessioni