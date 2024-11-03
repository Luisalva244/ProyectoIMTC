#include <esp_now.h>
#include <WiFi.h>

#define SENSORHUMEDAD_1 34      /* Define the pin for the first sensor */
#define SENSORHUMEDAD_2 35      /* Define the pin for the second sensor */

// Structure to hold data
typedef struct struct_message {
    float value;         // Sensor value
    int sensor;        // Sensor identifier
    int infraRojo;
} struct_message;



struct_message myData;
esp_now_peer_info_t peerInfo;

float totalSensor1 = 0;
float totalSensor2 = 0;


int sensor1 = 0;
int sensor2 = 0;

// MAC Address of the receiving ESP32 - edit as needed
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0xAA, 0xF5, 0xA4};

// Create an instance of the web server on port 80
WebServer server(80);

void setup() 
{
    Serial.begin(115200);
    

    pinMode(2, OUTPUT);
    pinMode(SENSORHUMEDAD_1, INPUT);  // Set the sensor pin as input
    pinMode(SENSORHUMEDAD_2, INPUT);  // Set the second sensor pin as input
    pinMode(15, INPUT);  // Set the second sensor pin as input
    WiFi.mode(WIFI_STA);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(OnDataSent);

    // Register the peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
	
    // Start the web server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() 
{
     bool estadoSensor1 = false;
     bool estadoSensor2 = false;
     bool estadoSensor3 = false;
     bool estadoSensor4 = false;

    // Read sensor values 10 times and accumulate
    for (int i = 0; i < 10; i++) 
    {
      totalSensor1 += 10; //readSensor(SENSORHUMEDAD_1); // Read value from sensor 1
      totalSensor2 += readSensor(SENSORHUMEDAD_2); // Read value from sensor 2
    }
   

    if (totalSensor1 >= 100)
    {
      // Calculate average values
      myData.sensor = 1;  // Identifying sensor 1
      myData.value = totalSensor1 / 10.0; // Average value of sensor 1
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
        digitalWrite(2, HIGH);  // Turn on the LED if data was sent successfully
      } else {
        Serial.println("Failed to send data for sensor 1");
      }
    }  else 
    {
      myData.sensor = 1;  // Identifying sensor 1
      myData.value =  0; // Average value of sensor 1
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
        digitalWrite(2, HIGH);  // Turn on the LED if data was sent successfully
      } else {
        Serial.println("Failed to send data for sensor 1");
      }
    }

    if (totalSensor2 > 100)
    {
      // Send accumulated data for sensor 2
      myData.sensor = 2;  // Identifying sensor 2
      myData.value = totalSensor2 / 10.0; // Average value of sensor 2
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
        digitalWrite(2, LOW);
      } else {
        Serial.println("Failed to send data for sensor 2");
     }
    } else 
    {
      myData.sensor = 2;  // Identifying sensor 1
      myData.value =  0; // Average value of sensor 1
      myData.infraRojo = 0;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
        digitalWrite(2, HIGH);  // Turn on the LED if data was sent successfully
        delay(500);
      } else {
        Serial.println("Failed to send data for sensor 1");
      }
    }
    
    
    do 
    {
     sensor1 = readInfrarojo(15);
     if (sensor1 == 0)
     {
      myData.sensor = 1;
      myData.value =  0;
      myData.infraRojo = 1;
      if (esp_now_send(peerInfo.peer_addr, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
        digitalWrite(2, HIGH); 
        delay(500); // Turn on the LED if data was sent successfully
        digitalWrite(2, LOW);
      }
      estadoSensor1 = true;
     }

    } while(estadoSensor1 == false);  
    
      
    totalSensor1 = 0;
    totalSensor2 = 0;
    // Add any other logic you want to execute
   
}

// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Function to read the sensor value from the given pin
int readSensor(int pin) {
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


void handleRoot()
 {
    String html = "<html><body><h1>ESP32 Sensor Data</h1>";
    html += "<p>Sensor 1 Value: " + String(myData.value) + "</p>";
    html += "<p>Sensor 2 Value: " + String(totalSensor2 / 10.0) + "</p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html); // Send HTML response
}