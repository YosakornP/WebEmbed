#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>  

#define SPI_FLASH_SEC_SIZE 1024

#define DEFAULT_AP_SSID  "Salapond_2.4G"
#define DEFAULT_AP_PASS  "love0844356554"

String apssid = DEFAULT_AP_SSID;
String appass = DEFAULT_AP_PASS;
String myapssid = "Esp-Salapond"; 
String myappass = "12345678";

WebServer server(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "text/plain", "hello from esp32!");
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApSetup(){
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <title>Access Point Connection</title> <style>body{font-family: 'Courier New', Courier, monospace;}.input-group{padding: 5px;}.button-group{padding: 5px;}input{font-family: inherit; width: 250px; padding: 5px; border: solid 1px gray; border-radius: 10px;}button{font-family: inherit; padding: 5px; border: solid 1px gray; border-radius: 10px;}</style></head><body align='center'> <div class='input-group'> <label>SSID: </label><input id='ssid'> </div><div class='input-group'> <label>Pass: </label><input id='pass'> </div><div class='button-group'> <button id='reloadButton'>Reload</button> <button id='submitButton'>Submit</button> </div></body><script>document.getElementById('reloadButton').onclick=function(){console.log('Reload button is click...'); var xmlHttp=new XMLHttpRequest(); xmlHttp.onreadystatechange=function(){if (xmlHttp.readyState==XMLHttpRequest.DONE){if (xmlHttp.status==200){var res=JSON.parse(xmlHttp.responseText); document.getElementById('ssid').value=res.ssid; document.getElementById('pass').value=res.pass; alert('Loader Finish')}else if (xmlHttp.status==500){alert('Loader Fail');}}}; xmlHttp.open('GET', '/ap');}; document.getElementById('submitButton').onclick=function(){console.log('Submit Button is click...'); var xmlHttp=new XMLHttpRequest(); xmlHttp.onreadystatechange=function(){if (xmlHttp.readyState==XMLHttpRequest.DONE){if (xmlHttp.status==200){alert('Sender Finish');}else if (xmlHttp.status==500){alert('Sender Fail');}}}; var data=JSON.stringify({ssid: document.getElementById('ssid').value, pass: document.getElementById('pass').value}); xmlHttp.open('POST', '/ap'); xmlHttp.send(data);}; document.getElementById('reloadButton').click();</script></html>");
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApGet(){
  digitalWrite(LED_BUILTIN, LOW);

  String str = "";
  str += "{";
  str += "\"ssid\":\"" + apssid + "\", ";
  str += "\"pass\": \"" + appass + "\"";
  str += "}";
  
  server.send(200, "text/json", str); 
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleApPost(){
  digitalWrite(LED_BUILTIN, LOW);

  if (server.args() != 1){

    server.send(400, "text/plain", "argument Error!!"); 
  }
  else {
    String str = server.arg(0);
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, str);

    if (err) {
      server.send(500, "text/plain", "server Error!!");
    }
    else {
      String _apssid = doc["ssid"].as<String>();
      String _appass = doc["pass"].as<String>();
      server.send(200, "text/plain", "success");

      if(_apssid != apssid || _appass != appass){
        apssid = _apssid;
        appass = _appass;
        eepromWrite();
      }
    }
  }
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void eepromWrite(){

  char c;
  int addr = 0;
  unsigned char s, i;
  
  EEPROM.begin(SPI_FLASH_SEC_SIZE);

  c = '@'; EEPROM.put(addr, c); addr ++;
  c = '$'; EEPROM.put(addr, c); addr ++;

  s = (unsigned char)apssid.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++){
    c = apssid[i]; EEPROM.put(addr, c); addr ++;
  }

  s = (unsigned char)appass.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++){
    c = appass[i]; EEPROM.put(addr, c); addr ++;
  }

  EEPROM.end();
}

void eepromRead(){

  char c;
  int addr = 0;
  unsigned char s, i;
  
  EEPROM.begin(SPI_FLASH_SEC_SIZE);


  char header[3] = {' ', ' ' ,'\0'};
  EEPROM.get(addr, header[0]); addr ++;
  EEPROM.get(addr, header[1]); addr ++;

  if (strcmp(header, "@$") != 0){
   eepromWrite();
   return;
  } else {

    EEPROM.get(addr, s); addr ++;
    apssid = "";
    for(i = 0;  i < s; i ++){
      EEPROM.get(addr, c); apssid = apssid + c; addr ++;
    }

    EEPROM.get(addr, s); addr ++;
    appass = "";
    for(i = 0;  i < s; i ++){
      EEPROM.get(addr, c); appass = appass + c; addr ++;
    }
  }
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);

  eepromRead();
  
  WiFi.begin(apssid.c_str(), appass.c_str());
  Serial.println("Connect to " + apssid + "");

  int counter = 0;
  do {
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print(".");
    counter += 1;
  } while(WiFi.status() != WL_CONNECTED && counter < 10);
  Serial.println("");
  if (WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    Serial.println("Fail");
  }
  else{
    Serial.println("Success..");
    Serial.println("IP address(STA mode): " + (WiFi.localIP()).toString());
    if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
    }
  }
  
  char temp[10];
  uint64_t chipid = ESP.getEfuseMac();
  sprintf(temp, "%04X", (uint16_t)(chipid >> 32));
  myapssid = myapssid + "-[" + String(temp);
  sprintf(temp, "%08X", (uint32_t)chipid);
  myapssid = myapssid + String(temp) + "]";
  Serial.println("Ap ssid: " + myapssid);
  
  WiFi.softAP(myapssid.c_str(), myappass.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address(AP Mode): ");
  Serial.println(myIP);
  

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });


  server.on("/apsetup", handleApSetup);

  server.on("/ap", HTTP_GET, handleApGet);
  server.on("/ap", HTTP_POST, handleApPost);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  

  digitalWrite(LED_BUILTIN, LOW);
}

void loop(void) {
  
  server.handleClient();
  
  delay(2);
}
