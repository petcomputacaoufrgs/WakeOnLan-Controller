/*

    Includes, defines, etc

*/

#ifndef WOL_CONTROLLER_H
#define WOL_CONTROLLER_H

#include <Ethernet.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Keypad
const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 3; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Sons do keypad
typedef enum Sound {
    SOUND_TURNON,
    SOUND_KEY_HIT,
    SOUND_SERVER_MODE_ON,
    SOUND_SERVER_MODE_OFF,
    SOUND_MAC_ADDED,
    SOUND_WAKING_PC,
    SOUND_CANCEL,
    SOUND_WAKEONLAN_ON,
} Sound;

// Columns:     3  1  5
//              45 41 49
// Rows:        2  7  6  4
//              43 53 51 47

byte pin_rows[ROW_NUM] = {25,35,33,29};      // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {27,23,31}; // connect to the column pinouts of the keypad

#define GET_REQUEST "GET"
#define POST_REQUEST "POST"

// macaddr=xxxxxxxxxxxx
// Pulamos até "="
#define POST_MAC_VALUE_STRING_BYTES_TO_SKIP 8
#define MAC_STRING_SIZE 12
#define POST_MAC_VALUE_SIZE (POST_MAC_VALUE_STRING_BYTES_TO_SKIP + MAC_STRING_SIZE)
// Endereços mac
struct macAddr{ char addr[12]; uint8_t byte_addr[6];};

// Requests HTML com os quais o controlador lida
enum WOLControllerRequests {
    GET,
    POST,
    UNKNOWN
};

// Modos de input do controlador
typedef enum ControllerMode {
    NONE,
    WAKING,
    SERVER,
} ControllerMode;



// Lê o request de um cliente e define se é POST ou GET
WOLControllerRequests getClientRequestProtocol(EthernetClient& client);

// Manda página HTML com as informações dos PCs conectados ao controlador
void serveConnectedPCListsPage(EthernetClient& client);

// Adiciona um MAC ao WakeOnLan
bool addMACforMonitoring(EthernetClient& client);

// Acorda PC com esse id
bool wakeUpPC(uint8_t id);

// Configura um pacote mágico WOL
void configureMagicPacket(uint8_t* mg_pkt, char* mac);

// Converte caracteres para valores hexadecimais
uint8_t char2hex(char c);

// Armazenamento na EEPROM
#define MAX_PCS 4

class PCStorage{

    private:
    uint8_t numPCs;
    macAddr MACArray[MAX_PCS];

    public:
    void begin();
    // Retorna número de PCs armazenados
    uint8_t getNumPCs();
    // Guarda um PC
    void pushPC(macAddr const& macInfo);
    // Obtém o endereço MAC do PC de id "id"
    macAddr const& getMACfromID(uint8_t id);
    // Reseta memória (número de PCs)
    void reset();

};

// Globais
extern PCStorage pcStorage;

//  LCD //
#define RS 40
#define EN 42
#define D4 22
#define D5 24
#define D6 26
#define D7 28
#define CONSTRAST_PIN 6
#define BASE_CONSTRAST 80



#endif