#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SENSORHUMEDAD_1 33      
#define SENSORHUMEDAD_2 32      
#define SENSORHUMEDAD_3 35
#define SENSORHUMEDAD_4 34

#define SENSOR_INFRAROJO_1 4
#define SENSOR_INFRAROJO_2 16
#define SENSOR_INFRAROJO_3 17
#define SENSOR_INFRAROJO_4 5

#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


float totalSensor[4] = {0, 0, 0, 0};  // Arreglo que contiene los valores de los 4 sensores
bool envioExitoso = false;  // Variable para indicar si el envío fue exitoso
int reintentosMaximos = 100;   // Número máximo de reintentos


typedef struct struct_message {
    float value;       
    int sensor;        
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

// Create an instance of the web server on port 80
//Server Server(80);

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

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(OnDataSent);
   
    Wire.begin(OLED_SDA,OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
      Serial.println(F("No se pudo encontrar la pantalla OLED."));
      while (true);
    }
    display.clearDisplay();

    // Register the peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
sadasdas
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() 
{
     bool estadoSensor1 = false;
     bool estadoSensor2 = false;
     bool estadoSensor3 = false;
     bool estadoSensor4 = false;


     for (int i = 1; i < 5; i++) 
     {
        totalSensor[i-1] = 0;  
        for (int j = 0; j < 10; j++) 
        {
          totalSensor[i-1] = readSensor(i + 31);  
        }
        totalSensor[i-1] /= 10.0; 
        Serial.print("Promedio Sensor ");
        Serial.print(i );
        Serial.print(": ");
        Serial.println(totalSensor[i]);
     }



     for(int i = 1; i < 5 ; i++ )
     {
       if (totalSensor[i-1] > 8)
       {
         sendSensorData(totalSensor[i-1], i);
       }
     }
     
     showText(totalSensor[0],totalSensor[1],totalSensor[2],totalSensor[3]); 
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
     } else {
      myData.sensor = 1;
      myData.value =  totalSensor[0];
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor1 = false;
     }
     
     if (sensor2 == 0 && estadoSensor2 == false)
     {
      myData.sensor = 2;
      myData.value =  0;
      myData.infraRojo = 2;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor2 = true;
     } else {
      myData.sensor = 2;
      myData.value =  totalSensor[1];
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor2 = false;
     }
     
     if (sensor3 == 0 && estadoSensor3 == false)
     {
      myData.sensor = 3;
      myData.value =  0;
      myData.infraRojo = 3;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor3 = true;
     } else {
      myData.sensor = 3;
      myData.value =  totalSensor[2];
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor3 = false;
     }

     if (sensor4 == 0 && estadoSensor4 == false)
     {
      myData.sensor = 4;
      myData.value =  0;
      myData.infraRojo = 4;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor4 = true;
     } else {
      myData.sensor = 4;
      myData.value =  totalSensor[3];
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      }
      estadoSensor4 = false;
     }

    } while(estadoSensor1 == false && estadoSensor2 == false && estadoSensor3 == false && estadoSensor4 == false);  
    
      

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        envioExitoso = true; 
        Serial.println("Data sent successfully.");
    } else {
        envioExitoso = false;  
        Serial.println("Failed to send data.");
    }
}

int readSensor(int pin) {
    int value = analogRead(pin);  
    Serial.print("Sensor value from pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(value);        
    return value;
}


int readInfrarojo (int pin) 
{
    int value = digitalRead(pin);  
    Serial.print("Sensor value from pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(value);        
    return value; 
}


void sendSensorData(float totalSensor, int sensorID) {
    myData.sensor = sensorID;
    myData.value = totalSensor / 10.0;  // Promedio de los valores
    myData.infraRojo = 0;  

    int intentos = 0;
    while (!envioExitoso && intentos < reintentosMaximos) {
        if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
            Serial.printf("Attempt %d: Data sent successfully for Sensor %d\n", intentos + 1, sensorID);
        } else {
            Serial.printf("Attempt %d: Failed to send data for Sensor %d\n", intentos + 1, sensorID);
        }
        

        intentos++;
        delay(100);  
    }


    if (envioExitoso) {
      Serial.printf("Sensor %d: Data was successfully received by the other ESP32.\n", sensorID);
    } else {
      Serial.printf("Sensor %d: Failed to send data after %d attempts.\n", sensorID, reintentosMaximos);
    }
    
    envioExitoso = false;
}



void showText(float line1, float line2, float line3, float line4) 
{    
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("Sensor 1: %.2f", line1);
  display.setCursor(0, 16);
  display.printf("Sensor 2: %.2f", line2);
  display.setCursor(0, 32);
  display.printf("Sensor 3: %.2f", line3);
  display.setCursor(0, 48);
  display.printf("Sensor 4: %.2f", line4);
 
 
  display.display();
}
