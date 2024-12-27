#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// Pines de control del motor
#define ENA 22          // PWM motores derecha
#define MOTOR_DER1 19   
#define MOTOR_DER2 18   

#define ENB 23          // PWM motores izquierda
#define MOTOR_IZQ1 4    
#define MOTOR_IZQ2 15   

// Pines de la bomba de agua
#define ENA1 17
#define BOMBA_N 25
#define BOMBA_P 26

// Sensores infrarrojos
#define SENSOR_INFRAROJO_DR 16  // Derecho
#define SENSOR_INFRAROJO_IZ 21 // Izquierdo
#define SENSOR_INFRAROJO_CENTRO 12 // Central

// Estructura de datos
typedef struct {
    float value;    // Valor del sensor
    int sensor;     // Identificador del sensor
    int infraRojo;  // Estado del sensor infrarrojo
} struct_message;

struct_message receivedData;

// Estados del seguidor
enum Seguidor { IZQUIERDA, DERECHA, CONTINUAR, STOP, ATRAS, DERECHA1 };
Seguidor Sensores_infrarojos;

// Variables de sensores y estados
const int NUM_SENSORES = 4;
bool sensorInfrarojo[NUM_SENSORES] = {false, false, false, false};
int sensorDetected[NUM_SENSORES] = {0, 0, 0, 0};
int valorHumedad[NUM_SENSORES] = {0, 0, 0, 0};
bool actionTriggered[NUM_SENSORES] = {false, false, false, false};

void setup() {
    Serial.begin(115200);
    ledcAttach(ENA, 5000, 12);
    ledcAttach(ENB, 5000, 12);
    ledcAttach(ENA1, 5000, 12);

    pinMode(MOTOR_DER1, OUTPUT);
    pinMode(MOTOR_DER2, OUTPUT);
    pinMode(MOTOR_IZQ1, OUTPUT);
    pinMode(MOTOR_IZQ2, OUTPUT);

    pinMode(BOMBA_N, OUTPUT);
    pinMode(BOMBA_P, OUTPUT);

    pinMode(SENSOR_INFRAROJO_DR, INPUT);
    pinMode(SENSOR_INFRAROJO_IZ, INPUT);
    pinMode(SENSOR_INFRAROJO_CENTRO, INPUT);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Receiver Ready");
}

void loop() {
    do {
        Sensores_infrarojos = Direccion(SENSOR_INFRAROJO_IZ, SENSOR_INFRAROJO_CENTRO, SENSOR_INFRAROJO_DR);
        ajustarVelocidadMotores(Sensores_infrarojos);

        for (int i = 0; i < NUM_SENSORES; i++) {
            if (valorHumedad[i] < 8 && sensorInfrarojo[i] && sensorDetected[i] <= 3 && !actionTriggered[i]) {
                detenerMotores();
                Serial.println("Action triggered for sensor " + String(i + 1));
                valorHumedad[i] = 0;
                sensorInfrarojo[i] = false;
                actionTriggered[i] = true;
                delay(6000);
            } else if (valorHumedad[i] > 8 && sensorInfrarojo[i] && sensorDetected[i] <= 3 && !actionTriggered[i]) {
                detenerMotores();
                prenderBomba();
                Serial.println("Action with pump for sensor " + String(i + 1));
                valorHumedad[i] = 10;
                sensorInfrarojo[i] = false;
                actionTriggered[i] = true;
                delay(6000);
            }
        }
    } while (!todosSensoresInactivos());

    reiniciarContadores();
}

void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    int sensorIdx = receivedData.sensor - 1;
    if (sensorIdx >= 0 && sensorIdx < NUM_SENSORES) {
        valorHumedad[sensorIdx] = receivedData.value;
        sensorDetected[sensorIdx]++;
        if (receivedData.infraRojo == sensorIdx + 1) {
            sensorInfrarojo[sensorIdx] = true;
        }
    }
}

Seguidor Direccion(int pinIzquierdo, int pinCentro, int pinDerecho) {
    int izquierda = digitalRead(pinIzquierdo);
    int centro = digitalRead(pinCentro);
    int derecha = digitalRead(pinDerecho);

    if (izquierda == LOW && centro == HIGH && derecha == LOW) return CONTINUAR;
    if (izquierda == HIGH && centro == HIGH && derecha == LOW) return IZQUIERDA;
    if (izquierda == LOW && centro == HIGH && derecha == HIGH) return DERECHA;
    if (izquierda == LOW && centro == LOW && derecha == LOW) return DERECHA1;
    if (izquierda == LOW && centro == LOW && derecha == HIGH) return DERECHA1;
    if (izquierda == HIGH && centro == LOW && derecha == LOW) return DERECHA;

    return STOP;
}

void ajustarVelocidadMotores(Seguidor direccion) {
    int velocidadBase = 4095;
    int velocidadIzquierda = 0;
    int velocidadDerecha = 0;
    bool direccionDerecha = false;

    switch (direccion) {
        case DERECHA:
            velocidadIzquierda = velocidadBase;
            velocidadDerecha = 995;
            break;
        case IZQUIERDA:
            velocidadIzquierda = 995;
            velocidadDerecha = velocidadBase;
            break;
        case CONTINUAR:
            velocidadIzquierda = velocidadBase;
            velocidadDerecha = velocidadBase;
            break;
        case STOP:
            velocidadIzquierda = 0;
            velocidadDerecha = 0;
            break;
        case DERECHA1:
            velocidadIzquierda = velocidadBase;
            velocidadDerecha = velocidadBase;
            direccionDerecha = true;
            break;
    }

    if (direccionDerecha) {
        configurarMotores(HIGH, LOW, HIGH, LOW);
    } else {
        configurarMotores(HIGH, LOW, LOW, HIGH);
    }

    ledcWrite(ENA, velocidadIzquierda);
    ledcWrite(ENB, velocidadDerecha);
}

void configurarMotores(int izq1, int izq2, int der1, int der2) {
    digitalWrite(MOTOR_IZQ1, izq1);
    digitalWrite(MOTOR_IZQ2, izq2);
    digitalWrite(MOTOR_DER1, der1);
    digitalWrite(MOTOR_DER2, der2);
}

void detenerMotores() {
    configurarMotores(LOW, LOW, LOW, LOW);
    ledcWrite(ENA, 0);
    ledcWrite(ENB, 0);
}

void prenderBomba() {
    detenerMotores();
    digitalWrite(BOMBA_N, HIGH);
    digitalWrite(BOMBA_P, LOW);
    ledcWrite(ENA1, 4095);
    delay(8000);
    digitalWrite(BOMBA_N, LOW);
    digitalWrite(BOMBA_P, LOW);
}

bool todosSensoresInactivos() {
    for (int i = 0; i < NUM_SENSORES; i++) {
        if (sensorInfrarojo[i]) return false;
    }
    return true;
}

void reiniciarContadores() {
    for (int i = 0; i < NUM_SENSORES; i++) {
        sensorDetected[i] = 0;
    }
}
