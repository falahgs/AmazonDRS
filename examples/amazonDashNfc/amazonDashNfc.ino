/*******************************************************************
 Check out the included Arduino sketches and the getting started
 guide here!
 https://github.com/andium/AmazonDRS

 This is an Arduino implementation of an Amazon Dash Replenishment
 device. It currently supports the critical API endpoints necessary
 for registering a device and submitting replenishment requests. This
 library is tightly coupled to the WiFi101 library, which means it will
 work great with the Arduino MKR1000, Adafruit Feather MO w/ the ATWINC1500,
 Arduino WiFi101 shiled or anywhere the WiFi library is supported. Json
 parsing is provided via ArduinoJson, thanks bblanchon!
 https://github.com/bblanchon/ArduinoJson

 Written by Brian Carbonette Copyright © 2017 Andium

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Andres Sabas @ Electronic Cats added ESP8266 support @ 2017

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 *******************************************************************/

#ifdef ESP8266 
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#else
#include <WiFi101.h>
#endif

#include "AmazonDRS.h"
#include "AnduinoNFC.h"

AmazonDRS DRS = AmazonDRS();
AnduinoNFC NFC = AnduinoNFC();


//WiFi creds ----------------------------------------------------------------------------------
char ssid[] = "";    // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
//------------------------------------------------------------------------------------------------------

int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
   }

   #ifdef ESP8266 
   WiFiClientSecure client;
   #else
   WiFiSSLClient client;
   #endif

  //Start up DRS
  DRS.begin(&client);

  //connect to WiFi
  Serial.println("Initializing DRS... connecting to WiFi");
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    Serial.println(".");
    delay(3000);
    status = WiFi.status();
  }

  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());

  //initialize slots
  DRS.retrieveSubscriptionInfo();  //check slot statuses

  
  //Setup NFC
  NFC.begin();

}

void loop() {

  //scan nfc tag containing slotId
  //if scanned id matches a valid slot and the slot is available
  //request replenishment for the supplied slot

   if (NFC.packetReady())
    {
        Serial.println("Packet Ready!");
        NfcTag tag = NFC.read();  //attempt to read the RFID tag
        tag.print();              //and print the results to the terminal
        NdefMessage message = tag.getNdefMessage();
        NdefRecord record = message.getRecord(0); //grab the bits that contain the DRS Slot ID

        int payloadLength = record.getPayloadLength();
        byte payloadBytes[payloadLength];
        record.getPayload(payloadBytes);
        String payloadString = "";  //store the RFID msg bits in a String for comparison

        for(int i=3; i<payloadLength; i++)
        {
          payloadString += (char)payloadBytes[i]; //load up the cmp string with payload less the encoding
        }

        if(true)    //eventually if(slotId[i] has a match and slotStatus[i] is available
        {
            //we have a match! replenish the products associated with that slot!
            DRS.requestReplenishmentForSlot(payloadString);

            //Switch back to PN532 friendly SPI settings
            SPI.setBitOrder(LSBFIRST);
            SPI.setClockDivider(16); 
        }
        else
        {
          Serial.print("Sorry, slot ");
          Serial.print(payloadString);
          Serial.println(" is not available at this time");
        }

     
    }


}