device per iniziare una conversazione deve passare dal server

- dev: manda al server il valore 10 per fargli capire che vuole iniziare una conversazione
- serv: manda un ak di avvenuta ricezione
- dev: manda lo username del destinatario
- serv: risponde con la porta (se dest è online) oppure con 0.

se dest è online dev ci si connette:
- prima però dev: manda ak al server
dest e dev si comportano sia da client che da server ricevendo ed inviando messaggi fino a che uno dei due non si disconnette oppure uno dei due termina la conversazione.
se l'altro vuole continuare ad inviare msg passa alla modalità offline.
i messaggi sono inviati con MSG_WAITALL e fanno uso di un timeout

se dest è offline:
- dev: manda il valore 20
- serv: manda un ak
- dev: manda source, dest e msg pendente
- serv: manda un ak
ciclicamente fino a che dev non manda 30 in source

***************************************************************

connessione p2p:
- source: manda il suo username al dest
- dest: se accetta la conversazione invia 0 altrimenti 1
- source e dest iniziano ad inviarsi messaggi