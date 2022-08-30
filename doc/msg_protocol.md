un utente (source) che vuole inviare messaggi ad un'altro utente deve digitare il comando 
        chat dest
dove dest è lo username dell'utente al quale si vuole inviare il messaggio.

- source: invia 10 
- server: invia 1 (ack)
- source: invia il suo username
- server: invia 1 (ack)
- source: invia username dest
- server: invia la porta di dest se onchat, 1 se offchat, 0 se dest non esiste

Qualora dest non fosse sulla chat di source (*dest offchat*) i messaggi sarannò recapitati dal server che li salverà come msg pendenti.

- source: invia msg pendente
- server: salva il messaggio pendente
(fino a quando source non invia '\q' o il server avvisa source che dest è andato onchat)

Nel caso in cui dest sia sulla chat di source (*dest onchat*) a lasciare msg pendenti invece verrà iniziata una chat live dai due utenti.

- source: si disconnette dal srv e si comporta da server p2p
- dest: si disconnette dal srv e si comporta da client p2p
effettuano una connessione tra loro
- source: invia 0 (chat singola per ora) 
iniziano a scambiarsi messaggi

Se uno dei due durante la chat live aggiunge un utente col comando '\u' (mettiamo lo faccia dest)

- dest: si connette con srv e invia 20
- server: invia lista utenti online
- dest: invia username dell'utente da aggiungere
- server: invia la porta di tale username
- dest: invia il proprio username
- server: mette dest onchat del nuovo utente

quando il nuovo utente (user3) entra nella chat 
        chat dest

- user3: si comporta da client p2p con dest e si connette
- dest: si comporta da server p2p e si connette a user3
- dest: invia 1 (chat di gruppo)
- user3: invia username e porta
- dest: invia il numero di altri utenti nella chat e i loro username e porta
- user3: riceve username e porta degli altri partecipanti e si collega con loro
- source: si comporta da server p2p e si connette a user3
- user3: invia username e porta
- source: invia i suoi username e porta, il numero di altri utenti nella chat e i loro username e porta 