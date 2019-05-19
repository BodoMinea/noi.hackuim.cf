#include <ESP8266WiFi.h>
#include <String.h>
#include <DNSServer.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <DoubleResetDetect.h>
#include <SoftwareSerial.h>

String line;
SoftwareSerial mySerial(D1, D2);

DoubleResetDetect drd(5.0, 0x00);

WiFiManager wifiManager;
WiFiClient client;

const char* host = "directapi.hackuim.cf";
const int httpsPort = 80;
const char* key = "abcde";

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.print("DEBUG: Entering AP+CP mode - IP: ");
  Serial.print(WiFi.softAPIP());Serial.print(" on SSID ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  String id1 = String(ESP.getChipId(), HEX);
  String id2 = String(ESP.getFlashChipId()+ESP.getChipId(), HEX);
  Serial.begin(9600);
  mySerial.begin(4800);
  if (drd.detect()) {
    Serial.println("DEBUG: Resetting wifi config");
    wifiManager.resetSettings();
  }
  wifiManager.autoConnect(("NECONFIGURAT-"+id1).c_str(),"Floricele123");
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect()) {
    Serial.println("ERROR: Wi-Fi connection failed!");
    ESP.reset();
    delay(1500);
  }
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void sendToServer(String data){
  String uidSTR=getValue(data, ',', 0);
  String amountSTR=getValue(data, ',', 1);
  if (!client.connect(host, httpsPort)) {
        Serial.println("ERROR: connection failed");
        return;
      }
                   
    String data2 = "uid="+uidSTR+"&amount="+amountSTR;

     Serial.print("Requesting POST: ");
     client.println("POST /transaction/create HTTP/1.1");
     client.println("Host: directapi.hackuim.cf");
     client.println("Accept: */*");
     client.println("login-token: abcde");
     client.println("Content-Type: application/x-www-form-urlencoded");
     client.print("Content-Length: ");
     client.println(data2.length());
     client.println();
     client.print(data2);
    
    data = "";
    
    while (client.connected()) {
        line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }
      while (client.available()) {
        data += char(client.read());
      }
      client.stop();
      data.trim();
      mySerial.println(data);
      Serial.println(data);
}

void loop() {
  if(mySerial.available()){
    String tmp = mySerial.readString();
    Serial.println(tmp);
      sendToServer("3061936775,-1.3");
    if (Serial.available()) {
      Serial.write(Serial.read());
    }
  }
}
