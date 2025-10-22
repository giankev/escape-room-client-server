#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "costantiClient.h"

int main(int argc, char *argv[]){
    int ret;
    struct sockaddr_in  srv_addr;
    char msg_server[LEN_MSG_SERVER];
    uint16_t random;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    while(1){
        ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
        if(ret < 0){
            printf("Attesa connessione al server..\n");
            sleep(2);
        }else if (ret == 0){
            break;
        }     
    }
    
    guida_autenticazione(); // stampa le operazioni possibili (reg,acc)
    lettura_autenticazione_stdin(); // lettura dell'operazione richiesta: registrazione o accesso
    lettura_ruolo(); // il client specifica se vuole essere un giocatore o partecipare come 'aiutante' per un altro giocatore

    if(operazione == 0){registrazione();}
    
    do{

        accedi();
        lettura_comando_server(msg_server); // verifico se l'autenticazione è andata a buon fine
        if(strcmp(msg_server, OPERAZIONE_FALLITA) == 0){printf("AUTENTICAZIONE FALLITA, riprovare\n\n");}

    }while(strcmp(msg_server, OPERAZIONE_FALLITA) == 0);
    
    printf("AUTENTICAZIONE AVVENUTA CON SUCCESSO!\n\n");
    if(ruolo == 0){ // il client ha specificato ruolo 'aiutante'
       invio_comando_server(AIUTANTE);
       //il ruolo dell'aiutante consiste nell'aumentare il tempo rimanente di un giocatore scelto casualmente
       srand(time(NULL)); //inizializza il generatore 
       // Genera un numero casuale compreso tra 1 e 60
       random = (rand() % 60) + 1;
       random = htons(random);
       ret = send(sd, (void*)&random, sizeof(uint16_t), 0);

       if(ret < 0){
            perror("Errore in fase di invio: \n");
            exit(EXIT_FAILURE);
        }

        return 0;

    }else{
        invio_comando_server(GIOCATORE);
    }

    guida_comandi();
    lettura_stanza_stdin(); // l'utente specifica in quale stanza vuole giocare
    conferma_stanza();// la stanza specificata dall'utente viene inviata al server
    
    //adesso in poi il gioco puo' iniziare
    printf("****************** STANZA DI GIOCO INIZIALIZZATA ******************\n");
    printf("GIOCO INIZIATO, USA IL COMANDO 'look' PER AVERE UNA DESCRIZIONE DELLA STANZA\n\n");

    while(start){lettura_comando();}

    close(sd);
    return 0;
}

void guida_comandi(){
    lettura_stanza_di_gioco();
    printf("****************** CLIENT STARTED ******************\n");
    printf("Elenco dei comandi accettati dal client:\n\n");
    printf("1) start room --> avvia il gioco nella stanza %s\n", stanza_di_gioco);
    printf("2) look [location | object]--> Fornisce una breve descrizione della stanza con le sue locazioni.\n");
    printf("3) take object --> Consente al giocatore di raccogliere un oggetto presente nella stanza corrente, il cui nome (object) è specificato come parametro.\n");
    printf("4) use object1 [object2] --> Permette al giocatore di utilizzare un oggetto (object1) precedentemente raccolto. Se il comando contiene anche il parametro object2, l'oggetto object1 viene usato con l'oggetto object2.\n");
    printf("5) objs --> Mostra all'utente l'elenco degli oggetti raccolti fino a quel momento.\n");
    printf("6) drop --> Lascia un oggetto presente nell'inventario.\n");
    printf("7) end --> Termina il gioco e la connessione con il server di gioco.\n\n");
    printf("PER INIZIARE IL GIOCO SPECIFICARE IL CODICE DELLA STANZA, ad esempio 'start %s'\n", stanza_di_gioco);
}

void lettura_ruolo(){
    char *input = NULL;
    size_t input_size = 0;
    printf("\nSPECIFICA IL RUOLO CHE VUOI AVERE NEL GIOCO:\n1)Giocatore\n2)Aiutante\n\n");

    while(1){
        
        printf("> ");
        getline(&input, &input_size, stdin);
        input[strcspn(input, "\n")] = '\0';
        if(strcmp(input,"1") == 0){
            ruolo = 1; //Giocatore
            free(input);
            return;
        }else if(strcmp(input,"2") == 0){
            ruolo = 0; //Aiutante
           free(input);
           return;
        }
        printf("COMANDO ERRATO!\n");
    }

    free(input);
    return;
}

void lettura_stanza_stdin(){
    char *input = NULL;
    char buffer[BUFFER_LEN]; 
    size_t input_size = 0;
    sprintf(buffer, "start %s", stanza_di_gioco);

    while(1){
        
        printf("> ");
        getline(&input, &input_size, stdin);
        input[strcspn(input, "\n")] = '\0';
        if(strcmp(input, buffer) == 0){
            printf("Richiesta al server stanza %s ..!\n\n", stanza_di_gioco);
            break;
        }

        printf("COMANDO ERRATO!\n");

    }

    free(input);
    return;
}

void conferma_stanza(){ // il client conferma il server su quale stanza vuole giocare
    int ret;
    
    ret = send(sd, (void*)stanza_di_gioco, LEN_CODICE_STANZA, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }
    
    return;
}

void guida_autenticazione(){
    printf("E' NECESSARIO AUTENTICARSI PRIMA DI INIZIARE IL GIOCO:\n");
    printf("1) Digita 'signup' --> nel caso in cui non possiedi un account\n");
    printf("2) Digita 'sigin' --> se possiedi un account esistente\n");
}

void lettura_autenticazione_stdin(){
    char *input = NULL;
    size_t input_size = 0;
    while(1){

        printf("> ");
        getline(&input, &input_size, stdin);
        input[strcspn(input, "\n")] = '\0';
        if(strcmp(input,"sigin") == 0){
            free(input);
            operazione = 1;
            return;
        }else if(strcmp(input,"signup") == 0){
           free(input);
           operazione = 0;
           return;
        }
        printf("COMANDO ERRATO!\n");
    }

    return;
}

void registrazione_stdin(){ // chiamata nell'operazione di autenticazione

    printf("FASE DI REGISTRAZIONE: \n\n");
    printf("a) digitare username valido, consentiti al massimo 10 caratteri\n");
    printf("b) digitare password valida, consentiti al massimo 10 caratteri\n");

    char *username = NULL;
    char *password = NULL;
    size_t username_size = 0;
    size_t password_size = 0;
    int len = 0;

    do{

        getline(&username, &username_size, stdin);
        len = strlen(username);
        if(len > 11){
            printf("LUNGHEZZA USERNAME ERRATA!\n");
        }else{
            username[strcspn(username, "\n")] = '\0';
            strncpy(U.username, username, len);
        }

    }while(len > 11);
        
    do{

        getline(&password, &password_size, stdin);
        len = strlen(password);
        if(len > 11){
            printf("LUNGHEZZA PASSWORD ERRATA!\n");
        }else{
            password[strcspn(password, "\n")] = '\0';
            strncpy(U.password, password, len);
        }

    }while(len > 11);

    free(username);
    free(password);

    return;
}

void registrazione(){
    int ret;
    char buffer[BUFFER_LEN];

    invio_comando_server(REGISTRAZIONE);
    registrazione_stdin(); // ottiene username e psw da stdin

    //invio di username e psw al server
    strcpy(buffer, U.username);
    ret = send(sd, (void*)buffer, LEN_USERNAME, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    strcpy(buffer, U.password);
    ret = send(sd, (void*)buffer, LEN_PSW, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;   
}

void accedi(){

    int ret;
    char buffer[BUFFER_LEN];

    invio_comando_server(ACCEDI);
    lettura_user_psw_stdin();

    //invio di username e psw al server
    strcpy(buffer, U.username);
    ret = send(sd, (void*)buffer, LEN_USERNAME, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    strcpy(buffer, U.password);
    ret = send(sd, (void*)buffer, LEN_PSW, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;   

}

void lettura_user_psw_stdin(){

    printf("FASE DI ACCESSO: \n\n");
    printf("a) digitare username valido e password valida, consentiti al massimo 10 caratteri per ognuno\n");

    char *username = NULL;
    char *password = NULL;
    size_t username_size = 0;
    size_t password_size = 0;
    int len = 0;

    do{

        getline(&username, &username_size, stdin);
        len = strlen(username);
        if(len > 11){
            printf("LUNGHEZZA USERNAME ERRATA!\n");
        }else{
            username[strcspn(username, "\n")] = '\0';
            strncpy(U.username, username, len);
        }

    }while(len > 11);
        
    do{

        getline(&password, &password_size, stdin);
        len = strlen(password);
        if(len > 11){
            printf("LUNGHEZZA PASSWORD ERRATA!\n");
        }else{
            password[strcspn(password, "\n")] = '\0';
            strncpy(U.password, password, len);
        }

    }while(len > 11);

    free(username);
    free(password);

    return;

}

void lettura_stanza_di_gioco(){
    int ret;
    ret = recv(sd, (void*)stanza_di_gioco, LEN_CODICE_STANZA, 0);
    if(ret < 0){
        perror("Errore in fase di rcv(2): \n");
        exit(EXIT_FAILURE);
    }

    return;
}

void lettura_comando(){ //la funzione che 'racchiude' tutto. Essa viene richiamata nel main e legge da stdin una stringa valida
    char buffer[BUFFER_LEN];
    char param[LEN_NAME];
    char restante[BUFFER_LEN];
    char restante2[BUFFER_LEN];

    while(1){
        
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        if(strcmp(buffer,"look") == 0){
            invio_comando_server(LOOK);//specifico il tipo di operazione
            look();
            break;
        }else if(sscanf(buffer, "look %99s %99s", param, restante) == 2){
            printf("ERRORE! TROPPI ARGOMENTI ALLA LOOK, look accetta un solo argomento o nessuno.\n\n");
            break;
        }else if(sscanf(buffer, "look %99s", param) == 1) {
            invio_comando_server(LOOK_PARAMETRO);//specifico il tipo di operazione
            look_parametro(param);
            break;
        }else if(sscanf(buffer, "take %99s %99s", param, restante) == 2){
            printf("ERRORE! TROPPI ARGOMENTI ALLA TAKE, take accetta un solo argomento sempre.\n\n");
            break;
        } else if(sscanf(buffer, "take %99s", param) == 1){
            invio_comando_server(TAKE);//specifico il tipo di operazione
            take(param);
            break;
        }else if(sscanf(buffer, "use %s %s %s", param, restante, restante2) == 3){
            printf("ERRORE! TROPPI ARGOMENTI ALLA USE, use accetta un solo argomento o due argomenti (dipende dall'oggetto).\n\n");
            break;
        }else if(sscanf(buffer, "use %s %s", param, restante) == 2){
            invio_comando_server(USE2);
            use2(param, restante);//specifico il tipo di operazione
            break;
        }else if(sscanf(buffer, "use %s", param) == 1){
            invio_comando_server(USE1);//specifico il tipo di operazione
            use1(param);
            break;
        }else if(strcmp(buffer,"objs") == 0){
            invio_comando_server(OBJ);//specifico il tipo di operazione
            objs();
            break;
        }else if(sscanf(buffer, "drop %s %s", param, restante) == 2){ 
            printf("ERRORE! LA DROP ACCETTA UN SOLO ARGOMENTO SEMPRE!\n\n");
            break;
        }else if(sscanf(buffer, "drop %s", param) == 1){ // il giocatore vuole rilasciare un oggetto presente nell'inventario
            invio_comando_server(DROP); //specifico il tipo di operazione
            drop(param);
            break;
        }else if(strcmp(buffer,"end") == 0){
            invio_comando_server(END); //specifico il tipo di operazione
            start = false;
            break;
        }

        printf("COMANDO ERRATO!\n\n");
    }
    
    return;
}

void look(){
    char buffer[BUFFER_LEN];

    recv_msg(buffer);
    printf("Descrizione: %s", buffer);
    informazioni_gioco();

    return;
}

void look_parametro(char* param){

    send_msg(param);
    look();

    return;
}

void take(char* param){
    char buffer[BUFFER_LEN];

    send_msg(param);
    lettura_comando_server(buffer);

    if(strcmp(buffer, OGGETTI_LIMITE) == 0){ //l'oggetto non puo' essere raccolto poichè limite massimo raggiunto
       printf("NUMERO OGGETTI RACCOLTI AL LIMITE!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OGGETTO_NON_TROVATO) == 0){ //oggetto non trovato
       printf("OGGETTO SPECIFICATO ERRATO!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OGGETTO_GIA_RACCOLTO) == 0){//l'oggetto non puo' essere raccolto
        printf("Oggetto raccolto precedentemente!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OPERAZIONE_RIUSCITA) == 0){//l'oggetto puo' essere raccolto
        printf("Oggetto %s raccolto!\n\n", param);
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OGGETTO_STATO_ALTO) == 0){//l'oggetto non puo' essere raccolto
        printf("L'oggetto non puo' essere raccolto, è necessario prima risolvere enigmi altrove..\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OGGETTO_ENIGMA) == 0){// l'oggetto per essere raccolto necessita di risolvere l'enigma associato

        recv_msg(buffer); // ricevo l'enigma
        printf("%s\n\n", buffer); // stampa dell'enigma

        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send_msg(buffer);
        lettura_comando_server(buffer);

        if(strcmp(buffer, ESCAPE_ROOM_VINTA) == 0){ //ultimo enigma dell'escape room risolto
           printf("ESCAPE ROOM COMPLETATA CON SUCCESSO!\n");
           start = false;
           return;
        }

        if(strcmp(buffer, ENIGMA_RISOLTO) == 0){ //enigma risolto
            printf("Enigma risolto! Oggetto sbloccato\n\n");
        }else{
            printf("Enigma non risolto!\n\n"); //enigma non risolto
        }

        informazioni_gioco();

    }

    return;
}

void use1(char* param){
    char buffer[BUFFER_LEN];

    send_msg(param);
    lettura_comando_server(buffer);

    if(strcmp(buffer, OGGETTO_NON_TROVATO) == 0){ 
       printf("OGGETTO NON PRESENTE NEL GIOCO!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OGGETTO_NON_INVENTARIO) == 0){ 
       printf("OGGETTO NON PRESENTE NELL'INVENTARIO!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OPERAZIONE_FALLITA) == 0){
        printf("OGGETTO NON USABILE SINGOLARMENTE O NON USABILE PROPRIO!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OPERAZIONE_RIUSCITA) == 0){
        printf("Oggetto %s usato.", param);
        informazioni_gioco();
        return;
    }

}

void use2(char* param, char* param2){
    char buffer[BUFFER_LEN];

    strcpy(buffer, param);
    strcat(buffer, " ");
    strcat(buffer, param2);

    send_msg(buffer);
    lettura_comando_server(buffer);

    if(strcmp(buffer, OGGETTO_NON_TROVATO) == 0){ 
       printf("OGGETTO NON PRESENTE NEL GIOCO!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OGGETTO_NON_INVENTARIO) == 0){ 
       printf("UN OGGETTO NON E' PRESENTE NELL'INVENTARIO!\n\n");
       informazioni_gioco();
       return;
    }

    if(strcmp(buffer, OPERAZIONE_FALLITA) == 0){
        printf("UN OGGETTO O ENTRAMBI NON POSSONO ESSERE USATI CON ALTRI OGGETTI!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OGGETTI_NON_INSIEME) == 0){
        printf("OGGETTI NON USABILI INSIEME!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OPERAZIONE_RIUSCITA) == 0){
        if(strcmp(stanza_di_gioco, "F6") == 0){ // nel caso della stanza di gioco F6 stampo la stringa relativa
            printf("++porta++ SBLOCCATA!\n\n");
        }

        informazioni_gioco();
        return;
    }

    return;
}

void objs(){
    char buffer[BUFFER_LEN];

    recv_msg(buffer);

    if(strcmp(buffer, OPERAZIONE_FALLITA) == 0){
        strcpy(buffer,"Nessun oggetto nell'inventario.");
    }

    printf("\nOggetti nell'inventario:\n%s\nRicorda che l'inventario ha capienza massima di due oggetti\n\n", buffer);
    informazioni_gioco();
    return;
}

void drop(char *param){
    char buffer[BUFFER_LEN];
    
    send_msg(param);
    lettura_comando_server(buffer);

    if(strcmp(buffer, OGGETTO_NON_INVENTARIO) == 0){
        printf("NESSUN OGGETTO NELL'INVENTARIO!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OPERAZIONE_FALLITA) == 0){
        printf("OGGETTO NON PRESENTE NELL'INVENTARIO!\n\n");
        informazioni_gioco();
        return;
    }

    if(strcmp(buffer, OPERAZIONE_RIUSCITA) == 0){
        printf("OGGETTO ELIMINATO DALL'INVENTARIO!\n\n");
        informazioni_gioco();
        return;
    }

    return;
}

void informazioni_gioco(){
    int len;
    char* spacePos;
    char buffer[BUFFER_LEN];
    char param1[BUFFER_LEN];
    char param2[BUFFER_LEN];

    recv_msg(buffer);

    spacePos = strchr(buffer, ' ');
    len = spacePos - buffer;
    strncpy(param1, buffer, len);
    param1[len] = '\0';  
    strcpy(param2, spacePos + 1);

    if(strcmp(param1, "00:00") == 0){ //tempo scaduto, il giocatore ha perso
       printf("HAI PERSO, TEMPO SCADUTO!\n\n");
       start = false;
       return;
    }

    printf("TEMPO RIMASTO: %s\nTOKEN RACCOLTI: %s/%d\n\n", param1, param2, MAX_TOKEN);

    return;
}

void send_msg(char* buffer){
    int ret, len;
    uint16_t len_msg;

    len = strlen(buffer) + 1;
    len_msg = htons(len);
    
    ret = send(sd, (void*)&len_msg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    ret = send(sd, (void*)buffer, len, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;
}

void recv_msg(char* buffer){
    int ret, len;
    uint16_t len_msg;

    ret = recv(sd, (void*)&len_msg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }

    len = ntohs(len_msg);
    ret = recv(sd, (void*)buffer, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }  

    /* possibile gestione nel caso in cui la recv non abbia letto tutti i dati, il controllo puo' essere fatto poichè si conosce la dimensione del messaggio
   
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

void invio_comando_server(char* cmd){ // invio al server il comando cmd passato come parametro
    int ret;
    ret = send(sd, (void*)cmd, LEN_MSG_SERVER, 0);
    if(ret < 0){
        perror("Errore in fase di invio: \n");
        exit(EXIT_FAILURE);
    }

    return;

}

void lettura_comando_server(char* cmd){ // riceve un codice di risposta dal server
    int ret;
    ret = recv(sd, (void*)cmd, LEN_MSG_SERVER, 0);
    if(ret < 0){
        perror("Errore in fase di recv(1): \n");
        exit(EXIT_FAILURE);
    }

    return;
}