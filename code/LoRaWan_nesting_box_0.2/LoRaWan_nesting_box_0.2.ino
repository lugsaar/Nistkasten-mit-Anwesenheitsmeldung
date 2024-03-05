/* Nistkasten mit Anwesenheitserkennung
 *
 * Nistkasten mit Heltec WiFi LoRa 32(V3) Node und Wägezelle HX711
 *
 * Funktion:
 * Das Gewicht im Nistkasten wird regelmäßig gemessen
 * und per LoRaWAN an TheThingsNetwork übertragen.
 * An dem Verlauf der Gewichte sehen wir ob und wie
 * der Nistkasten genutzt wird
 * 
 * Ein Projekt der Linux User Group Saar
 * mancas@lug-saar.de
 * dominik@lug-saar.de
 * 
 * 
 * v0.1 neu erstellt
 * v0.2 Kommentare nach DE übersetzt und ergänzt
 */



// Werte für das LoRaWan
// =====================

#include "LoRaWan_APP.h"

// Dies sind die Werte, mit denen unseren Rotkehlchenkasten im TheThingsNetwork angelegt ist

//  uint8_t appEui[] = { 0x6E, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x0A };
//  uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x44, 0x73 };
//  uint8_t appKey[] = { 0xBA, 0xC5, 0x2D, 0xAA, 0xEC, 0x79, 0x83, 0x90, 0x9F, 0xC2, 0x56, 0xB7, 0x1C, 0xC0, 0x1D, 0x97 };


// Dies sind die Werte, mit denen unseren Blaumeisenkasten im TheThingsNetwork angelegt ist
// Es wird nur eines der Beispiele geflasht 
// 

uint8_t appEui[] = { 0x07, 0x00, 0xB3, 0x0D, 0x50, 0x7E, 0x0D, 0x01 };
uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x44, 0x90 };
uint8_t appKey[] = { 0x23, 0x6F, 0xAE, 0x6B, 0x68, 0x2E, 0x5D, 0x26, 0xE9, 0x1A, 0x78, 0xC4, 0x21, 0xC0, 0x0A, 0xD0 };

// Diese ABP Parameter bleiben unverändert
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

// LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

// LoraWan Region, wird in der Arduiono-IDE bei den Boardeigenschaften gesetzt
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

// LoraWan Class, Class A bedeutet nur Senden und kein zeitliches Empfangsfenster öffnen
DeviceClass_t  loraWanClass = CLASS_A;

// Hier wird eingetragen wie oft gesendet wird, Wert in Millisekunden.
// Gemäß Policy dürfen wir nur 1% der Zeit senden. 240000 bedeutet alle 4 Minuten,
// das passt für ein Sendefenster von 2,4 s.
// und ist gut zum Testen. Weniger Energieverbrauch bei stündlichem Senden 

//uint32_t appTxDutyCycle = 86400000;  // einmal je Tag
uint32_t appTxDutyCycle = 3600000;  // einmal je Stunde
// uint32_t appTxDutyCycle = 240000;      // alle 4 Minuten  

// Wir akzeptieren Auktivierung vom Server
bool overTheAirActivation = true;

// Wir akzeptieren die Adresse von Server
bool loraWanAdr = true;

// Wir senden nur bestätigte Nachrichten
bool isTxConfirmed = true;

// Unser Port
uint8_t appPort = 2;

// Anzahl der Versuche wenn wir keine Bestätigung vom Gateway bekommen
// dabei akzeptieren wir eine Übertragungsreduzierung nach LoRaWAN Specification V1.0.2, chapter 18.4
uint8_t confirmedNbTrials = 4;



// Werte für die Wägezelle
// =======================

#include "HX711.h" //scale
HX711 scale;

uint8_t dataPin = 26;   // Der HX711 dataPin ist am GPIO 26
uint8_t clockPin = 48;  // der HX711 clockPin ist am GPIO 48
int int_nestweight = 0; // Integer-Variable für das Gewicht



// Subroutine Aktivieren der Waage und Aufbereiten der Payload
// ===========================================================
static void prepareTxFrame( uint8_t port )
{

// Waage nach dem Energiesparmodus initialisieren  
Serial.begin(115200);

Serial.println("wakeup scale");

scale.begin(dataPin, clockPin); // Anschluss der Kabel übergeben
scale.set_scale(1000.795471);   // Eichwert für leere Waage, wird vorher separat ermittelt, dann eingetragen    
scale.set_offset(4294624289);   // Eichwert für belastete Waage, wird vorher separat ermittelt, dann eingetragen

delay(5000); // Abwarten bis die Waage bereit ist
  
if (scale.is_ready())
  
  {
  Serial.println("scale is ready");
  Serial.println(scale.get_units(1));
  int_nestweight = abs(scale.get_units(1)*10); // Werte mit einer Nachkommastelle zu Integer aufbereiten
    if (int_nestweight > 3000) // Werte über 300 g sind Fehlmessungen und werden mit 9999 gekennzeichnet
    {
    int_nestweight = 9999;
    }
  }
  
else
  {
    int_nestweight = 9999; // Die Waage ist nicht bereit, wird wie eine Fehlmessung mit 9999 gelennzeichnet
    Serial.println("scale not ready");
  }

Serial.println(int_nestweight);

// Die Payload wird verpackt
appDataSize = 2; //für 16 bit integer
appData[0] = int_nestweight >> 8; // Schreiben der ersten 8 bit ans Ende
appData[1] = int_nestweight;      // Nachschieben der zweiten 8 bit

}



// Diese setup Schleife wird nur beim ersten Aufruf des Programmes durchlaufen
// =====================================================================================

void setup() {
  Serial.begin(115200);
  
  Mcu.begin();
  deviceState = DEVICE_STATE_INIT;

  scale.begin(dataPin, clockPin);
  scale.set_scale(1000.795471);       
  scale.set_offset(4294624289);

  
// Die Wägezelle wird auch in ihrer Subroutine aktiviert
// dies ist notwendig da wir aus dem Energiesparmodus kommen 
}





// Diese Schleife wird immer abgearbeitet
// ======================================
void loop()

{
  //prepareTxFrame( appPort )
  
  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {
#if(LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass,loraWanRegion);
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      prepareTxFrame( appPort ); // Aufrufen der Waage und Packen der  Payload
      LoRaWAN.send();            // Senden
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep(loraWanClass);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}



// Passender Payload Decoder 
// =========================

// Diese Java-Funktion nicht hier aktivieren sonder bei TheThingsNetwork einrichten.
/* 
  function decodeUplink(input) {
  
  var weight = input.bytes[0] << 8 | input.bytes[1];
  var errormessage = 'fehlmessung';
  
  if ( weight < 1000 ) {
    return {
      data: {
        nestgewicht_in_gramm: weight/10
      }
    };
  }
  
  else {
    return {
      data: {
        nestgewicht_in_gramm: errormessage
      } 
    };
  }

}
*/
