
/*

    Controlador WOL do PET

    Mantém uma lista de computadores os quais podem ser acordados por WOL
    usando o teclado do controlador (com cada computador sendo associado um número)

    Para adicionar ou remover um computador

*/

#include <SD.h>
#include <Ethernet.h>

#define MAX_CONNECTED_PCS 16

// Informações de cada PC conectado
// typedef struct pcInfo{
//     char hostname[] = "VERSAO BETA";
//     char mac_str[] = "VERSAO BETA";
//     char ip_str[] = "VERSAO BETA";
// } pcInfo;

//pcInfo pcInfoArray[MAX_CONNECTED_PCS];
uint8_t managedPCs = 0;

byte mac[] = {
    0xAA, 0xBB, 0xCC, 0x00, 0x11, 0x22
};

// Ouvindo por https
EthernetServer server(80);

// Arquivo no cartão SD onde são armazenados hostname, mac, ip

bool SDInitialized = false;

void setup()
{
    // Inicialização do Serial
    Serial.begin(9600);

    // disable the ethernet SPI
    pinMode(10, OUTPUT);
    digitalWrite(10,HIGH);

    // Inicialização cartão SD
    Serial.println("Initializing SD card...");
    if(!SD.begin(4)){
        Serial.println("Failed SD initialization");
        Serial.println("The controller will not be storing PC info. DO NOT TURN OFF THE BOARD!");
    }
    else{
        SDInitialized = true;
    }

    // Inicialização Ethernet

    Serial.println("Initializing Ethernet with DHCP...");


    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Checagem de problemas

        // Shield presente?
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true); // Idle
        }
        // Cabo conectado?
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("Ethernet cable is not connected.");
        }
    }
    else{
        Serial.print("My DHCP IP: ");
        Serial.println(Ethernet.localIP());
    }

    // Espera inicialização do shield
    delay(1000);



}

void loop(){
    // Esperando cliente
    EthernetClient client = server.available();
    if(client){
        Serial.println("Novo Cliente");
        boolean currentLineIsBlank = true;
        while(client.connected()){
            if(client.available()){
                char c = client.read();
                //Serial.write(c);
                if (c == '\n' && currentLineIsBlank) {
                    serveConnectedPCListsPage(client);
                    break;
                }
                if (c == '\n') {
                // Você está começando uma nova linha
                currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                // Você recebeu um caracter na linha atual.
                currentLineIsBlank = false;
                }
            }
        }
        // Dar tempo ao navegador para receber os dados
        delay(1);
        // Fecha a conexão:
        client.stop();
        Serial.println("Conexão encerrada");
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
    client.print("<p>PCs gerenciados: ");
    client.print(managedPCs);
    client.print("/");
    client.print(MAX_CONNECTED_PCS);

    // Lista os computadores conectados
    // TO-DO: Essa lista tem que ser lida do SD
    client.print("<ol>");
    for(int i=0; i < managedPCs; i++){
        client.print("<li>");
        client.print("UM HOSTNAME ");
        client.print("UM MAC");
        client.print("UM IP");
        client.print("</li>");
    }
    

    client.print("</ol>");

    client.println("</html>");

}