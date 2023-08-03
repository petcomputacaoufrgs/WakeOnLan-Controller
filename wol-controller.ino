
/*

    Controlador WOL do PET

    Mantém uma lista de computadores os quais podem ser acordados por WOL
    usando o teclado do controlador (com cada computador sendo associado um número)

    Para adicionar ou remover um computador

*/

#include "wol-controller.h"

macAddr macAddrArray[MAX_CONNECTED_PCS];
uint8_t managedPCs = 0;

byte mac[] = {
    0xAA, 0xBB, 0xCC, 0x00, 0x11, 0x22
};

// Ouvindo por https
EthernetServer server(80);



// bool SDInitialized = false;

void setup()
{
    // Inicialização do Serial
    Serial.begin(9600);

    // Inicialização Ethernet

    if (Ethernet.begin(mac) == 0) {
        Serial.println("Falha na obtenção de IP DHCP");
        while (true); // Idle
    }
    else{
        Serial.print("Meu IP DHCP: ");
        Serial.println(Ethernet.localIP());
    }
    
    // Espera inicialização do shield
    delay(1000);


}

void loop(){
    // Esperando cliente
    EthernetClient client = server.available();
    if(client){
        // Obtém tipo de request
        WOLControllerRequests requestType = getClientRequestProtocol(client);
        switch (requestType)
        {
        case WOLControllerRequests::GET:
            // Mostra página interface
            serveConnectedPCListsPage(client);
            break;
        case WOLControllerRequests::POST:
            // Adiciona MAC à lista
            addMACforMonitoring(client);
            serveConnectedPCListsPage(client);
            break;
        default:
            return;
        }

        // Dar tempo ao navegador para receber os dados
        delay(1);
        // Fecha a conexão:
        client.stop();
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
    client.print(managedPCs);
    client.print("/");
    client.print(MAX_CONNECTED_PCS);

    // Lista os computadores conectados
    // TO-DO: Essa lista tem que ser lida do SD
    client.print("<ol>");
    
    for(int i=0; i < managedPCs; i++){
        uint8_t readChars = 0;
        client.print("<li>");
        for(int j=0; j < MAC_STRING_SIZE; j++){
            // : do MAC
            if(readChars++ == 2){
                client.print(':');
                readChars = 1;
            }
            client.print(macAddrArray[i].addr[j]);
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
                    Serial.print(c);
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

    // Armazena MAC e retorna verdadeiro
    for(int i=0; i < MAC_STRING_SIZE; i++){
        Serial.print(addrBuffer[i]);
        macAddrArray[managedPCs].addr[i] = addrBuffer[i];
    }
    // Incrementa número de PCs
    managedPCs++;

}