#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);  // RX, TX
VBUSDecoder vbus(&vbusSerial);

uint32_t lastMillis = millis();

void setup() {
  // Configure serial based on protocol:
  // VBUS: 9600 baud, 8N1 (default)
  // KW-Bus/P300: 4800 baud, 8E2
  // KM-Bus: varies by device
  vbusSerial.begin(9600);  // For VBUS protocol
  // vbusSerial.begin(4800, SERIAL_8E2);  // Uncomment for KW-Bus or P300
  
  // Initialize with desired protocol:
  // PROTOCOL_VBUS - for RESOL VBUS devices (Vitosolic, DeltaSol)
  // PROTOCOL_KW - for KW-Bus Viessmann devices (requires 4800 8E2)
  // PROTOCOL_P300 - for P300/Optolink Viessmann devices (requires 4800 8E2)
  // PROTOCOL_KM - for KM-Bus Viessmann devices
  vbus.begin(PROTOCOL_VBUS);  // Default is VBUS
  
  Serial.begin(115200);
  Serial.println(F("Starting Viessmann Multi-Protocol Library test."));
  
  // Display active protocol
  switch(vbus.getProtocol()) {
    case PROTOCOL_VBUS:
      Serial.println(F("Active Protocol: VBUS (RESOL)"));
      break;
    case PROTOCOL_KW:
      Serial.println(F("Active Protocol: KW-Bus (VS1)"));
      break;
    case PROTOCOL_P300:
      Serial.println(F("Active Protocol: P300 (VS2/Optolink)"));
      break;
    case PROTOCOL_KM:
      Serial.println(F("Active Protocol: KM-Bus"));
      break;
  }
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
