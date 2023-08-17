#include "wol-controller.h"

void PCStorage::begin(){
    numPCs = EEPROM.read(0);

    for(int i = 0; i < numPCs; i++){
        EEPROM.get(1 + i*sizeof(macAddr), MACArray[i]);
    }

}

uint8_t PCStorage::getNumPCs(){
    return numPCs;
}

void PCStorage::pushPC(macAddr const& macInfo){
    EEPROM.put(1 + numPCs*sizeof(macAddr), macInfo);
    MACArray[numPCs] = macInfo;
    numPCs++;
    EEPROM.write(0, numPCs);
}

macAddr const& PCStorage::getMACfromID(uint8_t id){
    return MACArray[id];
}

void PCStorage::reset(){
    numPCs = 0;
    EEPROM.write(numPCs, 0);
}
