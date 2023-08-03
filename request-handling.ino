/*

    Funções de tratamento de requests HTML

*/

#include "wol-controller.h"

// Lê o request de um cliente e define se é POST ou GET
WOLControllerRequests getClientRequestProtocol(EthernetClient& client){
    String protocol;
    // Lê bytes de conexão até encontrar espaço em branco. A string resultante é o protocolo
    while(client.connected()){
        if(client.available()){
            char c = client.read();
            
            if (c == ' ') {
                // Retorna protocolo
                if(protocol == GET_REQUEST) return WOLControllerRequests::GET;
                if(protocol == POST_REQUEST) return WOLControllerRequests::POST;
                return WOLControllerRequests::UNKNOWN;
            }
            else{
                protocol += c;
            }
        }
    }

}