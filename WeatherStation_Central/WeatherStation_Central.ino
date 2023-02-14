#include "Arduino.h"
#include <ArduinoBLE.h>
#include "Nano33BLETemperature.h"
#include "Nano33BLEPressure.h"

#define BLE_BUFFER_SIZES  27  // massimo 27 byte in un pacchetto BLE
#define BLE_DEVICE_NAME   "Arduino Nano 33 BLE 2"   // Nome dispositivo
#define BLE_LOCAL_NAME    "Stazione Meteo (BLE) 2" // Nome Locale

const char* deviceServiceUuid = "181A"; // UUID 181A Environmental Sensing

char bleBuffer[BLE_BUFFER_SIZES]; // Buffer BLE per le caratterestiche offerte
// 6 byte max Temperatura , 6 byte max per Umidità, 5 byte max Pressione, 7 byte max Altitudine = 24 + 3(',') = 27byte
//byte value[BLE_BUFFER_SIZES];
//int16_t temperature = 0;
//uint16_t humidity = 0;
//uint32_t pressure = 0;
//uint32_t elevation = 0;

void setup(){
  if (!Serial)
  {
    Serial.println("Serial not working correctly");
  }
  Serial.begin(115200); // setup della porta seriale (baud rate = 115200)
  Serial.println("Serial began correctly");
  while (!Serial); // controllo di apertura seriale
  //il while commmentato in modo da poter disconnettere

  //controllo su sensori di temp e pressione non vanno per un problema di conversione a bool
  pinMode(LED_BUILTIN, OUTPUT);  // setup led di segnalazione funzionamento

  if (!BLE.begin()){ // Setup BLE
    Serial.println("Errore - Procedura di setup del modulo Bluetooth Low Energy (BLE) fallita!");
    while (1);    
  }
    else{
      Serial.println("Setup modulo Bluetooth Low Energy (BLE)...");
      BLE.setDeviceName(BLE_DEVICE_NAME);
      BLE.setLocalName(BLE_LOCAL_NAME);
      BLE.advertise();
      Serial.println("Stazione Meteo (BLE) Centrale");
      Serial.println("Ricerca di un dispositivo periferico BLE ...");
  }
}

void loop() {
  
  Serial.println("- Trying to connect to peripheral...");
  connectToPeripheral();
}

// funzione di connessione alla periferica
void connectToPeripheral(){
 digitalWrite(LED_BUILTIN,LOW); // spento non c'è connessione

  BLEDevice peripheral;
  
  Serial.println("- Discovering peripheral device...");
 
  // scansiona fino a che non trova una periferica
  do
  {
    BLE.scanForUuid(deviceServiceUuid);
    peripheral = BLE.available();
  } while (!peripheral);
  
  if (peripheral) {
    Serial.println("* Peripheral device found!");
    Serial.print("* Device MAC address: ");
    Serial.println(peripheral.address());
    Serial.print("* Device name: ");
    Serial.println(peripheral.localName());
    Serial.print("* Advertised service UUID: ");
    Serial.println(peripheral.advertisedServiceUuid());
    Serial.println(" ");
    BLE.stopScan();
    controlPeripheral(peripheral);
  }
}

// funzione controllo della periferica
void controlPeripheral(BLEDevice peripheral) {
  Serial.println("- Connecting to peripheral device...");

  if (peripheral.connect()) {
    Serial.println("* Connected to peripheral device!");
    Serial.println(" ");
  } else {
    Serial.println("* Connection to peripheral device failed!");
    Serial.println(" ");
    return;
  }

// discover peripheral attributes
  Serial.println("- Discovering peripheral device attributes...");
  if (peripheral.discoverAttributes()) {
    Serial.println("* Peripheral device attributes discovered!");
    Serial.println(" ");
  } else {
    Serial.println("* Peripheral device attributes discovery failed!");
    Serial.println(" ");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic globalBLE = peripheral.characteristic("2A25");  
    
  if (!globalBLE) {
    Serial.println("* Peripheral device does not have globalBLE characteristic!");
    peripheral.disconnect();
    return;
  } else if (!globalBLE.canRead()) {
    Serial.println("* Peripheral does not have a readble globalBLE characteristic!");
    peripheral.disconnect();
    return;
  } else if (!globalBLE.canSubscribe()) {
    Serial.println("* globalBLE characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!globalBLE.subscribe()) {
    Serial.println("* Subscription failed!");
    peripheral.disconnect();
    return;
  }

 while (peripheral.connected()) {
   digitalWrite(LED_BUILTIN,HIGH); // acceso c'è connessione
  // globalBLE.readValue(bleBuffer, BLE_BUFFER_SIZES);
    
   if (globalBLE.valueUpdated()) {  
    globalBLE.readValue(bleBuffer, BLE_BUFFER_SIZES);
    Serial.println(bleBuffer);  // stampa su monitor seriale il buffer
    }
  }

  Serial.println("- Peripheral device advertise");
}