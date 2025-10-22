#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "costantiServer.h"

int main(int argc, char *argv[]){
    int ret, newfd, port, i;
    struct sockaddr_in my_addr, cl_addr;
    socklen_t len;

    if(argc > 1){
        port = atoi(argv[1]);
    }else{
        port = DEFAULT_PORT;
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(ret < 0){
        perror("Errore in fase di bind: \n");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listener,&master);
    FD_SET(STDIN_FILENO,&master);
    fdmax = listener;

    guida_comandi(port); //stampa delle operazioni possibili, start o stop
    gestione_comandi_server();//lettura da stdin

    if(server_status == 0){
        close(listener);
        return 0;
    }

    ret = listen(listener, BACKLOG);
    if(ret < 0){
        perror("Errore in fase di listen: \n");
        exit(EXIT_FAILURE);
    }

   while(server_status == 1){

        read_fds = master;
        ret = select(fdmax+1,&read_fds,NULL, NULL, NULL);
        if(ret < 0){
            perror("Errore in fase di select: \n");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &read_fds)){

                if(i == STDIN_FILENO){ //stdin

                    gestione_comandi_server();
                    if(server_status == 0){
                         close(listener);
                         return 0;
                    }

                    continue;
                }

                if(i == listener){ //nuova connessione da parte di un nuovo client

                    len = sizeof(cl_addr);
                    newfd = accept(listener, (struct sockaddr*) &cl_addr, &len);
                    if(newfd < 0){
                        perror("Errore in fase di accept: \n");
                        exit(EXIT_FAILURE);
                    }

                    FD_SET(newfd, &master); 
                    if(newfd > fdmax)
                        fdmax = newfd;

                    client_connessi++;
                    ts = getTimestamp();
                    printf("%s: CONNESSIONE AVVENUTA CON UN NUOVO CLIENT\n", ts);

                }else{

                    gestore_richieste_client(i); // gestore delle richieste

                }
            }
        }
    }

    close(listener);
    return 0;
}

void guida_comandi(int port){
    printf("****************** SERVER STARTED ******************\n");
    printf("Digita un comando:\n\n");
    printf("1) start --> avvia il server (porta %d) di gioco\n",port);
    printf("2) stop --> termina il server di gioco\n");
}

void gestione_comandi_server(){
    char *input = NULL;
    size_t input_size = 0;
    while(1){

        getline(&input, &input_size, stdin);
        input[strcspn(input, "\n")] = '\0';
        if(strcmp(input,"start") == 0){
            if(server_status == 1){
                printf("SERVER AVVIATO PRECEDENTEMENTE!\n");
                break;
            }
            server_status = 1;
            printf("SERVER AVVIATO!\n");
            break;
        }else if(strcmp(input,"stop") == 0){
            if(client_connessi != 0){
                printf("CI SONO DEI CLIENT, IL SERVER NON PUO' TERMINARE\n");
                break;
            }
            server_status = 0;
            printf("SERVER OFFLINE!\n");
            break;
        }

        printf("COMANDO ERRATO!\n");

    }

    free(input);
    return;
}

void gestore_richieste_client(int i){ // riceve un comando dal client e in base al tipo chiama la funzione associata
    char cmd[LEN_MSG_SERVER];
    check = true; // check mi serve per verificare se nelle fasi di registrazione o autenticazione il client si disconnette
    ts = getTimestamp();
    lettura_comando_client(i, cmd); //leggo il byte inviato dal client che specifica il tipo di comando
    struct giocatore *player = trovaGiocatore(i);
    if(player != NULL)
       printf("%s: RICHIESTA DEL GIOCATORE %s IL COMANDO %s.\n", ts, player->username, cmd);

    if(strcmp(cmd, REGISTRAZIONE) == 0){ //registrazione
        gestione_registrazione(i);
    }else if(strcmp(cmd, ACCEDI) == 0){ //accesso
        gestione_accesso(i);
    }else if(strcmp(cmd, LOOK) == 0){ //look
        gestione_look(i);
    }else if(strcmp(cmd, LOOK_PARAMETRO) == 0){ // look [location | object]
        gestione_look_parametro(i);
    }else if(strcmp(cmd, TAKE) == 0){ //take
        gestione_take(i);
    }else if(strcmp(cmd, USE1) == 0){ //use obj1
        gestione_use1(i);
    }else if(strcmp(cmd, USE2) == 0){ //use obj1 obj2
        gestione_use2(i);
    }else if(strcmp(cmd, OBJ) == 0){ //obj
        gestione_obj(i);   
    }else if(strcmp(cmd, DROP) == 0){//drop obj
        gestione_drop(i);
    }else if(strcmp(cmd, END) == 0){ //end
        gestione_end(i);
    }
    
    return;   
}

void gestione_registrazione(int i){ // riceve dal descrittore i l'username e password del nuovo utente e li copia nella struttura utente  
    ts = getTimestamp();
    printf("%s: RICHIESTA DA UN NUOVO UTENTE DI REGISTRARSI\n", ts);
    ricevi_credenziali_client(i);
    if(!check) return; //verifico se il client non si sia disconnesso
    scrittura_file_registrazione();
    return;

}

void gestione_accesso(int i){ //gestore accesso
    ts = getTimestamp();
    printf("%s: RICHIESTA DA UN NUOVO UTENTE DI ACCEDERE\n", ts);
    ricevi_credenziali_client(i);
    if(!check) return; //verifico se il client non si sia disconnesso

    if(verifica_credenziali_client() == 1){
        printf("%s: CLIENT %s AUTENTICATO\n", ts, U.username); 
        invio_comando_client(i, OPERAZIONE_RIUSCITA);
        lettura_ruolo(i);
    }else{
        printf("%s: AUTENTICAZIONE CLIENT FALLITA!\n", ts);
        invio_comando_client(i, OPERAZIONE_FALLITA); //autenticazione fallita
    }
    
    return;
}

void lettura_ruolo(int i){
    char buffer[BUFFER_LEN];
    lettura_comando_client(i, buffer);

    if(strcmp(buffer, GIOCATORE) == 0){ // i possibili scenari vengono inviati solamente ai client che decidono di giocare al gioco e inoltre vengono aggiunti alla lista dei giocatori
        invio_scenario(i); // invia al client i possibili scenari di gioco
        lettura_stanza(i); // il giocatore specifica in quale stanza vuole giocare
        aggiorna_strutture_dati(i); // inserisco il nuovo giocatore nella lista dei giocatori
    }else{
        gestione_aiutante(i);
    }

    return;
}

void gestione_look(int i){ //caso di look senza parametri
    char buffer[BUFFER_LEN];
    struct elem *elemento = trovaElemento("LOOK");

    strcpy(buffer, elemento->descrizione);

    send_msg(i, buffer);
    informazioni_gioco(i);
    
    return;
}

void gestione_look_parametro(int i){ // 'look qualcosa', l'utente invia il parametro alla look e il server cerca una corrispondenza nella strututtura elementi e se c'è un match invia la descrizione
    char buffer[BUFFER_LEN];
  
    recv_msg(i, buffer);
    struct elem *elemento = trovaElemento(buffer);
    struct giocatore *player = trovaGiocatore(i);

    if(elemento == NULL){
        strcpy(buffer, "PARMETRO ERRATO!\n\n");
    }else{

        if(player->stato >= elemento->stato){ //l'utente riceve la descrizione sbloccata
            strcpy(buffer, elemento->descrizione_sbloccato);
        }else{
            strcpy(buffer, elemento->descrizione);
        }
        
    }

    send_msg(i, buffer);
    informazioni_gioco(i);

    return;
}

void gestione_take(int i){
    char buffer[BUFFER_LEN];
  
    recv_msg(i, buffer);// riceve il parametro dalla take
    struct elem *elemento = trovaElemento(buffer);
    struct giocatore *player = trovaGiocatore(i);

    if(player->num_oggetti_raccolti == MAX_OBJ){ //l'utente ha raggiunto il limite di oggetti raccolti
        invio_comando_client(i, OGGETTI_LIMITE);
        informazioni_gioco(i);
        return;
    }

    if(elemento == NULL || elemento->oggetto == false){ //oggetto non trovato
        invio_comando_client(i, OGGETTO_NON_TROVATO);
        informazioni_gioco(i);
        return;
    }else if(verificaOggetto(buffer, player)){ // verifico se l'oggetto è stato raccolto precedentemente, se si invio un codice di errore
        invio_comando_client(i, OGGETTO_GIA_RACCOLTO);
        informazioni_gioco(i);
        return;
    }else if(player->stato >= elemento->stato){ // non è necessario risolvere un enigma per sbloccare l'oggetto, l'oggetto puo' essere raccolto
        strcpy(player->oggetti_raccolti[player->num_oggetti_raccolti], elemento->nome);
        player->num_oggetti_raccolti++;
        invio_comando_client(i, OPERAZIONE_RIUSCITA);
        informazioni_gioco(i);
        return;
    }else if(elemento->stato - player->stato >= 2){ //il giocatore non puo'fare la take su un oggetto con stato maggiore di almeno due rispetto allo stato del giocatore, questo vuol dire che il giocatore deve risolvere un enigma altrove
        invio_comando_client(i, OGGETTO_STATO_ALTO);
        informazioni_gioco(i);
        return;
    }else{ // c'è da risolvere un'enigma
        invio_comando_client(i, OGGETTO_ENIGMA); 
        strcpy(buffer, elemento->enigma);
    }

    send_msg(i, buffer); //invio l'enigma
    recv_msg(i, buffer); //qui ricevo la risposta all'enigma da parte del client
   
    if(strcmp(buffer, elemento->risposta) == 0 && elemento->stato == MAX_STATO){ //risposta giusta e l'enigma era l'ultimo dell'escape room
       invio_comando_client(i, ESCAPE_ROOM_VINTA);
       gestione_end(i);
       return;
    }

    if(strcmp(buffer, elemento->risposta) == 0){ //risposta giusta
       player->stato = elemento->stato;
       player->token++;
       invio_comando_client(i, ENIGMA_RISOLTO);
    }
    
    informazioni_gioco(i);

    return;
}


void gestione_use1(int i){
    int j;
    bool trovato = false;
    char buffer[BUFFER_LEN];
   
    recv_msg(i, buffer);
    struct elem *elemento = trovaElemento(buffer);
    struct giocatore *player = trovaGiocatore(i);
    
    if(elemento == NULL){ // oggetto non presente nel gioco
        invio_comando_client(i, OGGETTO_NON_TROVATO);
        informazioni_gioco(i);
        return;
    }

    for(j = 0; j < player->num_oggetti_raccolti; j++){
        if(strcmp(buffer, player->oggetti_raccolti[j]) == 0){
            trovato = true;
            break;
        }
    }

    if(!trovato){ // oggetto non presente nell'inventario
        invio_comando_client(i, OGGETTO_NON_INVENTARIO);
        informazioni_gioco(i);
        return;
    } 

    if(elemento-> usabile == false){ //oggetto non usabile singolarmente o non è usabile proprio
        invio_comando_client(i, OPERAZIONE_FALLITA);
        informazioni_gioco(i);
        return;
    } 

    if(elemento-> usabile == true){ //oggetto puo' essere usato
        invio_comando_client(i, OPERAZIONE_RIUSCITA);
        informazioni_gioco(i);
        return;
    } 

    return;
}

void gestione_use2(int i){
    ssize_t len;
    char* spacePos;
    int j, x = 0;
    char buffer[BUFFER_LEN];
    char param1[LEN_NAME];
    char param2[LEN_NAME];
   
    recv_msg(i, buffer);
    // splitto la stringa ricevuta e ottengo i due parametri della use
    spacePos = strchr(buffer, ' ');
    len = spacePos - buffer;
    strncpy(param1, buffer, len);
    param1[len] = '\0';  
    strcpy(param2, spacePos + 1);

    struct elem *elemento = trovaElemento(param1);
    struct elem *elemento2 = trovaElemento(param2);
    struct giocatore *player = trovaGiocatore(i);

    //Gestione di tutti i possibili casi che possono accadere e invio al client i possibili codici di errore:
    
    if(elemento == NULL){ // oggetto non presente nel gioco
        invio_comando_client(i, OGGETTO_NON_TROVATO);
        informazioni_gioco(i);
        return;
    }

    if(elemento2 == NULL){ // oggetto non presente nel gioco
        invio_comando_client(i, OGGETTO_NON_TROVATO);
        informazioni_gioco(i);
        return;
    }

    for(j = 0; j < player->num_oggetti_raccolti; j++){ //verifico che entrambi gli oggetti siano presenti nell'inventario
        if(strcmp(param1, player->oggetti_raccolti[j]) == 0 || strcmp(param2, player->oggetti_raccolti[j]) == 0){
            x++;
        }
    }

    if(x != 2){ // un oggetto non è presente nell'inventario
        invio_comando_client(i, OGGETTO_NON_INVENTARIO);
        informazioni_gioco(i);
        return;
    } 

    if(elemento->usabile_obj2 == false || elemento2->usabile_obj2 == false){ //oggetti non usabile con altri
        invio_comando_client(i, OPERAZIONE_FALLITA);
        informazioni_gioco(i);
        return;
    } 
    

    if((strcmp(elemento->nome_usabile, elemento2->nome) == 0) && (strcmp(elemento2->nome_usabile, elemento->nome) == 0)){ //verifico che i due oggetti si possano usare insieme 
        //lo stato del giocatore è incrementato prendendo il massimo stato dei due oggetti
        if(elemento->stato > elemento2->stato){
            player->stato = elemento->stato + 1; //l'utilizzo di due oggetti insieme mi incrementa lo stato del giocatore perchè questo portera' allo sblocco di una nuova parte del gioco(in caso di gameF6 la porta viene aperta)
        }else{
            player->stato = elemento2->stato + 1; //l'utilizzo di due oggetti insieme mi incrementa lo stato del giocatore perchè questo portera' allo sblocco di una nuova parte del gioco(in caso di gameF6 la porta viene aperta)
        }
        
        invio_comando_client(i, OPERAZIONE_RIUSCITA);
        informazioni_gioco(i);
    }else{ // gli oggetti non possono essere usati insieme
        invio_comando_client(i, OGGETTI_NON_INSIEME);
        informazioni_gioco(i);
        return;
    }

    return;
}

void gestione_obj(int i){ // risposta al comando del client 'objs'
    int j;
    char buffer[MAX_OBJ*LEN_NAME] = "";
    struct giocatore *player = trovaGiocatore(i);

    if(player->num_oggetti_raccolti != 0){ // verifico che ci siano oggetti nell'inventario e nel buffer concateno gli oggetti
        for(j = 0; j < player->num_oggetti_raccolti; j++){
            strcat(buffer, player->oggetti_raccolti[j]);
            strcat(buffer, "\n");  
        }
    }else{
        strcpy(buffer, OPERAZIONE_FALLITA); //nessun oggetto nell'inventario
    }
    
    send_msg(i, buffer);
    informazioni_gioco(i);

    return;
}

void gestione_drop(int i){
    int j;
    bool trovato = false;;
    char buffer[BUFFER_LEN];
    recv_msg(i, buffer); //ricevo l'oggetto per il quale il giocatore vuole lasciare
    struct giocatore *player = trovaGiocatore(i);

    if(player->oggetti_raccolti == 0){ // non ci sono oggetti nell'inventario
        invio_comando_client(i, OGGETTO_NON_INVENTARIO);
        informazioni_gioco(i);
        return;
    }

    for(j = 0; j < player->num_oggetti_raccolti; j++){
        if(strcmp(buffer, player->oggetti_raccolti[j]) == 0){
            trovato = true;
            break;
        }
    }

    if(!trovato){ // oggetto non trovato
        invio_comando_client(i, OPERAZIONE_FALLITA);
        informazioni_gioco(i);
        return;
    }

    if(trovato){ // oggetto trovato
        if(j == 0){ // aggiorno il suo inventario
            strcpy(player->oggetti_raccolti[j], player->oggetti_raccolti[j + 1]);
            strcpy(player->oggetti_raccolti[j+1], "");
        }else{
            strcpy(player->oggetti_raccolti[j], "");
        }
        
        player->num_oggetti_raccolti--;
        invio_comando_client(i, OPERAZIONE_RIUSCITA);
        informazioni_gioco(i);
        return;
    }

    return;
}

void gestione_end(int i){
    ts = getTimestamp();
    printf("%s: IL SOCKET %d HA EFFETTUATO LA DISCONNESSIONE CON IL SERVER!\n", ts, i);
    client_connessi--;
    close(i);
    FD_CLR(i, &master);
    elimina_giocatore(i);
    return;
}

void gestione_aiutante(int i){ // il server si aspetta di ricevere un intero compreso tra 1 e 60, rapresentano i secondi, a quel punto scegliera un giocatore a caso e incrementera' i secondi rimanenti
    int ret, giocatore_casuale, j;
    uint16_t random;
    struct giocatore *temp;
    ts = getTimestamp();

    ret = recv(i, (void*)&random, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    
    random = ntohs(random);  
    gestione_end(i);
    if(giocatori_in_corso == 0){
        printf("%s: OPERAZIONE DI AIUTO FALLITA POICHE' NESSUN GIOCATORE IN CORSO\n", ts);
        return;
    }
    
    srand(time(NULL));
    giocatore_casuale = rand() % giocatori_in_corso + 1; // genera un numero casuale compreso tra [1,giocatori_in_corso], incremento il tempo a un giocatore casuale

    //stampa_giocatori(); debug
    temp = head; 
    for (j = 1; j < giocatore_casuale; j++) {
        temp = temp->next;
    }

    temp->startTime += random;
    printf("%s: AL GIOCATORE %s SONO STATI AGGIUNTI %d SECONDI!\n", ts, temp->username, random);

    return;
}

void invio_scenario(int i){ //l'utente ha effettuato l'accesso, verranno inviati i scenari di gioco disponibili
    int ret;
    char buffer[LEN_CODICE_STANZA];
    strcpy(buffer, STANZA);
    ret = send(i, (void*)buffer, LEN_CODICE_STANZA, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;
}

void lettura_stanza(int i){
    int ret;
    ret=recv(i, (void*)stanza_di_gioco, LEN_CODICE_STANZA, 0);
    
    if(ret < 0){
        perror("Errore in fase di recv(5): \n");
        return;   
    }else if(ret == 0){
        return;
    }

    return;

}

void scrittura_file_registrazione(){
    FILE *fileUtenti = fopen("utenti.txt", "a");

    if (fileUtenti == NULL) {
        perror("Errore nell'apertura del file!");
        exit(EXIT_FAILURE);
    }

    fprintf(fileUtenti, "%s %s", U.username, U.password);
    fprintf(fileUtenti, "\n");
    fclose(fileUtenti);
    return;
}


int verifica_credenziali_client(){ // verifica che nel file utenti.txt ci sia un match con username e psw inviati dal client
    FILE *fileUtenti = fopen("utenti.txt", "r");
    char buffer[256];
    char fileUsername[11];
    char filePassword[11];

    if (fileUtenti == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
      
    while (fgets(buffer, sizeof(buffer), fileUtenti) != NULL) {

        sscanf(buffer, "%10s %10s", fileUsername, filePassword);

        if (strcmp(U.username, fileUsername) == 0 && strcmp(U.password, filePassword) == 0) {
            fclose(fileUtenti);
            return 1;
        }

    }

    fclose(fileUtenti);
    return 0; 
}

void ricevi_credenziali_client(int i){
    int ret;
    char buffer[BUFFER_LEN];
    ret=recv(i, (void*)buffer, LEN_USERNAME, 0);
    if(ret < 0){
        perror("Errore in fase di recv(3): \n");
        return;
    }else if(ret == 0){
        check = false;
        return;
    }

    strcpy(U.username, buffer);

    ret=recv(i, (void*)buffer, LEN_PSW, 0);
    if(ret < 0){
        perror("Errore in fase di recv(4): \n");
        return;
    }else if(ret == 0){
        check = false;
        return;
    }

    strcpy(U.password, buffer);
    return;
}

void informazioni_gioco(int i){ // invia al socket i le informazioni sullo stato del gioco: tempo rimanente, tocken raccolti
    char buffer[BUFFER_LEN];

    struct giocatore *player = trovaGiocatore(i);
    time_t now = time(NULL);
    int tempoTrascorso = difftime(now, player->startTime);
    int tempoRimanente = MAX_TIME - tempoTrascorso;

    if (tempoRimanente < 0) {tempoRimanente = 0;}

    int minuti = tempoRimanente / 60;
    int secondi = tempoRimanente % 60;

    snprintf(buffer, BUFFER_LEN, "%02d:%02d", minuti, secondi);
    snprintf(buffer + strlen(buffer), BUFFER_LEN - strlen(buffer), " %d", player->token);

    send_msg(i, buffer);

    return;
}

void aggiorna_strutture_dati(int i){ //crea la struttura giocatore e la aggiunge alla lista dei giocatori, inizializza tutte le strutture dati

    struct giocatore *nuovoGiocatore = (struct giocatore*)malloc(sizeof(struct giocatore));
    
    if (nuovoGiocatore == NULL) {
       perror("Errore nell'allocazione della memoria!.\n");
       exit(EXIT_FAILURE);
    }
  
    strncpy(nuovoGiocatore->username, U.username, LEN_USERNAME);
    strncpy(nuovoGiocatore->stanza, stanza_di_gioco, LEN_CODICE_STANZA);
    nuovoGiocatore->socket = i;
    nuovoGiocatore->stato = 0;
    nuovoGiocatore->num_oggetti_raccolti = 0;
    nuovoGiocatore->token = 0;
    nuovoGiocatore->startTime = time(NULL);
    nuovoGiocatore->next = head;
    head = nuovoGiocatore;

    giocatori_in_corso++;

    return;
}

void elimina_giocatore(int i){
    struct giocatore *temp = head;
    struct giocatore *precedente = NULL;

    while (temp != NULL && temp->socket != i){
        precedente = temp;
        temp = temp->next;
    }

    if (temp != NULL) { // se temp == null allora il giocatore non è nella lista dei giocatori

        if (precedente != NULL){
            precedente->next = temp->next;
        } else{
            head = temp->next;
        }

        giocatori_in_corso--;
        free(temp);
    }

    return;
}

struct elem *trovaElemento(char *nome) {
    int i;
    for (i = 0; i < NUM_ELEM; i++) {
        if (strcmp(ELEMENTI[i].nome, nome) == 0) {
            return &ELEMENTI[i];
        }
    }
    return NULL; // Elemento non trovato
}

struct giocatore *trovaGiocatore(int i){
    struct giocatore *temp = head;

    while (temp != NULL) {
        if (temp->socket == i) {
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}

bool verificaOggetto(char* buffer, struct giocatore* player){ // verifico se il player possiede già l'oggetto nel suo inventario
    int i;
    for (i = 0; i < player->num_oggetti_raccolti; i++) {
        if (strcmp(buffer, player->oggetti_raccolti[i]) == 0) {
            return true;  
        }
    }

    return false; 
}

char* getTimestamp() { //per il log
    static char timestamp[MAX_TIME_STRING];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, MAX_TIME_STRING, "[%H:%M:%S %d/%m/%Y]", timeinfo);

    return timestamp;
}

void stampa_giocatori(){ // per debug
    struct giocatore *temp = head;
    
    if(head == NULL){
        printf("NESSUN GIOCATORE!\n");
    }

    printf("Numero giocatori in corso: %d\n", giocatori_in_corso);
    printf("Lista dei giocatori:\n");

    while (temp != NULL) {
        printf("Username: %s, Stanza: %s, Socket: %d\n", temp->username, temp->stanza, temp->socket);
        temp = temp->next;
    }

    return;
}

void send_msg(int i, char* buffer){
    int ret, len;
    uint16_t len_msg;

    len = strlen(buffer) + 1;
    len_msg = htons(len);
    
    ret = send(i, (void*)&len_msg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    ret = send(i, (void*)buffer, len, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;
}

void recv_msg(int i, char* buffer){
    int ret, len;
    uint16_t len_msg;

    ret = recv(i, (void*)&len_msg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }

    len = ntohs(len_msg);
    ret = recv(i, (void*)buffer, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }  

    if(ret == 0){
        gestione_end(i);
        return;
    }

    /* possibile gestione nel caso in cui la recv non abbia letto tutti i dati, il controllo puo' essere fatto poichè si conosce la dimensione del messaggio (inviata precedentemente)
   
    int bytes_received = 0;
    while (bytes_received < len) {
        ret = recv(i, (void*)(buffer + bytes_received), len - bytes_received, 0);
        if(ret < 0){
            perror("Errore in fase di ricezione: ");
            return;
        } else if (ret == 0) {
            printf("Connessione chiusa dal peer.\n");
            return;
        }
        bytes_received += ret;
    }
    
    */

    return;
}

void invio_comando_client(int i, char* cmd){
    int ret;
    ret = send(i, (void*)cmd, LEN_MSG_SERVER, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;
}


void lettura_comando_client(int i, char* cmd){
    int ret;
    ret = recv(i, (void*)cmd, LEN_MSG_SERVER, 0);
    if(ret < 0){
        perror("Errore in fase di recv(1): \n");
        return;   
    }

    if(ret == 0){
        strcpy(cmd, END);
    }

    return;
}