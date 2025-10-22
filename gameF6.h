#define LEN_DESCRIPTION 1024
#define LEN_ENIGMA 256
#define LEN_NAME 128
#define LEN_RISPOSTA 128
#define NUM_ELEM 6
#define MAX_TOKEN 2
#define MAX_STATO 3
#define MAX_TIME 180 //secondi
#define LEN_CODICE_STANZA 3
const char STANZA[LEN_CODICE_STANZA] = "F6";

struct elem{
    char nome[LEN_NAME];
    char descrizione[LEN_DESCRIPTION];
    char descrizione_sbloccato[LEN_DESCRIPTION];
    bool bloccato;
    bool locazione;
    bool oggetto;
    char enigma[LEN_ENIGMA];
    char risposta[LEN_RISPOSTA];
    bool usabile;
    bool usabile_obj2; // se l'oggetto è usabile con un altro oggetto
    char nome_usabile[LEN_NAME]; //specifica con quale oggetto puo' essere usato nel comando use
    int stato;
};

struct elem ELEMENTI[NUM_ELEM] ={
    {
        "LOOK",
        "Sei in un aula universitaria. Difronte a te c'è una ++cattedra++,\nalla tua destra c'è una ++porta++.\n\n",
        "Sei in un aula universitaria. Difronte a te c'è una ++cattedra++,\nalla tua destra c'è una ++porta++.\n\n",
        false,
        true,
        false,
        "",
        "",
        false,
        false,
        "",
        0
    },
    {
        "cattedra",
        "Sulla ++cattedra++ c'è uno **smartphone** acceso, la schermata mostra un form di una pagina web.\n\n",
        "Sulla ++cattedra++ c'è uno **smartphone** acceso, la schermata mostra un form di una pagina web.\n\n",
        false,
        true,
        false,
        "",
        "",
        false,
        false,
        "",
        0
    },
    {
        "smartphone",
        "Il telefono è bloccato. E' necessario inserire una password per sbloccarlo\n\n",
        "Inquadrare QR-CODE..\n\n",
        true,
        false,
        true,
        "OGGETTO BLOCCATO, è Necessario risolvere l'enigma!\nCOMPLETA LA SEQUENZA:\n                LOCE LOCE LOCE\n LOCE LOCE LOCE LOCE LOCE LOCE LOCE LOCE LOCE\n\n -----_------?\n",
        "treno veloce",
        false,
        true,
        "QR-CODE",
        1
    },
    {
        "porta",
        "La porta è chiusa, ma è attaccato un **QR-CODE** su di essa.\n\n",
        "PORTA SBLOCCATA! Nella stanza c'è una piccola **cassaforte**..\n\n",
        false,
        true,
        false,
        "",
        "",
        false,
        false,
        "",
        2
    },
    {
        "QR-CODE",
        "QR-CODE.\n\n",
        "QR-CODE.\n\n",
        false,
        false,
        true,
        "",
        "",
        false,
        true,
        "smartphone",
        0
    },
    {
        "cassaforte",
        "La **cassaforte** è chiusa con un lucchetto.\n\n",
        "Cassaforte sbloccata! Escape room completata con successo!\n\n",
        true,
        false,
        true,
        "OGGETTO BLOCCATO, è Necessario risolvere l'enigma!\nSulla cassaforte c'è un display touch, è mostrato a video una domanda:\n1 5 21 ..?\na)78\nb)55\nc)85",
        "c",
        false,
        false,
        "",
        3
    }
};
