#include "cmdClientServer.h"
#define BUFFER_LEN 1024
#define SERVER_PORT 4242
#define LEN_CODICE_STANZA 3
#define LEN_USERNAME 11
#define LEN_PSW 11
#define LEN_NAME 128
#define MAX_TOKEN 2

struct utente{
    char username[LEN_USERNAME];
    char password[LEN_PSW];
};

struct utente U;

int sd;
int operazione = 0; // se 0 l'utente deve registrarsi, altrimenti no
char stanza_di_gioco[LEN_CODICE_STANZA]; //nel caso di piu' scenari, andrebbe allocato un vettore di stanze di gioco
bool start  = true;
int ruolo;

void guida_comandi();
void lettura_stanza_stdin();
void lettura_autenticazione_stdin();
void guida_autenticazione();
void registrazione_stdin(); // lettura dei dati da tastiera per la registrazione
void registrazione(); //invio dei dati al server per la registrazione
void accedi(); //invio di username e psw per verificare l'autenticazione col server
void invio_comando_server(char*);
void lettura_user_psw_stdin();
void lettura_comando_server(char*);
void lettura_stanza_di_gioco();
void conferma_stanza();
void lettura_comando();
void look();
void look_parametro(char*);
void take(char*);
void objs();
void use1(char*);
void use2(char*, char*);
void drop(char*);
void informazioni_gioco(); //stampa i token raccolti e il tempo rimanente
void lettura_ruolo();
void send_msg(char*);
void recv_msg(char*);