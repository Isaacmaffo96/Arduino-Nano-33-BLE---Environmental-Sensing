#include "Arduino.h"
#include <ArduinoBLE.h>
#include "Nano33BLETemperature.h"
#include "Nano33BLEPressure.h"

#define BLE_BUFFER_SIZES  27  // massimo 27 bit in un pacchetto BLE
#define BLE_DEVICE_NAME   "Arduino Nano 33 BLE 1"   // Nome dispositivo
#define BLE_LOCAL_NAME    "Stazione Meteo 1 (BLE)" // Nome Locale

Nano33BLETemperatureData temperatureData;  // oggetto di tipo Nano33BLETemperatureData
Nano33BLEPressureData pressureData;        // oggetto di tipo Nano33BLEPressureData

// Caratteristiche BLE
BLEService environmentalSensingService("181A"); // UUID 181A Environmental Sensing
// BLECharacteristic(uuid, properties, stringValue)
// Caratteristica globale con buffer contenente temperatura, umidità, pressione e altitudine
BLECharacteristic globalBLE("2A25",  BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES); // Serial Number String UUID 2A25
BLEDescriptor globalDescriptor("2901", "Temperature[*C], Humidity[%], Pressure[Kpa], Altitude[m]"); // Characteristic User Description UUID 2901
// Caratteristiche specifiche dello standard ble per ogni dato con descrittore
BLEShortCharacteristic temperatureCharacteristic("2A6E", BLERead | BLENotify); // Temperature UUID 2A6E
BLEDescriptor temperatureDescriptor("2901", "Temperature misure [*C]"); // Characteristic User Description UUID 2901
BLEUnsignedShortCharacteristic humidityCharacteristic("2A6F", BLERead | BLENotify); // Humidity UUID 2A6F 
BLEDescriptor  humidityDescriptor("2901", "Humidity misure [%]"); // Characteristic User Description UUID 2901
BLEUnsignedLongCharacteristic pressureCharacteristic("2A6D", BLERead | BLENotify); // Pressure UUID 2A6D
BLEDescriptor pressureDescriptor("2901", "Pressure misure [Kpa]"); // Characteristic User Description UUID 2901
BLEUnsignedLongCharacteristic elevationCharacteristic("2A6C", BLERead | BLENotify); // Eleavtion UUID 2A6C
BLEDescriptor altitudeDescriptor("2901", "Elevation misure [m]"); // Characteristic User Description UUID 2901

char bleBuffer[BLE_BUFFER_SIZES]; // Buffer BLE per le caratterestiche offerte
// 6 bit max Temperatura , 6 bit max per Umidità, 5 bit max Pressione, 7 bit max Altitudine = 24 + 3(',') = 27

void setup(){
  Serial.begin(115200); // setup della porta seriale (baud rate = 115200)
  while(!Serial); // controllo di apertura seriale

  pinMode(LED_BUILTIN, OUTPUT);  // setup led di segnalazione funzionamento
  Temperature.begin(); // setup sensore temperatura
  Pressure.begin();    // setup sensore pressione

  if (!BLE.begin()){ // Setup BLE
    Serial.println("Errore - Procedura di setup del modulo Bluetooth Low Energy (BLE) fallita!");
    while (1);    
  }
    else{
      Serial.println("Setup modulo Bluetooth Low Energy (BLE)...");
      BLE.setDeviceName(BLE_DEVICE_NAME);
      BLE.setLocalName(BLE_LOCAL_NAME);
      BLE.setAdvertisedService(environmentalSensingService);
      environmentalSensingService.addCharacteristic(globalBLE);
      globalBLE.addDescriptor(globalDescriptor);
      environmentalSensingService.addCharacteristic(temperatureCharacteristic);
      temperatureCharacteristic.addDescriptor(temperatureDescriptor);
      environmentalSensingService.addCharacteristic(humidityCharacteristic);
      humidityCharacteristic.addDescriptor(humidityDescriptor);
      environmentalSensingService.addCharacteristic(pressureCharacteristic);
      pressureCharacteristic.addDescriptor(pressureDescriptor);
      environmentalSensingService.addCharacteristic(elevationCharacteristic);
      elevationCharacteristic.addDescriptor(altitudeDescriptor);
      BLE.addService(environmentalSensingService);
      BLE.advertise();
      Serial.println("Ricerca di un dispositivo centrale BLE ...");
    }
}

void loop(){
  digitalWrite(LED_BUILTIN,LOW); // spento non c'è connessione
  //centrale è un dispositivo che ricerca i dispositivi Bluetooth per connettersi
  BLEDevice central = BLE.central(); //ascolta le periferiche Bluetooth Low Energy da connettere:
  if(central){ // se una centrale è collegata alla periferica:
    Serial.print("Connesso al dispositivo centrale: ");
    Serial.println(central.address());
    int writeLength;
    // Quando si connette un dispositivo si esegue la lettura dei dati
    while(central.connected()){  // mentre la centrale è ancora connessa
      digitalWrite(LED_BUILTIN,LOW); // spento non c'è connessione
      if(Temperature.pop(temperatureData)){ // se è presente un valore del sensore di temperatura
        if (Pressure.pop(pressureData)) {      // se è presente un valore del sensore di pressione
          digitalWrite(LED_BUILTIN, HIGH);     // lampeggia mentre campiona i valori
          delay(200);                          // attesa di 200ms

          // BLE defines Temperature UUID 2A6E Type sint16
          // Unit is in degrees Celsius with a resolution of 0.01 degrees Celsius
          int16_t temperature = round( temperatureData.temperatureCelsius * 100.0 );
          temperatureCharacteristic.writeValue(temperature); // scrive il valore della caratteristica

          // BLE defines Humidity UUID 2A6F Type uint16
          // Unit is in percent with a resolution of 0.01 percent
          uint16_t humidity = round( temperatureData.humidity * 100.0 );
          humidityCharacteristic.writeValue(humidity); // scrive il valore della caratteristica

          // BLE defines Pressure UUID 2A6D Type uint32
          // Unit is in Pascal with a resolution of 0.1 Pa
          uint32_t pressure = round( pressureData.barometricPressure * 10.0 );
          pressureCharacteristic.writeValue(pressure); // scrive il valore della caratteristica

          // BLE defines Eleavtion UUID 2A6C Type sint24
          // Unit is in meters with a resolution of 0.01 m
          // H = 44330 * [1 - (P/p0)^(1/5.255)] dove p0 è la pressione di riferimento all'altezza del mare (101.325 kPa).
          float altitude = 44330 * ( 1 - pow(pressureData.barometricPressure/101.325, 1/5.255) ); // calcolo dell'altitudine data la pressione atmosferica
          uint32_t elevation = round(altitude * 100);
          elevationCharacteristic.writeValue(elevation); // scrive il valore della caratteristica

          // limite massimo pacchetto ble generato 27 bit 
          // static_cast<void*> : cast forzato a puntatore generico void per evitare ambiguità
          writeLength = sprintf(bleBuffer, "%.2f,%.2f,%.1f,%.2f",temperatureData.temperatureCelsius, temperatureData.humidity, pressureData.barometricPressure, altitude);
          globalBLE.writeValue(static_cast<void*>(bleBuffer), writeLength); // scrive il valore della caratteristica 
          Serial.println(bleBuffer);  // stampa su monitor seriale il buffer
        }
      }
    }
    Serial.print("Disconnesso dal dispositivo centrale: ");
    Serial.println(central.address());
    Serial.println("Ricerca di un dispositivo centrale BLE ...");
  }
}