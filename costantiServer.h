#include "gameF6.h"
#include "cmdClientServer.h"
#define BUFFER_LEN 1024
#define DEFAULT_PORT 4242
#define LEN_USERNAME 11
#define LEN_PSW 11
#define LEN_DESCRIPTION 1024
#define LEN_NAME 128
#define BACKLOG 10
#define MAX_OBJ 2
#define MAX_TIME_STRING 30

struct utente{
    char username[LEN_USERNAME];
    char password[LEN_PSW];
};

struct giocatore{ // descrive un giocatore connesso
    char username[LEN_USERNAME];
    char stanza[LEN_CODICE_STANZA];
    int socket;
    char oggetti_raccolti[MAX_OBJ][LEN_NAME];
    int num_oggetti_raccolti;
    int stato;
    int token;
    time_t startTime;
    struct giocatore *next;
};

struct utente U;
struct giocatore *head = NULL;

int giocatori_in_corso = 0; // dimensione della lista dei giocatori in corso
int server_status = 0; // 0 il server non è stato ancora avviato
int listener, fdmax;
int client_connessi; // client connessi, sono compresi anche quelli nell'operazione di sigin e signup
bool check; // serve per verificare una possibile disconnessione del client durante le fasi di registrazione o accesso
char stanza_di_gioco[LEN_CODICE_STANZA];
fd_set master, read_fds;
char* ts; //timestamp

void guida_comandi(int);
void gestione_comandi_server();
void gestore_richieste_client(int);
void gestione_registrazione(int);
void lettura_comando_client(int, char*);
void invio_comando_client(int, char*);
void scrittura_file_registrazione();
void gestione_accesso(int);
int verifica_credenziali_client();
void ricevi_credenziali_client(int);
void invio_scenario(int);
void lettura_stanza(int);
void aggiorna_strutture_dati(int);
void stampa_giocatori();
void elimina_giocatore(int);
void gestione_look(int);
void gestione_look_parametro(int);
void gestione_end(int);
void gestione_obj(int);
void gestione_take(int);
void gestione_use1(int);
void gestione_use2(int);
void gestione_drop(int);
void gestione_aiutante(int);
struct elem *trovaElemento(char*);
struct giocatore *trovaGiocatore(int);
bool verificaOggetto(char*, struct giocatore*);
char* getTimestamp();
void informazioni_gioco(int);
void lettura_ruolo(int);
void send_msg(int, char*);
void recv_msg(int, char*);