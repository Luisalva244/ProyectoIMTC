#include <esp_now.h>
#include <WiFi.h>

// Structure to receive data
typedef struct struct_message {
  int b = 0;         // Sensor value
  int sensor;    // Sensor identifier
} struct_message;

struct_message receivedData;
int sensor1Values[10] = {0};  // Array to store the last 10 sensor 1 values
int i = 0;  // Index to track sensor1Values array
bool isFilled = false;  // To track when the array has been filled with 10 values


// Callback function executed when data is received
void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Print the sensor identifier and value
  Serial.print("Received data from sensor: ");
  Serial.println(receivedData.sensor);

  if (receivedData.sensor == 0) {
    // Store sensor 1 values
    Serial.print("Sensor 1 value: ");
    Serial.println(receivedData.b);

    sensor1Values[i] = receivedData.b;  // Store the sensor 1 value in the array
    i++;

    // If the index reaches 10, reset it to 0 (circular buffer logic)
    if (i == 10) {
      i = 0;
      isFilled = true;  // Mark the array as filled after receiving the first 10 values
    }
    
    // If the array is filled, calculate the average
    if (isFilled) {
      float average = calculateAverage();
      Serial.print("Average of last 10 sensor 1 values: ");
      Serial.println(average);
      !isFilled;
      // Add your custom logic here after the average is calculated
      // For now, just print a message to indicate this is where your logic would go
      Serial.println("Custom logic based on the average can be added here.");
    }
  } else if (receivedData.sensor == 1) {
    Serial.print("Sensor 2 value: ");
    Serial.println(receivedData.b);
  }
}

void setup() {
  Serial.begin(115200);

  // Set ESP32 to station mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}








float calculateAverage() {
  int sum = 0;
  for (int j = 0; j < 10; j++) {
    sum += sensor1Values[j];
  }
  return sum / 10.0;  // Return the average as a float
}