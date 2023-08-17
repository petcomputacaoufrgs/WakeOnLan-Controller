#include "wol-controller.h"

bool wakeUpPC(uint8_t id){
    static const uint8_t broadcast[] = { 255, 255, 255, 255};

    if(id > (pcStorage.getNumPCs() - 1)){
        return false;
    }

    // Endereço MAC em bytes
    uint8_t* mac  = pcStorage.getMACfromID(id).byte_addr;

    // Pacote mágico
    uint8_t magicPacket[102];
    configureMagicPacket(magicPacket, mac);

    EthernetUDP Udp;
    Udp.begin(9);
    Udp.beginPacket(broadcast, 9);
    Udp.write(magicPacket, sizeof(magicPacket));
    Udp.endPacket();

    return true;

}

void configureMagicPacket(uint8_t* mg_pkt, uint8_t* mac){
    int i, j, idx;
    // 6 bytes FF
    for(idx = 0; idx < 6; idx++){
        mg_pkt[idx] = 0xFF;
        // Serial.print(mg_pkt[idx], HEX);
    }
    // Serial.println();
    // 16 iterações do MAC
    for(i = 0; i < 16; i++){
        for(j = 0; j < 6; j++, idx++){
            mg_pkt[idx] = mac[j];
            // Serial.print(mg_pkt[idx], HEX);
            // Serial.print(" ");
        }
        // Serial.println();
    }

}