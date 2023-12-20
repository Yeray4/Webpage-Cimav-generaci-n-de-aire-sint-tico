/*
Codigo Servidor Web para ESP32 para mostrar resultados


Discord por si hay dudas: Yeray#6132
*/

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <SD.h>
File myFile;
#include <SPI.h>
#include <ESP32Time.h>
ESP32Time rtc;
#include <Wire.h>
#include <freertos/task.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
WiFiClient client;
HTTPClient http;

String GetTemp_Pronos();
String GetHum_Pronos();
String GetTemp_Emular();
String GetHum_Emular();
void config_Wifi_mDns();
void config_rtc();
/// Configuracion wifi
const char *ssid = "CIMAV-Visita";      // Enter SSID
const char *password = "Investigacion"; // Enter Password
const char *ssid2 = "x_xz";
const char *password2 = "8831HGuadiana212@";
IPAddress localIP(192, 168, 1, 68);  //  dirección IP
IPAddress gateway(192, 168, 1, 254); // puerta de enlace
IPAddress subnet(255, 255, 255, 0);  // máscara de subred

// Configuracion Servidor NTP
/* const char *ntpServer = "mx.pool.ntp.org";
const char *ntpServer2 = "1.mx.pool.ntp.org";
const long gmtOffset_sec = -21600; // offset en segundos GMT-6 Durango Mexico
const int daylightOffset_sec = 0; */

// Colas
#define MAX_SIZE 24
float queue_Temp[MAX_SIZE];
int front1 = 0;
int rear1 = -1;
int itemCount1 = 0;

float queue_Hum[MAX_SIZE];
int front2 = 0;
int rear2 = -1;
int itemCount2 = 0;

void enqueue_Temp(float value);
float dequeue_Temp();
void enqueue_Hum(float value);
float dequeue_Hum();

///////////////////////////////////////////////////////////////////////////////////
#define EnableTxRs485 33
///////////////////////////////////////////////////////////////////////////////////

// funciones para UART Y TRAMA
unsigned long getUlong(byte packet[], byte i);
unsigned int getInt(byte packet[], byte i);
float getFloat(byte packet[], byte i);
void fromLongToBytes(byte *bytes, long ing);
void fromFloatToBytes(byte *bytes, float f);
void _tramaServerToControl();
void fragmentar_trama_Sensores(byte buffer[]);
void fragmentar_trama_recibida(byte buffer[]);
void fragmentar_trama_recibidaI2C(byte buffer[]);
int TryGetACK(int TimeOut);
bool al_cambiarTrama_enviar();
int ProccesACK(const int timeOut, void (*okCallBack)(), void (*errorCallBack)());
void okAction();
void recibir_trama_sensores();
int ProccesSerialData(const int timeOut, byte *buffer, const uint8_t bufferLength, void (*okCallBack)(), void (*errorCallBack)());
void okRecepcion();
void errorRecepcion();
byte estadoMx1 = 0;
int variador = 0;
char ACK = 'F';
char NAK = 'E';
byte sendData[18];
byte sendData2[30];
byte sendData3[58];
byte data[18]; 
const int timeOut = 50;

enum SerialState
{
  OKEY,
  ERROR,
  NO_RESPONSE
};

#define cs 5
String sinaloa = "/sinaloa1.csv";

void getClima_db(String estadoMX);
float TempHoraCambio_Sonora = 0;
float HumHoraCambio_Sonora = 0;
int currentHour = 0;
int currentMinute = 0;
int previousHour;
int8_t conteo_;

int ubi_pagina = 0;
int ubi_pagina_siguiente = 0;
int on_off_boton = 0;
int on_off_boton_anterior = 0;
float temperatura_estado_anterior = 0;
float humedad_estado_anterior = 0;
float temperatura_estadoMx_setpoint = 0;
float humedad_estadoMx_setpoint = 0;
String sensoresData;
float control_tempFinal = 0;
float control_humFinal = 0;
void i2c_recibir(int num ); 
void i2c_request();  
void _i2cTramaServerToControl();

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void setup()
{
  Wire.begin(20);// maestro I2C
  Wire.onReceive(i2c_recibir);
   Wire.onRequest(i2c_request);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // 16 es RX y 17 es TX
  Serial.setTimeout(500);
  Serial2.setTimeout(500);
  pinMode(EnableTxRs485, OUTPUT);
  // Imprimir mensaje de error al iniciar spiffs
  if (!SPIFFS.begin(true))
    Serial.println("An Error has occurred while mounting SPIFFS");

  config_Wifi_mDns();
  config_rtc();
  /* configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    rtc.setTimeStruct(timeinfo); // establece la fecha y hora almacenadas en la estructura timeinfo en el Real-Time Clock (RTC) del ESP32
    Serial.printf("RTC: Conectado correctamente a servidor NTP\n");
  }
  else
    Serial.printf("RTC: Failed to obtain time \n"); */

  Serial.print("Iniciando SD ...");
  if (SD.begin(cs))
    Serial.printf("Sd: inicializacion exitosa \n\n");
  else
    Serial.printf("Sd: inicializacion fallida \n\n");
  getClima_db(sinaloa); // extraer del SD los datos de temperatura y humedad de la base de datos

  // Seleccionando archivo index.html main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  // Seleccionando archivo index.css main page
  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.css"); });
  // Seleccionando archivo script.js main page
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.js"); });
  // Seleccionando archivo sensores.js main page
  server.on("/sensores.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/sensores.js"); });
  // Selecionando archivo sensores.html y cargando temperatura
  server.on("/sensores.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/sensores.html"); });
  // Selecionando archivo sensores.css
  server.on("/sensores.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/sensores.css"); });
  // Selecionando archivo extra.html
  server.on("/extra.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/extra.html"); });
  // Selecionando archivo extra.css
  server.on("/extra.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/extra.css"); });
  // yucantan
  server.on("/allyucatan.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/allyucatan.js"); });
  server.on("/yucatan.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/yucatan.html"); });
  // Selecionando archivo sinaloa.html
  server.on("/sinaloa.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/sinaloa.html"); });
  // Selecionando archivo sinaloa.css
  server.on("/all.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/all.css"); });
  // Selecionando archivo sinaloa.js
  server.on("/all.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/all.js"); });
  // Recibir variable de control en pagina sonora.html
  server.on("/ubi", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (request->hasParam("ubi")) {
    String ubiValue = request->getParam("ubi")->value();
    // Realiza las acciones correspondientes con el valor recibido
    ubi_pagina = ubiValue.toInt();
    request->send(200, "text/plain", "/ubi Valor recibido correctamente");
  } else {
    request->send(400, "text/plain", "/ubi Falta el parámetro 'ubi'");
  } });
  // Seleccionando archivo index.html para back page
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  // Pagina no encontrada
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(400, "text/plain", "Not found"); });
  // Seleccionando archivo Cimav.png
  server.on("/cimav.png", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/cimav.png", "image/png"); });
  // Subiendo temperatura a pagina sensores.html
  server.on("/GetTemp_Pronos", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", GetTemp_Pronos()); });
  // Subiendo humedad relativa a pagina sensores.html
  server.on("/GetHum_Pronos", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", GetHum_Pronos()); });
  // Parar de emular al ser presionado el boton
  server.on("/Emular", HTTP_GET, [](AsyncWebServerRequest *request) { // 1 = on
    if (request->hasParam("estado"))
    {
      String myValue = request->getParam("estado")->value();
      // Utiliza el valor recibido como desees
      Serial.println("on  emular: " + myValue);
      on_off_boton = myValue.toInt();
      request->send(200, "text/plain", "Valor emular recivido correctamente ");
    }
    else
    {
      request->send(400, "text/plain", "Falta el parámetro emular");
    }
  });
  server.on("/StopEmular", HTTP_GET, [](AsyncWebServerRequest *request) { // 0 = off
    if (request->hasParam("estado"))
    {
      String myValue = request->getParam("estado")->value();
      // Utiliza el valor recibido como desees
      Serial.println("off stopemular " + myValue);
      on_off_boton = myValue.toInt();
      request->send(200, "text/plain", "Valor recibido correctamente");
    }
    else
    {
      request->send(400, "text/plain", "Falta el parámetro 'estado'");
    }
  });
  /* server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", sensoresData); }); */
  // Subiendo temperatura a pagina sensores.html
  server.on("/GetTemp_Emular", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", GetTemp_Emular()); });
  // Subiendo humedad relativa a pagina sensores.html
  server.on("/GetHum_Emular", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", GetHum_Emular()); });

  previousHour = rtc.getHour(true);
  TempHoraCambio_Sonora = dequeue_Temp(); // ingreso de dato de temperatura y humedad de la cola
  HumHoraCambio_Sonora = dequeue_Hum();
  _tramaServerToControl(); // envio de datos a control para que tenga un valor/referencia inicial. Normalmente Estada:0, Boton:0, Temp:xx, Hum:xx

  /* server.addHandler(&webSocket);
  webSocket.onEvent(onWebSocketEvent); */

  // Inciar el servidor
  server.begin();
  Serial.printf("Servidor HTTP iniciado\n\n");
}

void loop()
{
  currentHour = rtc.getHour(true);
  // currentMinute = rtc.getMinute();
  if (currentHour != previousHour)
  { // para al momento de cambiar la hora se actualicen los datos de temperatura y humedad
    conteo_++;
    previousHour = currentHour;
    TempHoraCambio_Sonora = dequeue_Temp();
    HumHoraCambio_Sonora = dequeue_Hum();
    Serial.printf("Hora %d , min %d \n", currentHour, currentMinute);
    Serial.println("Hora actualizada y datos de temperatura y humedad actualizados");
    if (conteo_ == 23)
    {
      getClima_db(sinaloa); // reabastecer la cola de datos de temperatura y humedad al vaciarse la cola
      conteo_ = 0;
    }
  }


  al_cambiarTrama_enviar();//funcion adaptada PARA I2C


  // USAR PARA UART
  /* if (al_cambiarTrama_enviar() == true)  
  {
    ProccesACK(timeOut, okAction, _tramaServerToControl); // procesar el ACK de control . Tiempo de espera, accion a realizar si es okey, funcion a realizar si es error
  }  */                                                     

 
}

/// i2c
void i2c_request() {
  
  _i2cTramaServerToControl();
}
void i2c_recibir(int num ){
 
   while (Wire.available()) {
    for (int i = 0; i < 18; i++) {
      data[i] = Wire.read();  // Leer cada byte recibido y almacenarlo en el arreglo
    }
  }
  Serial.printf("\n\t I2C TRAMA recibida \n ");
  fragmentar_trama_recibidaI2C(data);

}
void _i2cTramaServerToControl(){
    byte seleccion_pagina[4];
  byte boton[4];
  byte temp[4];
  byte hum[4];

  // hacer la desfragmentacion  de bytes necesaria para enviar la trama
  fromLongToBytes(seleccion_pagina, ubi_pagina);
  fromLongToBytes(boton, on_off_boton);
  fromFloatToBytes(temp, TempHoraCambio_Sonora);
  fromFloatToBytes(hum, HumHoraCambio_Sonora);

  sendData[0] = 'I';                 // ascii 73
  sendData[1] = seleccion_pagina[0]; // para saber en que pagina se encuentra el usuario
  sendData[2] = seleccion_pagina[1];
  sendData[3] = seleccion_pagina[2];
  sendData[4] = seleccion_pagina[3];

  sendData[5] = boton[0]; // para saber si el boton esta encendido o apagado
  sendData[6] = boton[1];
  sendData[7] = boton[2];
  sendData[8] = boton[3];

  sendData[9] = temp[0];
  sendData[10] = temp[1];
  sendData[11] = temp[2];
  sendData[12] = temp[3];

  sendData[13] = hum[0];
  sendData[14] = hum[1];
  sendData[15] = hum[2];
  sendData[16] = hum[3];
  sendData[17] = ACK;  // ascii 70
  sendData[18] = '\n'; // ascii 10

  // impresion();
  
  Wire.write(sendData, 18);

}
//////////////////////  Subir Datos al  SERVIDOR WEB ////////////////////////
String GetTemp_Pronos() // Subir la temperatura pronosticada dependiendo de la pagina abierta
{
  if (ubi_pagina == 1)
    return String(TempHoraCambio_Sonora);

  return "_";
}
String GetHum_Pronos() // Subir la humedad pronosticada dependiendo de la pagina abierta
{
  if (ubi_pagina == 1) // 1 es Sonora
    return String(HumHoraCambio_Sonora);

  return "_";
}
String GetTemp_Emular() // Subir la humedad emulada dependiendo de la pagina abierta
{

  return String(control_tempFinal);
}
String GetHum_Emular() // Subir la humedad emulada dependiendo de la pagina abierta
{

  return String(control_humFinal);
}
//////////////CONFIGURACION DE WIFI/////////////////////
void config_Wifi_mDns()
{
  // Conectarse a la red WiFi
  Serial.println("Connecting to Wifi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");
  // Imprimir IP local en el Monitor Serie
  // WiFi.config(localIP, gateway, subnet);
  Serial.println(WiFi.localIP());

  // CONFIG MDNS
  /*  if (!MDNS.begin("emuladorcimav"))
   {
     Serial.println("Error setting up MDNS responder!");
     while (1)
     {
       delay(1000);
     }
   }
   Serial.printf("mDNS responder started. Nombre para acceder: emuladorcimav.local \n");
   MDNS.addService("http", "tcp", 80); */
}
/////////////// EXTRAER DATOS DE LA SD ///////////////////////
void getClima_db(String estadoMX)
{
  int d = rtc.getDayofYear(); //
  int h = rtc.getHour(true);

  int horaBuscada = h + (d * 24); // hora actual
  int diaBuscado = d + 1;         // dia actual

  Serial.printf("SD: diaBuscado: %d, horaBuscada: %d \n", diaBuscado, horaBuscada);
  // Buscar los datos de temperatura y humedad a partir de la hora y día actual
  myFile = SD.open(sinaloa, FILE_READ); // abrimos  el archivo
  if (myFile)
    Serial.printf("SD: Archivo abierto correctamente \n");
  else
    Serial.printf("SD_: Error al abrir el archivo \n");

  String line;
  while (myFile.available())
  {
    line = myFile.readStringUntil('\n');
    line.trim();

    // Dividir la línea en sus componentes: día, hora, temperatura, humedad
    int day = line.substring(0, line.indexOf(',')).toInt();
    line = line.substring(line.indexOf(',') + 1);
    int hour = line.substring(0, line.indexOf(',')).toInt();

    if (day == diaBuscado && hour == horaBuscada)
    {
      // Encontrado el primer dato correspondiente al día y hora actual
      float temperature = line.substring(line.indexOf(',') + 1, line.lastIndexOf(',')).toFloat();
      float humidity = line.substring(line.lastIndexOf(',') + 1).toFloat();

      // Mostrar el primer dato
      Serial.print("Primer dato: Temperatura=");
      Serial.print(temperature);
      Serial.print(", Humedad=");
      Serial.println(humidity);

      // Guardar el primer dato en la cola
      enqueue_Temp(temperature);
      enqueue_Hum(humidity);

      // Extraer los siguientes 23 datos
      for (int i = 0; i < 23; i++)
      {
        if (!myFile.available())
        {
          Serial.println("No hay suficientes datos disponibles.");
          break;
        }

        line = myFile.readStringUntil('\n');
        line.trim();

        // Extraer temperatura y humedad
        temperature = line.substring(line.indexOf(',', line.indexOf(',') + 1) + 1, line.lastIndexOf(',')).toFloat();
        /* Al agregar line.indexOf(',', line.indexOf(',') + 1) como el primer índice en line.substring(), se busca el siguiente
        carácter ',' después de la posición del primer carácter ','. */
        humidity = line.substring(line.lastIndexOf(',') + 1).toFloat();

        // Guardar los siguientes datos en la cola
        enqueue_Temp(temperature);
        enqueue_Hum(humidity);

        // Mostrar los datos extraídos
        Serial.print("Dato ");
        Serial.print(i + 1);
        Serial.print(": Temperatura=");
        Serial.print(temperature);
        Serial.print(", Humedad=");
        Serial.println(humidity);
      }

      break;
    }
  }

  myFile.close();
}
/////////////// COLAS ///////////////////////
void enqueue_Temp(float value)
{
  if (itemCount1 < MAX_SIZE)
  {
    rear1 = (rear1 + 1) % MAX_SIZE;
    queue_Temp[rear1] = value;
    // Serial.printf("insertando queue[%d] = %d \n", rear1, value);
    itemCount1++;
  }
  else
  {
    // Cola llena, no se puede agregar más elementos
    Serial.println("La cola está llena.");
  }
}
float dequeue_Temp()
{
  if (itemCount1 > 0)
  {
    float value_temp_cola = queue_Temp[front1];
    front1 = (front1 + 1) % MAX_SIZE;
    itemCount1--;
    return value_temp_cola;
  }
  else
  {
    // Cola vacía, no se puede extraer ningún elemento
    Serial.println("La cola está vacía.");
    return -1; // Otra forma de indicar un error
  }
}
void enqueue_Hum(float value)
{
  if (itemCount2 < MAX_SIZE)
  {
    rear2 = (rear2 + 1) % MAX_SIZE;
    queue_Hum[rear2] = value;
    // Serial.printf("insertando queue[%d] = %d \n", rear2, value);
    itemCount2++;
  }
  else
  {
    // Cola llena, no se puede agregar más elementos
    Serial.println("La cola está llena.");
  }
}
float dequeue_Hum()
{
  if (itemCount2 > 0)
  {
    float value = queue_Hum[front2];
    front2 = (front2 + 1) % MAX_SIZE;
    itemCount2--;
    return value;
  }
  else
  {
    // Cola vacía, no se puede extraer ningún elemento
    Serial.println("La cola está vacía.");
    return -1; // Otra forma de indicar un error
  }
}
/////////////// FUNCIONES PARA OBTENER TRAMA POR UART ///////////////////////
void recibir_trama_sensores()
{
  digitalWrite(EnableTxRs485, LOW); // habilita la recepcion de datos por el puerto serial
  if (Serial2.available() > 0)
  {

    byte buffer[50];
    size_t n = Serial2.readBytesUntil('\n', buffer, 50); // devuelve el numero de bytes leidos
    if (n <= 49)
    {

      Serial.printf("Procesando trama sensores\n");
      ProccesSerialData(timeOut, buffer, 50, okRecepcion, errorRecepcion);
      for (int8_t i = 0; i < n; i++)
      {
        Serial.print(buffer[i]);
        Serial.print(" ");
      }
    }
  }
}
////////////////// CONVERSIONES PARA TRAMA ///////////////////////
// FRAGMENTAR TRAMA
unsigned long getUlong(byte packet[], byte i)
{
  // big endian
  unsigned long value = 0;
  value = (value * 256) + packet[i];
  value = (value * 256) + packet[i + 1];
  value = (value * 256) + packet[i + 2];
  value = (value * 256) + packet[i + 3];
  return value;
}
unsigned int getInt(byte packet[], byte i)
{
  unsigned int value = 0;
  value = (value * 256) + packet[i];
  return value;
}
float getFloat(byte packet[], byte i)
{
  union tag
  {
    byte bin[4];
    float num;
  } u;

  u.bin[0] = packet[i];
  u.bin[1] = packet[i + 1];
  u.bin[2] = packet[i + 2];
  u.bin[3] = packet[i + 3];
  return u.num;
}
// DESFRAGMETAR TRAMA
void fromFloatToBytes(byte *bytes, float f)
{
  int length = sizeof(float);
  for (int i = 0; i < length; i++)
    bytes[i] = ((byte *)&f)[i];
}
void fromLongToBytes(byte *bytes, long ing)
{
  bytes[0] = (byte)((ing & 0xff000000) >> 24);
  bytes[1] = (byte)((ing & 0x00ff0000) >> 16);
  bytes[2] = (byte)((ing & 0x0000ff00) >> 8);
  bytes[3] = (byte)((ing & 0x000000ff));
}
///////////////////// FUNCIONES envio y recepcion de trama DE TRAMA //////////////
void fragmentar_trama_Sensores(byte buffer[])
{

  float t1 = getFloat(buffer, 1);
  Serial.println("");
  Serial.print("t1: ");
  Serial.println(t1);

  float h1 = getFloat(buffer, 5);
  Serial.print("h1: ");
  Serial.println(h1);

  float t2 = getFloat(buffer, 9);
  Serial.print("t2: ");
  Serial.println(t2);

  float h2 = getFloat(buffer, 13);
  Serial.print("h2: ");
  Serial.println(h2);

  float t3 = getFloat(buffer, 17);
  Serial.print("t3: ");
  Serial.println(t3);

  float h3 = getFloat(buffer, 21);
  Serial.print("h3: ");
  Serial.println(h3);

  float t4 = getFloat(buffer, 25);
  Serial.print("t4: ");
  Serial.println(t4);

  float h4 = getFloat(buffer, 29);
  Serial.print("h4: ");
  Serial.println(h4);

  float t5 = getFloat(buffer, 33);
  Serial.print("t5: ");
  Serial.println(t5);

  float h5 = getFloat(buffer, 37);
  Serial.print("h5: ");
  Serial.println(h5);

  float dstemp = getFloat(buffer, 41);
  Serial.print("termisor: ");
  Serial.println(dstemp);

  float fhviento = getFloat(buffer, 45);
  Serial.print("fh400  viento: ");
  Serial.println(fhviento);

  /* float fhtemp = getFloat(buffer, 49);
  Serial.print("fh temp: ");
  Serial.println(fhtemp);

  float fhhum = getFloat(buffer, 53);
  Serial.print("fh hum: ");
  Serial.println(fhhum);
  Serial.println(); */
  sensoresData = String(t1) + "," +
                 String(t2) + "," +
                 String(t3) + "," +
                 String(t4) + "," +
                 String(t5) + "," +
                 String(h1) + "," +
                 String(h2) + "," +
                 String(h3) + "," +
                 String(h4) + "," +
                 String(h5) + "," +
                 String(3) + "," +
                 String(fhviento);

  /*  webSocket.textAll(sensoresData); */
}
void fragmentar_trama_recibida(byte buffer[])
{
  ubi_pagina = getUlong(buffer, 1);
  Serial.printf("\n ubi_pagina: %d \n", ubi_pagina); 


  on_off_boton = getUlong(buffer, 5);
  Serial.printf("boton en : %d \n", on_off_boton); 
  

  temperatura_estadoMx_setpoint = getFloat(buffer, 9);
  Serial.printf("temp recibida: %0.2f \n", temperatura_estadoMx_setpoint);
 

  humedad_estadoMx_setpoint = getFloat(buffer, 13);
  Serial.printf("hum recibida: %0.2f \n", humedad_estadoMx_setpoint);
  
}
void fragmentar_trama_recibidaI2C(byte buffer[])
{
  int ubi_paginai2c = getUlong(buffer, 1);
  Serial.printf("\n ubi_pagina: %d \n", ubi_paginai2c); 


 int  on_off_botoni2c = getUlong(buffer, 5);
  Serial.printf("boton en : %d \n", on_off_botoni2c); 
  

  control_tempFinal = getFloat(buffer, 9);
  Serial.printf("temp recibida: %0.2f \n", control_tempFinal);
 

  control_humFinal = getFloat(buffer, 13);
  Serial.printf("hum recibida: %0.2f \n", control_humFinal);
  
}
void _tramaServerToControl()
{ // FUNCION PARA DESFRAGMETAR TRAMA DE 19 BYTES
  byte seleccion_pagina[4];
  byte boton[4];
  byte temp[4];
  byte hum[4];

  // hacer la desfragmentacion  de bytes necesaria para enviar la trama
  fromLongToBytes(seleccion_pagina, ubi_pagina);
  fromLongToBytes(boton, on_off_boton);
  fromFloatToBytes(temp, TempHoraCambio_Sonora);
  fromFloatToBytes(hum, HumHoraCambio_Sonora);

  sendData[0] = 'I';                 // ascii 73
  sendData[1] = seleccion_pagina[0]; // para saber en que pagina se encuentra el usuario
  sendData[2] = seleccion_pagina[1];
  sendData[3] = seleccion_pagina[2];
  sendData[4] = seleccion_pagina[3];

  sendData[5] = boton[0]; // para saber si el boton esta encendido o apagado
  sendData[6] = boton[1];
  sendData[7] = boton[2];
  sendData[8] = boton[3];

  sendData[9] = temp[0];
  sendData[10] = temp[1];
  sendData[11] = temp[2];
  sendData[12] = temp[3];

  sendData[13] = hum[0];
  sendData[14] = hum[1];
  sendData[15] = hum[2];
  sendData[16] = hum[3];
  sendData[17] = ACK;  // ascii 70
  sendData[18] = '\n'; // ascii 10

  // impresion();
  digitalWrite(EnableTxRs485, HIGH);
  Serial2.write(sendData, 18);
  vTaskDelay(pdMS_TO_TICKS(19)); // necesario para terminar de enviar toda la trama completa
 
 
}

bool al_cambiarTrama_enviar()
{ // al cambiar cualquier variable de control, temperatura o humedad enviar trama

  if (on_off_boton != on_off_boton_anterior)
  {
    //_tramaServerToControl(); PARA UART
    on_off_boton_anterior = on_off_boton;
    return true;
  }

  if (ubi_pagina != ubi_pagina_siguiente)
  {
    //_tramaServerToControl(); PARA UART
    ubi_pagina_siguiente = ubi_pagina;
    Serial.printf("pagina cambió: %d \n", ubi_pagina);
    return true;
  }

  if ((TempHoraCambio_Sonora != temperatura_estado_anterior) || (HumHoraCambio_Sonora != humedad_estado_anterior))
  {
    //_tramaServerToControl(); PARA UART
    temperatura_estado_anterior = TempHoraCambio_Sonora;
    humedad_estado_anterior = HumHoraCambio_Sonora;
    Serial.printf("temp y hum cambio \n");
    return true;
  }

  return false; // ninguno de los casos entonces no se ejecutara la funcion de ProccesACK
}

int TryGetACK(int TimeOut)
{

  digitalWrite(EnableTxRs485, LOW); // habilita la recepcion de datos rs485
  unsigned long StartTime = millis();
  while ((millis() - StartTime) < TimeOut)
  {
  }

  if (Serial2.available())
  {
    if (Serial2.read() == ACK)
      return OKEY;
    if (Serial2.read() == NAK)
      return ERROR;
  }
  return NO_RESPONSE;
}

int ProccesACK(const int timeOut, void (*okCallBack)(), void (*errorCallBack)())
{

  int rst = TryGetACK(timeOut);
  if (rst == OKEY)
  {
    if (okCallBack != NULL)
      okCallBack(); // se comprueba si la función de devolución de llamada okCallBack no es un puntero nulo
  }
  else if (rst == ERROR)
  {
    if (okCallBack != NULL)
      errorCallBack();
    Serial.printf("Reenviado Trama por error en recepcion\n");
  }
  else if (rst == NO_RESPONSE)
  {
    errorCallBack();
    Serial.printf("No se recibio respuesta, reenviar\n");
  }
  return rst;
}

void okAction()
{
  Serial.printf("\n ACK recibido por el otro esp32 \n");
}

int TryGetSerialData(int TimeOut, byte *buffer) // encontrar ACK en el buffer y saber como proceder
{
  // intenta conseguir los delimitadores de la trama para verificar que la trama se recibió correctamente dentro de un tiempo
  //  de 100ms, si no se recibe la trama en ese tiempo, se devuelve un error
  //  si se recibe la trama, se devuelve un OK.
  //  el Ok sirve para proceder a fragmentar la trama
  unsigned long StartTime = millis();
  digitalWrite(EnableTxRs485, LOW); // habilita la recepcion de datos por el puerto serial
  while (!Serial2.available() && (millis() - StartTime) < TimeOut)
  {
  }

  if (buffer[0] == 'I' && buffer[49] == ACK)
    return OKEY;
  else
    return ERROR;
}

int ProccesSerialData(const int timeOut, byte *buffer, const uint8_t bufferLength, void (*okCallBack)(), void (*errorCallBack)())
{ // Comprueba que en el buffer se encuentre el ACK , recepciono de datos completa

  int rst = TryGetSerialData(timeOut, buffer);
  if (rst == OKEY)
  {
    Serial.print(ACK);
    if (okCallBack != NULL)
    {
      okCallBack(); // al comprobar ACK  envia ACK por UART para avisar correcta recepcion de datos y comienza a frangmentar la trama
      fragmentar_trama_Sensores(buffer);
      Serial.printf("  ACK enviado \n");
    }
  }
  else if (rst == ERROR)
  {
    if (okCallBack != NULL)
    {
      errorCallBack();                  // al comprobar que no esta ACK  envia NAK por UART para avisar incorrecta recepcion de datos
      digitalWrite(EnableTxRs485, LOW); // deshabilitar recepcion de trama
      Serial.printf("NAK enviado \n");
    }
  }
  return rst;
}

void okRecepcion()
{ // envia ACK por UART para avisar correcta recepcion de datos
  Serial.println("Recepcion correcta");
  digitalWrite(EnableTxRs485, HIGH);
  Serial2.write(ACK);
  Serial2.write(ACK);
}
void errorRecepcion()
{ // envia NAK por UART para avisar incorrecta recepcion de datos
  digitalWrite(EnableTxRs485, HIGH);
  Serial.println("Recepcion incorrecta");
  Serial2.write(NAK);
  Serial2.write(NAK);
}

void config_rtc()
{
  const String serverName = "http://worldtimeapi.org/api/timezone/America/Monterrey";
  http.begin(client, serverName);
  http.GET();

  StaticJsonDocument<768> doc;
  DeserializationError error = deserializeJson(doc, http.getStream());
  // falta filtrar el json
  long unixtime = doc["unixtime"];
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  http.end();
  Serial.printf("Unix time: %ld\n", unixtime);

  rtc.setTime(unixtime);
}