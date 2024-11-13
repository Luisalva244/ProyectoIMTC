#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

#define SENSORHUMEDAD_1 33      /* Define the pin for the first sensor */
#define SENSORHUMEDAD_2 34      /* Define the pin for the second sensor */
#define SENSORHUMEDAD_3 35
#define SENSORHUMEDAD_4 32

#define SENSOR_INFRAROJO_1 5
#define SENSOR_INFRAROJO_2 17
#define SENSOR_INFRAROJO_3 16
#define SENSOR_INFRAROJO_4 4


float totalSensor[4] = {0, 0, 0, 0};  // Arreglo que contiene los valores de los 4 sensores
bool envioExitoso = false;  // Variable para indicar si el envío fue exitoso
int reintentosMaximos = 100;   // Número máximo de reintentos

// WiFi Credentials
const char* ssid = "INFINITUM84AF";
const char* password = "4tPVYEG7FE";
WiFiServer server(80);  // El servidor escucha en el puerto 80

// Structure to hold data
typedef struct struct_message {
    float value;         // Sensor value
    int sensor;        // Sensor identifier
    int infraRojo;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;
int sensor4 = 0;

// MAC Address of the receiving ESP32 - edit as needed
uint8_t broadcastAddress[] = {0xA0, 0xB7, 0x65, 0x22, 0xF5, 0x28};


void setup() 
{
    Serial.begin(115200);

    //Humidity sensor inputs
    pinMode(SENSORHUMEDAD_1, INPUT); 
    pinMode(SENSORHUMEDAD_2, INPUT);  
    pinMode(SENSORHUMEDAD_3, INPUT);  
    pinMode(SENSORHUMEDAD_4, INPUT);    

    //Infrarojo inputs
    pinMode(SENSOR_INFRAROJO_1, INPUT);
    pinMode(SENSOR_INFRAROJO_2, INPUT);
    pinMode(SENSOR_INFRAROJO_3, INPUT);
    pinMode(SENSOR_INFRAROJO_4, INPUT);  

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) 
    {
       Serial.println("Error initializing ESP-NOW");
       return;
    }

    // Register the send callback
    esp_now_register_send_cb(OnDataSent);

    // Register the peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) 
    {
       Serial.println("Failed to add peer");
       return;
    }

    // Setup web server handlers
    server.on("/", handle_OnConnect);
    
    server.begin();
    Serial.println("HTTP server started");
}

void loop() 
{
     bool estadoSensor1 = false;
     bool estadoSensor2 = false;
     bool estadoSensor3 = false;
     bool estadoSensor4 = false;

     for (int i = 1; i < 5; i++) 
     {
        totalSensor[i] = 0;  // Resetear el valor acumulado antes de la suma
        for (int j = 0; j < 10; j++) 
        {
          totalSensor[i] = readSensor(i+31); 
          delay(50);  // Esperar un poco entre lecturas para estabilizar
        }
        totalSensor[i] /= 10.0;  // Calcular el promedio
        Serial.print("Promedio Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(totalSensor[i]);
     }



     for(int i = 1; i < 5 ; i++ )
     {
       if (totalSensor[i] > 350)
       {
         sendSensorData(totalSensor[i], i);
         Serial.println("Hola");
         Serial.println(i);
         Serial.println(totalSensor[i]);
       }
     }

    server.handleClient();  // Gestionar las solicitudes HTTP

    do 
    {
     sensor1 = readInfrarojo(SENSOR_INFRAROJO_1);
     sensor2 = readInfrarojo(SENSOR_INFRAROJO_2);
     sensor3 = readInfrarojo(SENSOR_INFRAROJO_3);
     sensor4 = readInfrarojo(SENSOR_INFRAROJO_4);

     
     if (sensor1 == 0 && estadoSensor1 == false)
     {
      myData.sensor = 1;
      myData.value =  0;
      myData.infraRojo = 1;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor1 = true;
     }
     
     if (sensor2 == 0 && estadoSensor2 == false)
     {
      myData.sensor = 2;
      myData.value =  0;
      myData.infraRojo = 2;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor2 = true;
     } else 
     
     if (sensor3 == 0 && estadoSensor3 == false)
     {
      myData.sensor = 3;
      myData.value =  0;
      myData.infraRojo = 3;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor3 = true;
     }

     if (sensor4 == 0 && estadoSensor4 == false)
     {
      myData.sensor = 4;
      myData.value =  0;
      myData.infraRojo = 4;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor4 = true;
     }

    } while(estadoSensor1 == false && estadoSensor2 == false && estadoSensor3 == false && estadoSensor4 == false);  
    
      
    totalSensor1 = 0;
    totalSensor2 = 0;
    totalSensor3 = 0;
    totalSensor4 = 0;
    // Add any other logic you want to execute
}

// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    if (status == ESP_NOW_SEND_SUCCESS) 
    {
       envioExitoso = true;  // Marca el envío como exitoso
       Serial.println("Data sent successfully.");
    } else 
    {
       envioExitoso = false;  // Si falló, marcar como falso
       Serial.println("Failed to send data.");
    }
}

// Function to read the sensor value from the given pin
int readSensor(int pin) 
{
    int value = analogRead(pin);  // Read the analog value from the specified pin
    Serial.print("Sensor value from pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(value);        // Print the sensor value for debugging
    return value;
}


int readInfrarojo (int pin) 
{
    int value = digitalRead(pin);  // Read the analog value from the specified pin
    Serial.print("Sensor value from pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(value);        // Print the sensor value for debugging
    return value; 
}


void sendSensorData(float totalSensor, int sensorID) 
{
    myData.sensor = sensorID;
    myData.value = totalSensor / 10.0;  // Promedio de los valores
    myData.infraRojo = 0;  // Aquí puedes ajustar el estado si es necesario

    // Intentar enviar hasta que el envío sea exitoso o se alcance el número máximo de intentos
    int intentos = 0;
    while (!envioExitoso && intentos < reintentosMaximos) {
        if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
            Serial.printf("Attempt %d: Data sent successfully for Sensor %d\n", intentos + 1, sensorID);
            Serial.printf("Valor del sensor %.2f:", totalSensor);
        } else {
            Serial.printf("Attempt %d: Failed to send data for Sensor %d\n", intentos + 1, sensorID);
        }
        
        // Incrementar los intentos y esperar un poco antes de reintentar
        intentos++;
        delay(100);  // Espera
    }


    if (envioExitoso) {
      Serial.printf("Sensor %d: Data was successfully received by the other ESP32.\n", sensorID);
    } else {
      Serial.printf("Sensor %d: Failed to send data after %d attempts.\n", sensorID, reintentosMaximos);
    }

    // Resetear el estado de envioExitoso para el próximo envío
    envioExitoso = false;
}

void handle_OnConnect() 
{
    String html = "<html><body><h1>Sensor Data</h1>";
    html += "<p>Sensor 1: " + String(totalSensor[0]) + "</p>";
    html += "<p>Sensor 2: " + String(totalSensor[1]) + "</p>";
    html += "<p>Sensor 3: " + String(totalSensor[2]) + "</p>";
    html += "<p>Sensor 4: " + String(totalSensor[3]) + "</p>";
    html += "<br><br><a href='/start'>Start</a><br>";
    html += "<a href='/stop'>Stop</a>";
    html += "</body></html>";

    server.send(200, "text/html", html);  // Send the HTML response
}



