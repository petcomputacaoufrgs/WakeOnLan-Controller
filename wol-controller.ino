
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

// Keypad 
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Modo servidor
int serverModeStartTime = 0;

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
    static char key;

    key = keypad.getKey();
    if(key){
        Serial.print("New key input: ");
        Serial.println(key);
        if(key == '#'){
            Serial.println("Entrando em modo servidor por 1 minuto...");
            serverModeStartTime = millis();
            // Modo servidor
            while(millis() - serverModeStartTime < 60000){
                // Esperando cliente
                EthernetClient client = server.available();
                // Se há um cliente, lida com a comunicação com ele
                if(client){
                    handleClient(client);
                }
            }
            Serial.println("Saindo do modo servidor");
        }
    }




    // Esperando cliente
    EthernetClient client = server.available();
    // Se há um cliente, lida com a comunicação com ele
    if(client){
        handleClient(client);
    }

    // Checa input do teclado


}

// Lida com requests HTML do cliente
void handleClient(EthernetClient& client){
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