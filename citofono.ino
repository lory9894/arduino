#include <ESP8266WiFi.h>
 
const char* ssid = "TIM-89519835";
const char* password = "200E04D910600D4";
const int releCancello = 16; //D0
const int releCancelletto = 12;

WiFiServer server(80);
void wifiConnection() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connesso");
 
  // avvio del server
  server.begin();
  Serial.println("Server Avviato");
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void networkSwitch() {
  // controlla se qualcuno e collegato
  WiFiClient client = server.available();
 
  if (client) {
    // Aspetta finche non arriva una richiesta
    Serial.println("new client");
 
    // legge la prima linea della richiesta
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();
 
    // Controlla la richiesta
    if (request.indexOf("/CANCELLETTO") != -1) {
      Serial.println("apertura cancelletto");
      digitalWrite(releCancelletto,0);
      delay(100);
      digitalWrite(releCancelletto,1);
      
    }
    if (request.indexOf("/CANCELLO") != -1) {
      Serial.println("apertura cancello");
      digitalWrite(releCancello,0);
      delay(1000);
      digitalWrite(releCancello,1);
      
    }
 
    // Risposta del server
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
 
    client.print("/CANCELLO  per aprire il cancello grande \n /CANCELLETTO per quello piccolo");
 
  }
}
void setup() {
  pinMode(releCancello, OUTPUT);
  digitalWrite(releCancello,1);
  pinMode(releCancelletto, OUTPUT);
  digitalWrite(releCancelletto,1);
  Serial.begin(9600);
  wifiConnection();
}

void loop() {
 networkSwitch();
}
