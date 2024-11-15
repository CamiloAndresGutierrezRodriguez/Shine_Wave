//Dimmer Code
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Galaxy";
const char* password = "Digitales24";

AsyncWebServer server(80);
int number = 0; // Variable para almacenar el número


const int interruptPin = 13;  // Pin GPIO donde se conecta el botón
const int pinSCR = 27;
volatile int level = 1050;  // Pin para controlar el SCR
const int delta = 10;
volatile bool interruptFlag = false;  // Variable que se activa en la interrupción
volatile int counter = 0;
unsigned long lastDebounceTime = 0;  // Última vez que se detectó un cambio
unsigned long debounceDelay = 3000;  // Tiempo de debounce (3 segundos)
volatile int inputPercentage = 0;

// Manejador de la interrupción con debounce
void IRAM_ATTR handleInterrupt() {
  unsigned long currentTime = micros();  // Obtener el tiempo actual en microsegundos
  if (currentTime - lastDebounceTime > debounceDelay) {
    interruptFlag = true;            // Cambia el estado cuando se detecta la interrupción
    lastDebounceTime = currentTime;  // Actualiza el tiempo de la última interrupción válida
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinSCR, OUTPUT);                                                        // Configura el pin del SCR como salida
  pinMode(interruptPin, INPUT_PULLUP);                                            // Configura el pin del botón con resistencia pull-up
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);  // Configura la interrupción


  
  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  
  Serial.println("Conectado a WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Ruta para la página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html lang=\"es\">";
    html += "<head><meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<style> body { font-family: Arial, sans-serif; text-align: center; } ";
    html += "form { display: inline-block; margin-top: 20px; } ";
    html += "input[type='number'] { width: 100px; } </style></head>";
    html += "<body>";
    html += "<h1>Ingrese un número (0-100)</h1>";
    html += "<form action=\"/submit\" method=\"POST\">";
    html += "<input type=\"number\" name=\"number\" min=\"0\" max=\"100\" required>";
    html += "<input type=\"submit\" value=\"Enviar\">";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Ruta para manejar el formulario
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("number", true)) {
      number = request->getParam("number", true)->value().toInt();
      Serial.print("Número recibido: ");
      Serial.println(number);
      
      // Mostrar nuevamente el formulario después de recibir el número
      String html = "<!DOCTYPE html><html lang=\"es\">";
      html += "<head><meta charset=\"UTF-8\">";
      html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
      html += "<style> body { font-family: Arial, sans-serif; text-align: center; } ";
      html += "form { display: inline-block; margin-top: 20px; } ";
      html += "input[type='number'] { width: 100px; } </style></head>";
      html += "<body>";
      html += "<h2>Número guardado: " + String(number) + "</h2>";
      html += "<form action=\"/submit\" method=\"POST\">";
      html += "<input type=\"number\" name=\"number\" min=\"0\" max=\"100\" required>";
      html += "<input type=\"submit\" value=\"Enviar\">";
      html += "</form>";
      html += "</body></html>";
      request->send(200, "text/html", html);
    } else {
      request->send(400, "text/html", "No se recibió ningún número");
    }
  });

  // Iniciar servidor
  server.begin();
}

void loop() {
  level = map(number, 0, 100, 1050, 85);  // Mapea el porcentaje a nivel
  if (interruptFlag) {
    Serial.print("Nivel actual: ");
    Serial.print(level);
    Serial.print(" - Contador: ");
    Serial.print(counter);
    Serial.print(" - inputper: ");
    Serial.println(number);
    counter = 0;
    interruptFlag = false;  // Reinicia la bandera de interrupción
  }

  if (counter == level) {
    digitalWrite(pinSCR, HIGH);  // Enciende el SCR
  }

  if (counter >= level + delta) {
    digitalWrite(pinSCR, LOW);  // Apaga el SCR
  }
  

  counter++;  // Incrementa el contador
}
