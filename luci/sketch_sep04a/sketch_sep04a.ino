#include <ESP8266WiFi.h>
 
const char* ssid = "TIM-89519835";
const char* password = "200E04D910600D4";
int timeout;
int rele = 16;
int light;
int on = 1;
int off = 0;
int state;
int button = 12; // D6
const int trigPin = 4;
const int echoPin = 5;
long timer = 0;
long sonarTimer = 0;
long timePassed = 0;
int onlyButton = 0;
int noLight = 0;
 
WiFiServer server(80);
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rele, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  digitalWrite(rele, off);
  state = off;
  wifiConnection();
}
 
void wifiConnection() {
  if (timePassed + 1000 <= millis()) {
    timeout++;
    timePassed = millis();
    Serial.println(timeout);
  }
  if (timeout > 15) {
    timeout = 0;
    return;
  }
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
 
void buttonPressed() {
  timer = 0;
  Serial.println("button pressed");
  if (state == on ) {
    Serial.println("turning it off");
    releSwitch();
  } else {
    Serial.println("turning it on");
    releSwitch();
    noLight=on;
  }
  timer = 0;
}
 
void releSwitch() {
  if (state == off) {
    digitalWrite(rele, on);
    state = on;
  } else {
    digitalWrite(rele, off);
    state = off;
  }
}
 
void tripwire() {
  timer = 0;
  light = analogRead(0);
  Serial.print("light value: ");
  Serial.println(analogRead(0));
  if (state == on ) {
    Serial.println("turning it off");
    releSwitch();
  } else {
    if (light < 300) {
      Serial.println("turning it on");
      releSwitch();
    } else Serial.println("there is already enough light");
  }
}
 
void networkSwitch() {
  // controlla se qualcuno e collegato
  WiFiClient client = server.available();
 
  if (client) {
    // Aspetta finche non arriva una richiesta
    Serial.println("new client");
    while (!client.available()) {
      if (timePassed + 1000 <= millis()) {
        timeout++;
        timePassed = millis();
        Serial.println(timeout);
      }
      if (timeout > 5) {
        Serial.println("Request timeout");
        timeout = 0;
        return;
      }
      delay(1);
    }
 
    // legge la prima linea della richiesta
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();
 
    // Controlla la richiesta
    if (request.indexOf("/ON") != -1) {
      digitalWrite(rele, on);
      state = on;
    }
    if (request.indexOf("/OFF") != -1) {
      digitalWrite(rele, off);
      state = off;
    }
    if (request.indexOf("/CHANGE") != -1) {
      if (state == on) {
        Serial.println("turning it off");
        releSwitch();
      } else {
        Serial.println("turning it on");
        noLight = on;
        releSwitch();
      }
    }
    if (request.indexOf("/PROX") != -1) {
      if (onlyButton) {
        onlyButton = off;
      } else {
        onlyButton = on;
      }
    }
    if (request.indexOf("/LUM") != -1) {
      if (noLight) {
        noLight = off;
      } else {
        noLight = on;
      }
    }
 
 
 
    // Risposta del server
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
 
    client.print("Stato luce : ");
 
    if (state) {
      client.print("On");
    } else {
      client.print("Off");
    }
    client.println("<br><br>");
    client.println("<a href=\"/CHANGE\">Cambia stato</a><br>");
    if (onlyButton)
      client.println("<a href=\"/PROX\">attiva prossimita' e luminosita'</a><br>");
    else
      client.println("<a href=\"/PROX\">disattiva prossimita' e luminosita'</a><br>");
    if (noLight)
      client.println("<a href=\"/LUM\">attiva controllo luminosita'</a><br>");
    else
      client.println("<a href=\"/LUM\">disattiva controllo luminosita'</a><br>");
 
    client.println("</html>");
    timeout = 0;
    Serial.println("Client Disconnesso");
    Serial.println("");
 
  }
}
 
void loop() {
  long duration;
 
  if (WiFi.status() == WL_CONNECTED)
    networkSwitch();
  else if (timer > 50)
    wifiConnection();
 
  if (digitalRead(button) && timer >= 1 ) buttonPressed();
  if (timer > 600 && noLight){
    noLight = off;
  }
 
  if (state) digitalWrite(LED_BUILTIN, LOW);
  else digitalWrite(LED_BUILTIN, HIGH);
  //timer updating and overflow control (ancora da fare overfolw controll)
  if (timePassed + 1000 <= millis()) {
    timer++;
    sonarTimer++;
    timePassed = millis();
  }
 
  // light and tripwire power
  if (!onlyButton) {
    //sonar calculation
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
 
 
    duration = pulseIn(echoPin, HIGH);
    if (duration > 116000 && sonarTimer > 5) {
      Serial.print(duration);
      Serial.print("microseconds");
      Serial.println();
      tripwire();
      sonarTimer = 0;
    }
 
    //solar light poweroff
    if (timer > 5 && !noLight) {
      light = analogRead(0);
      if (state == on && light > 700) {
        Serial.println("solar light poweroff");
        Serial.println("turning it off");
        Serial.print("light value: ");
        Serial.println(analogRead(0));
        digitalWrite(rele, off);
        state = off;
      }
    }
  }
}
