// Biblioteka do połączenia się z wifi
#include <ESP8266WiFiMulti.h> 

// Biblioteka do wystawienia serwera mDNS
#include <ESP8266mDNS.h>

// Biblioteka do uruchomienia serwera http
#include <ESP8266WebServer.h>

// biblioteki do odczytu temperatury 
#include <OneWire.h>
#include <DallasTemperature.h>

// biblioteka ułatwiająca serwoanie statycznych plików na serwerze
#include "LittleFS.h"

// Pin w kótry wpięliśmy czujnik temperatury
const int oneWireBus = 4;  

// Klasa pozwalająca na odczyt temperatury z czujnika
OneWire oneWire(oneWireBus);

// Klasa ułatwiająca odczyt temperatury - możemy od razu odczytać wynik w stopniach
// zamiast czytac pojedyncze bity
DallasTemperature sensors(&oneWire);

ESP8266WiFiMulti wifiMulti;     // Klasa pozwalająca na połączenie do wifi

ESP8266WebServer server(80);    // WebServer słuchający na porcie 80
 
void handleRoot();              
void handleTemp();
void handleNotFound();
bool handleFileRead();

void setup(void){
  Serial.begin(115200);         // Ustaw Serial Monitor port 115200
  delay(10);
  Serial.println('\n');

  // dodawanie sieci wifi - połączy się z tą z najlepszym zasięgiem.
  // Mikrokontroler robiłem w domu, a jego docelowe miejsce jest u rodziców
  // więc to była całkiem wygodna opcja żeby nie musieć flashować kodu przy zmianie miejsca
  wifiMulti.addAP("Orange_Swiatlowod_E6E0", "super_secret_password1");   
  wifiMulti.addAP("OnePlus", "super_secret_password2");
  wifiMulti.addAP("domek", "super_secret_password3");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Czekamy na połączenie do WIFI
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Nazwa sieci do której się połączyliśmy
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Adres IP naszego serwera http

  if (MDNS.begin("esp8266")) {              // Uruchomienie lokalnego serwera MDNS o adresie esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  // rejestrowanie handlerów dla danych urli
  server.on("/", HTTP_GET, handleRoot);     
  server.on("/temp", HTTP_GET, handleTemp);

  // jeśli nie znalazło danego URL sprawdź czy jest plik - jeśli tak to go zwróc, jeśli nie zwróć 404
  server.onNotFound([]() {                              
    if (!handleFileRead(server.uri()))                  
      handleNotFound();
  });     

  server.begin();                           // rozpocznij nasłuch
  Serial.println("HTTP server started");

   // uruchomienie helpera dla czytania plików
   if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Pobierze MME dla pliku
  if (LittleFS.exists(path)) {                          // Jeśli istnieje
    File file = LittleFS.open(path, "r");               // otwórz plik
    size_t sent = server.streamFile(file, contentType); // zwróc jego zawartość
    file.close();                                       // zamknij plik
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                     
}


void loop(void){
  // nasłuchujemy na requesty http 
  server.handleClient();
}

void handleRoot() { // przekierowanie na index.html
  server.sendHeader("Location","index.html");
  server.send(303);
}

// pobierz temperature i wyślij ją jako tekst
void handleTemp() {
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  server.send(200, "text/html", String(temperatureC));
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}
