/*

    Includes, defines, etc

*/

#ifndef WOL_CONTROLLER_H
#define WOL_CONTROLLER_H

#include <Ethernet.h>
#include <EEPROM.h>

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

// Lê o request de um cliente e define se é POST ou GET
WOLControllerRequests getClientRequestProtocol(EthernetClient& client);

#endif