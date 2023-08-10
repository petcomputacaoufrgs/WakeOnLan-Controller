
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
unsigned long serverModeStartTime = 0;

// Buzzer
const int audioPin = 48; //Definição do pino do Buzzer 

// Modo atual do controlador
ControllerMode controllerMode = NONE;

// Buffer de input
char inputString[3] = {' ',' ','\0'};
uint8_t inputKeysPressed = 0;


void setup()
{
    // Inicialização do Serial
    Serial.begin(9600);

    // Buzzer
    pinMode(audioPin, OUTPUT);

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

    // Som de inicialização
    makeSound(SOUND_TURNON);


}

void loop(){
    static char key;

    key = keypad.getKey();
    if(key){
        Serial.print("New key input: ");
        Serial.println(key);

        if(controllerMode == WAKING){
            // Pressionar * novamente cancela.
            if(key == '*'){
                controllerMode = NONE;
                Serial.println("Saíndo do modo WOL");
                makeSound(SOUND_CANCEL);
                return;
            }
            // Pressionar # ou ter mais de uma tecla no buffer para acordar 
            if(key == '#'){
                uint8_t wakingID = atoi(inputString);
                // Tentando acordar PC não gerenciado
                if(wakingID > managedPCs){
                    Serial.print("Nao ha PC de id ");
                    Serial.println(wakingID);
                    makeSound(SOUND_CANCEL);
                    inputKeysPressed = 0;
                    return;
                }
                else{
                    Serial.print("Acordando PC de ID ");
                    Serial.println(wakingID);
                    Serial.print("e MAC: ");
                    for(int i = 0; i < MAC_STRING_SIZE; i++){
                        Serial.print(macAddrArray[wakingID].addr[i]);
                    }
                    Serial.println();
                    makeSound(SOUND_WAKING_PC);
                    inputKeysPressed = 0;
                    return;
                }
            }
            // Armazena inputs numéricos
            if(inputKeysPressed < 2){
                inputString[inputKeysPressed] = key;
                inputKeysPressed++;
            }
            makeSound(SOUND_KEY_HIT);


        }
        else if(controllerMode == SERVER){
            // Pressionar # novamente ou timeout retorna ao modo normal
             if((key == '#') || (millis() - serverModeStartTime < 60000)){
                controllerMode = NONE;
                Serial.println("Saindo do modo servidor");
                makeSound(SOUND_SERVER_MODE_OFF);
                return;
             }
        }
        else{
            // Entra no modo WakeOnLan
            if(key == '*'){
                makeSound(SOUND_WAKEONLAN_ON);
                controllerMode = WAKING;
                inputKeysPressed = 0;
            }

            // Entra no modo servidor por um tempo
            if(key == '#'){
                makeSound(SOUND_SERVER_MODE_ON);
                Serial.println("Entrando em modo servidor por 1 minuto...");
                controllerMode = SERVER;
                serverModeStartTime = millis();
            }
        }
    }


    if(controllerMode == SERVER){
        EthernetClient client = server.available();
        // Se há um cliente, lida com a comunicação com ele
        if(client){
            handleClient(client);
        }
    }
        


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
        if(addMACforMonitoring(client)){
            makeSound(SOUND_MAC_ADDED);
        }
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


void makeSound(Sound sound){
    switch (sound)
    {
    case SOUND_TURNON:
        tone(audioPin, 1046.5, 150);
        delay(100);
        tone(audioPin, 1174.66, 150);
        delay(100);
        tone(audioPin, 1318.51, 150);
        delay(100);
        noTone(audioPin);
        break;

    case SOUND_KEY_HIT:
        tone(audioPin, 329.63, 100);
        delay(100);
        noTone(audioPin);
        break;
    
    case SOUND_SERVER_MODE_ON:
        tone(audioPin, 987.77, 100);
        delay(100);
        noTone(audioPin);
        delay(75);
        tone(audioPin, 987.77, 100);
        delay(100);
        noTone(audioPin);
        delay(75);
        tone(audioPin, 1174.66, 100);
        delay(100);
        noTone(audioPin);
        break;

    case SOUND_SERVER_MODE_OFF:
        tone(audioPin, 1174.66, 100);
        delay(100);
        noTone(audioPin);
        delay(75);
        tone(audioPin, 987.77, 100);
        delay(100);
        noTone(audioPin);
        delay(75);
        tone(audioPin, 783.99, 100);
        delay(100);
        noTone(audioPin);
        break;

    case SOUND_MAC_ADDED:
        tone(audioPin, 493.88, 200);
        delay(200);
        noTone(audioPin);
        delay(100);
        tone(audioPin, 1975.53, 100);
        delay(100);
        noTone(audioPin);
        delay(50);
        tone(audioPin, 1975.53, 100);
        delay(100);
        noTone(audioPin);
        break;

    case SOUND_CANCEL:
        tone(audioPin, 349.23, 100);
        delay(100);
        tone(audioPin, 349.23, 100);
        delay(100);
        noTone(audioPin);
        break;
    
    case SOUND_WAKEONLAN_ON:
        tone(audioPin, 261.63, 100);
        delay(100);
        noTone(audioPin);
        delay(50);
        tone(audioPin, 261.63, 100);
        delay(100);
        noTone(audioPin);
        break;

    case SOUND_WAKING_PC:
        tone(audioPin, 329.63, 100);
        delay(100);
        tone(audioPin, 440.00, 100);
        delay(100);
        noTone(audioPin);
        break;

    }
}