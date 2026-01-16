#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);  // RX, TX
VBUSDecoder vbus(&vbusSerial);

uint32_t lastMillis = millis();

void setup() {
  vbusSerial.begin(9600);
  vbus.begin();
  
  Serial.begin(115200);
  Serial.println(F("Starting test of VBUS code.")); 
}

void loop() {
  vbus.loop();

  if ((millis() - lastMillis) > (5 * 1000UL)) {
    char * vbusStat = (vbus.getVbusStat())?"Ok":"Error";
    Serial.print(F("VBUS communication status: "));
    Serial.println(vbusStat);

    char * vbusReady = (vbus.isReady())?"Yes":"No";
    Serial.print(F("VBUS data ready: "));
    Serial.println(vbusReady);

    if (vbus.isReady()) {
      uint8_t tempNum = vbus.getTempNum();
      uint8_t relayNum = vbus.getRelayNum();
      uint8_t pumpNum = vbus.getPumpNum();
      
      Serial.print(F("Temperature sensors: "));
      for (uint8_t i = 0; i < tempNum; i++) {
        Serial.print(vbus.getTemp(i));
        Serial.print(F(", "));
      }
      Serial.println();

      Serial.print(F("Pump power: "));
      for (uint8_t i = 0; i < pumpNum; i++) {
        Serial.print(vbus.getPump(i));
        Serial.print(F(", "));
      }
      Serial.println();

      Serial.print(F("Relay Status: "));
      for (uint8_t i = 0; i < relayNum; i++) {
        Serial.print(vbus.getRelay(i)?"ON": "OFF");
        Serial.print(F(", "));
      }
      Serial.println();

      // Display extended information
      uint16_t errorMask = vbus.getErrorMask();
      if (errorMask != 0) {
        Serial.print(F("Error Mask: 0x"));
        Serial.println(errorMask, HEX);
      }

      uint16_t systemTime = vbus.getSystemTime();
      if (systemTime > 0) {
        Serial.print(F("System Time: "));
        Serial.print(systemTime);
        Serial.println(F(" minutes"));
      }

      uint32_t opHours0 = vbus.getOperatingHours(0);
      uint32_t opHours1 = vbus.getOperatingHours(1);
      if (opHours0 > 0 || opHours1 > 0) {
        Serial.print(F("Operating Hours [1]: "));
        Serial.print(opHours0);
        Serial.print(F(" h, [2]: "));
        Serial.print(opHours1);
        Serial.println(F(" h"));
      }

      uint16_t heatQty = vbus.getHeatQuantity();
      if (heatQty > 0) {
        Serial.print(F("Heat Quantity: "));
        Serial.print(heatQty);
        Serial.println(F(" Wh"));
      }

      uint8_t sysVariant = vbus.getSystemVariant();
      if (sysVariant > 0) {
        Serial.print(F("System Variant: "));
        Serial.println(sysVariant);
      }
    }
    lastMillis = millis();
  }
}
