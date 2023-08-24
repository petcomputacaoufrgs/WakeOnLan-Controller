
/*

    Controlador WOL do PET

    Mantém uma lista de computadores os quais podem ser acordados por WOL
    usando o teclado do controlador (com cada computador sendo associado um número)

    Para adicionar ou remover um computador

*/

#include "wol-controller.h"

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

// LCD
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Armazenamento de PCs
PCStorage pcStorage;

void setup()
{
    // Inicialização do Serial
    Serial.begin(9600);

    // Memória
    pcStorage.begin();

    // Buzzer
    pinMode(audioPin, OUTPUT);

    // LCD
    lcd.begin(16, 2);
    analogWrite(CONSTRAST_PIN, BASE_CONSTRAST);
    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print("Inicializando...");

    // Inicialização Ethernet

    if (Ethernet.begin(mac) == 0) {
        Serial.println("Falha na obtenção de IP DHCP");
        lcd.setCursor(0, 1);
        lcd.print("Falha em obter DHCP!");
        while (true); // Idle
    }
    else{
        Serial.print("Meu IP DHCP: ");
        Serial.println(Ethernet.localIP());
    }
  
    lcd.setCursor(0, 1);
    lcd.print("Sucesso!");

    // Som de inicialização
    makeSound(SOUND_TURNON);

    // Espera inicialização do shield
    delay(1000);

    lcd.clear();



}

void loop(){
    static char key;
    static bool printMenu = true; // TO-DO: Encapsular esses comportamentos do LCD em classe?
    static bool zeroPressed = false; // Tecla zero pressionada durante menu normal. Duas vezes faz com que a memória resete.

    key = keypad.getKey();

    if(controllerMode == WAKING){
        // Pressionar * novamente cancela.
        if(key == '*'){
            controllerMode = NONE;
            Serial.println("Saíndo do modo WOL");
            
            lcd.clear();
            
            makeSound(SOUND_CANCEL);
            return;
        }
        // Pressionar # para acordar 
        if((key == '#')){
            // Faz nada se não há input
            if(inputKeysPressed == 0){
                return;
            }

            uint8_t wakingID = atoi(inputString);
            // Tentando acordar PC não gerenciado
            if(!wakeUpPC(wakingID)){
                Serial.print("Nao ha PC de id ");
                Serial.println(wakingID);

                // Reseta input
                inputKeysPressed = 0;

                lcd.setCursor(0, 1);
                lcd.print("ID N REGISTRADO");

                lcd.setCursor(4, 0);
                lcd.print("  ");

                makeSound(SOUND_CANCEL);

            }
            else{
                macAddr const& mac = pcStorage.getMACfromID(wakingID);

                Serial.print("Acordando PC de ID ");
                Serial.println(wakingID);
                Serial.print("e MAC: ");
                for(int i = 0; i < MAC_STRING_SIZE; i++){
                    Serial.print(mac.addr[i]);
                }
                Serial.println();

                lcd.setCursor(14, 0);
                lcd.print("OK");
                lcd.setCursor(0, 1);
                for(int i = 0; i < MAC_STRING_SIZE; i++){
                    lcd.print(mac.addr[i]);
                }

                makeSound(SOUND_WAKING_PC);
            }
            inputKeysPressed = 0;
            inputString[0] = '0';
            inputString[1] = '0';


            return;
        }
        
        if(key){
            // Armazena inputs numéricos
            if(inputKeysPressed < 2){
                
                lcd.setCursor(4 + inputKeysPressed, 0);
                lcd.print(key);

                inputString[inputKeysPressed] = key;
                inputKeysPressed++;
            }
            makeSound(SOUND_KEY_HIT);

        }


    }
    else if(controllerMode == SERVER){
        // Pressionar # novamente ou timeout retorna ao modo normal
            if((key == '#') || ((millis() - serverModeStartTime) > 60000)){
            controllerMode = NONE;
            Serial.println("Saindo do modo servidor");
            lcd.clear();
            makeSound(SOUND_SERVER_MODE_OFF);
            return;
            }
            EthernetClient client = server.available();
            // Se há um cliente, lida com a comunicação com ele
            if(client){
                handleClient(client);
            }
    }
    else{
        // Entra no modo WakeOnLan
        if(key == '*'){
            makeSound(SOUND_WAKEONLAN_ON);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("WOL: ");
            controllerMode = WAKING;
            printMenu = true;
            inputKeysPressed = 0;
            return;
        }

        // Entra no modo servidor por um tempo
        if(key == '#'){
            makeSound(SOUND_SERVER_MODE_ON);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("SERVINDO PAG. EM:");
            lcd.setCursor(0, 1);
            lcd.print(Ethernet.localIP());
            // for(int i = 0; i < 12; i++){
            //   if((i > 0) && i % 3){
            //     lcd.print('.');
            //   }  
            // }
            Serial.println("Entrando em modo servidor por 1 minuto...");
            controllerMode = SERVER;
            printMenu = true;
            serverModeStartTime = millis();
            return;
        }

        // Apertar zero duas vezes reseta memória
        if(key == '0'){
            if(!zeroPressed){
                zeroPressed = true;
                return;
            }

            pcStorage.reset();
            lcd.clear();
            lcd.print("MEM RESET");
            delay(1500);

            zeroPressed = false;
            printMenu = true;
        
        }

        if(printMenu == true){
            lcd.setCursor(0, 0);
            lcd.print("* - WOL  # - GER");
            lcd.setCursor(0, 1);
            lcd.print("PCS: ");
            lcd.print(pcStorage.getNumPCs());
            printMenu = false;
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

uint8_t char2hex(char c){
    if(c > '9'){
        return 15 - ('F' - c);
    }
    return 9 - ('9' - c);
}