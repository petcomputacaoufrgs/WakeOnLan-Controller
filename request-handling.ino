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

// Manda página HTML com as informações dos PCs conectados ao controlador
void serveConnectedPCListsPage(EthernetClient& client){
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1> Controlador WakeOnLan </h1>");
    client.print("<br>PCs gerenciados: ");
    client.print(pcStorage.getNumPCs());
    client.print("/");
    client.print(MAX_PCS);

    // Lista os computadores conectados
    // TO-DO: Essa lista tem que ser lida do SD
    client.print("<ol>");
    
    for(int i=0; i < pcStorage.getNumPCs(); i++){
        uint8_t readChars = 0;
        macAddr const& mac = pcStorage.getMACfromID(i);
        client.print("<li>");
        for(int j=0; j < MAC_STRING_SIZE; j++){
            // : do MAC
            if(readChars++ == 2){
                client.print(':');
                readChars = 1;
            }
            client.print(mac.addr[j]);
        }
        client.print("</li>");
    }
    
    client.print("</ol>");
    
    // Botão para inserir MAC
    client.print("<form action=\"\" method=\"post\">");
    client.print("<label for=\"mac\">Insira seu endereco MAC (APENAS DÍGITOS HEXADECIMAIS)</label>");
    client.print("<br>");
    client.print("<input name=\"macaddr\" id=\"mac\" type=\"text\">");
    client.print("<button>Enviar</button>");
    client.print("</form>");

    client.println("</html>");

}

// Adiciona um MAC ao WakeOnLan
bool addMACforMonitoring(EthernetClient& client){

    // Retorna se limite já foi atingido
    if(pcStorage.getNumPCs() == MAX_PCS){
        return false;
    }


    bool currentLineIsBlank = false;
    bool readingPOSTInfo = false;
    bool readMAC = false;
    char addrBuffer[MAC_STRING_SIZE];
    uint8_t POSTReadBytes = 0;
    uint8_t macBytesRead = 0;
    while(client.connected()){
        if(client.available()){
            char c = client.read();

            // Lê bytes até encontrar as informações do POST
            if(!readingPOSTInfo){
                if (c == '\n' && currentLineIsBlank) {
                    readingPOSTInfo = true;
                }
                if (c == '\n') {
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
            // Lê bytes contendo info de post
            else{
                // Checa se já passou por "macaddr="
                if(POSTReadBytes >= POST_MAC_VALUE_STRING_BYTES_TO_SKIP){
                    // Não permite caracteres que não sejam HEX
                    if(!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))){
                        break;
                    }

                    // Copia para buffer
                    addrBuffer[macBytesRead] = c;
                    macBytesRead++;
                }

                POSTReadBytes++;

                if((POSTReadBytes == POST_MAC_VALUE_SIZE)){
                    readMAC = true;
                }

            }
        }
        else{
            break;
        }
    }

    // Checa se MAC foi lido
    if(!readMAC){
        return false;
    }

    // Armazena MAC
    macAddr newMAC;

    Serial.print("Novo MAC: ");
    for(int i=0; i < MAC_STRING_SIZE; i++){
        Serial.print(addrBuffer[i]);
        newMAC.addr[i] = addrBuffer[i];
    }
    Serial.println();
    for(int i=0, j=0; j<12; i++, j+=2){
        newMAC.byte_addr[i] = ((char2hex(addrBuffer[j]) << 4) | char2hex(addrBuffer[j+1]));
    }

    Serial.println();
    
    pcStorage.pushPC(newMAC);


    return true;

}