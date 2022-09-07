# Progetto del corso di Reti Informatiche A.A. 2021/2022

## Univeristà di Pisa, Ingegneria Informatica

Applicazione distribuita basata su paradigma ibrido peer-to-peer e client-server che implementi una moderna applicazione di instant messaging sviluppata interamente in linguaggio C in ambiente Linux.

Per avviare l'applicazione:
        bash chaTT.sh

Per ricompilarla da zero
        make clean
        bash server/azzera.sh
        bash elimina_chat.sh
        bash chaTT.sh

### Protocollo di Avviso

Fa parte del protocollo di avviso tutto ciò che è inerente alla comunicazione tra il device e il server in fase di avvio e chiusura dell'applicazione lato device (login, logout e signup).

### Protocollo Segreteria

Fanno parte del protocollo di segreteria tutte le interazioni tra server e device dei comandi hanging e show lato device.

### Protocollo Chat

Fanno parte del protocollo chat tutte le interazioni tra server e device, device e device svolte al fine di inviare o ricevere messaggi.

### Protocollo Share

Protocollo per la condivisione di file peer to peer tra 2 o più device 

### Scelte progettuali

Tutti i socket utilizzati all'interno dell'applicazione sono di tipo TCP bloccante, questo perchè essendo una chat non si può prescindere dall'affidabilità del trasporto dei messaggi.
L'applicazione è strettamente dipendente dal server, infatti la maggior parte dei protocolli necessita connessioni col server, soltanto la chat live tra 2 o più device è strettamente peer to peer, questo comporta che il server debba essere sempre online e anche un single point of failure ma non potrebbe essere altrimenti dato che i device stessi non possono garantire di essere sempre online e quindi renderebbero troppo complesso un approccio peer to peer puro.
Il device si connette al server soltanto quando necessario, la connessione quindi è persistente soltanto per la durata di un protocollo, questo per non appesantire il server di processi inutili (quando un utente è sulla home dell'app non ha senso dedicare un processo server a quello).