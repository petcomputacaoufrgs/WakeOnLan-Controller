/*

    Includes, defines, etc

*/

#ifndef WOL_CONTROLLER_H
#define WOL_CONTROLLER_H

#include <Ethernet.h>
#include <EEPROM.h>
#include <Keypad.h>

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

// Limitação da memória RAM
#define MAX_CONNECTED_PCS 4
#define GET_REQUEST "GET"
#define POST_REQUEST "POST"

// macaddr=xxxxxxxxxxxx
// Pulamos até "="
#define POST_MAC_VALUE_STRING_BYTES_TO_SKIP 8
#define MAC_STRING_SIZE 12
#define POST_MAC_VALUE_SIZE (POST_MAC_VALUE_STRING_BYTES_TO_SKIP + MAC_STRING_SIZE)
// Endereços mac
struct macAddr{ char addr[MAC_STRING_SIZE]; };

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

// Globais

extern macAddr macAddrArray[MAX_CONNECTED_PCS];
extern uint8_t managedPCs;


#endif