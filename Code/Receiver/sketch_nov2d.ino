#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// Pines de control del motor
#define ENA 22          // PWM motor izquierdo
#define MOTOR_DER1 19   // Motor derecho
#define MOTOR_DER2 18   // Motor derecho
#define ENB 23          // PWM motor derecho
#define MOTOR_IZQ1 4    // Motor izquierdo
#define MOTOR_IZQ2 15   // Motor izquierdo

// Pines de la bomba de agua

// Sensores infrarrojos
#define SENSOR_INFRAROJO_DR 16  // Derecho
#define SENSOR_INFRAROJO_IZ 21   // Izquierdo
#define SENSOR_INFRAROJO_CENTRO 12 // Central

// Estructura para recibir datos
typedef struct struct_message {
    float value;    // Valor del sensor
    int sensor;     // Identificador del sensor (0 para sensor 1, 1 para sensor 2)
    int infraRojo;  // Estado del sensor infrarrojo
} struct_message;

struct_message receivedData;

enum Seguidor {
    IZQUIERDA,
    DERECHA,
    CONTINUAR,
    STOP,
    ATRAS,
    DERECHA1,
};

Seguidor Sensores_infrarojos;

int valorHumedad1 = 0;
bool sensorInfrarojo1 = false;
bool actionTriggered = false; // Nueva bandera para rastrear si la acción ya fue activada

void setup() {
    Serial.begin(115200);
       // Configuración de pine
    ledcAttach(ENA, 5000, 12);  // Configura el pin ENA para PWM de 12 bits
    ledcAttach(ENB, 5000, 12);  // Configura el pin ENB para PWM de 12 bits
  //  ledcAttach(ENA1, 5000, 12); // Configura el control de la bomba

    pinMode(MOTOR_DER1, OUTPUT);
    pinMode(MOTOR_DER2, OUTPUT);
    pinMode(MOTOR_IZQ1, OUTPUT);
    pinMode(MOTOR_IZQ2, OUTPUT);
    
    
    pinMode(SENSOR_INFRAROJO_DR, INPUT);
    pinMode(SENSOR_INFRAROJO_IZ, INPUT);
    pinMode(SENSOR_INFRAROJO_CENTRO, INPUT); // Habilita el sensor central

    WiFi.mode(WIFI_STA);

    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Registrar el callback de recepción
    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("ESP-NOW Receiver Ready");
}

void loop() {
    // Leer los valores de los sensores y determinar la dirección
    Sensores_infrarojos = Direccion(SENSOR_INFRAROJO_IZ, SENSOR_INFRAROJO_CENTRO, SENSOR_INFRAROJO_DR);

    // Ajustar la velocidad de los motores según la dirección detectada
    ajustarVelocidadMotores(Sensores_infrarojos);
    
    // Verifica si la acción debe ser activada
    if (valorHumedad1 > 8 && sensorInfrarojo1 == true) {
        Serial.println("Action triggered based on received data.");
        valorHumedad1 = 0; // Reiniciar el valor
        sensorInfrarojo1 = false; // Reiniciar el estado
        actionTriggered = true; // Marcar la acción como activada
        ajustarVelocidadMotores(STOP);
        delay(5000);
    }
}

// Callback que se ejecuta cuando se reciben datos
void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len) {
    // Copia los datos entrantes en la estructura receivedData
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    // Imprime el identificador del sensor y el valor recibido
    Serial.print("Received data from sensor: ");
    Serial.println(receivedData.sensor);

    // Manejar los datos recibidos
    if (receivedData.sensor == 1) {
        // Datos del Sensor 1
        Serial.print("Sensor 1 average value: ");
        Serial.println(receivedData.value);
        valorHumedad1 = receivedData.value; // Almacenar el valor de humedad
    } 
     
    if (receivedData.sensor == 2) {
        // Datos del Sensor 2
        Serial.print("Sensor 2 average value: ");
        Serial.println(receivedData.value);
    }

    // Manejar el estado del sensor infrarrojo
    if (receivedData.infraRojo == 1) {
        sensorInfrarojo1 = true;
        Serial.println("Infrared sensor detected arrival.");
    }
}


// Función para leer los sensores y determinar la dirección
Seguidor Direccion(int pinIzquierdo, int pinCentro, int pinDerecho) {
    int izquierda = digitalRead(pinIzquierdo);
    int centro = digitalRead(pinCentro);
    int derecha = digitalRead(pinDerecho);

    Serial.print("Izquierda: "); Serial.println(izquierda);
    Serial.print("Centro: "); Serial.println(centro);
    Serial.print("Derecha: "); Serial.println(derecha);
    
    // Lógica adaptada para que LOW significa que el seguidor está sobre la línea negra
    if (izquierda == LOW && centro == HIGH && derecha == LOW) {
        Serial.println("Continuar");
        return CONTINUAR;  // Mover hacia adelante
    } 
    if (izquierda == HIGH && centro == HIGH && derecha == LOW) {
        Serial.println("Vuelta Izquierda");
        return DERECHA;  // Girar a la izquierda
    } 
    if (izquierda == LOW && centro == HIGH && derecha == HIGH) {
        Serial.println("Vuelta Derecha");
        return IZQUIERDA;  // Girar a la derecha
    } 
    if (izquierda == HIGH && centro == HIGH && derecha == HIGH) {
        Serial.println("Parado");
        return STOP;  // Detener
    } 
    if (izquierda == LOW && centro == LOW && derecha == LOW){
        Serial.println("ajuste");
        return DERECHA1;
    }
    if (izquierda == LOW && centro == LOW && derecha == HIGH){
        Serial.println("ajuste");
        return DERECHA1;
    }

    return STOP; // Por defecto, si no se cumplen las condiciones
}

// Función para ajustar la velocidad de los motores según la dirección detectada
void ajustarVelocidadMotores(Seguidor direccion) {
    int velocidadBase = 2800;  // Velocidad base para ambos motores (12 bits)
    int velocidadIzquierda;
    int velocidadDerecha;
    bool Derecha = false;

    // Asignación de dirección y velocidad de motores
    switch (direccion) {
        case DERECHA:
            velocidadIzquierda = velocidadBase ;  // Aumenta la velocidad del motor izquierdo
            velocidadDerecha = 595;  // Detiene el motor derecho
            break;
        case IZQUIERDA:
            velocidadIzquierda = 595;  // Detiene el motor izquierdo
            velocidadDerecha = velocidadBase ;  // Aumenta la velocidad del motor derecho
            break;
        case CONTINUAR:
            velocidadIzquierda = velocidadBase ;  // Velocidad normal para el motor izquierdo
            velocidadDerecha = velocidadBase ;  // Velocidad normal para el motor derecho
            break;
        case STOP:
            velocidadIzquierda = 0;  // Detiene ambos motores
            velocidadDerecha = 0;
            break;
        case DERECHA1:
            velocidadIzquierda = 3000;
            velocidadDerecha = 1800;
            Derecha = true;
            break;    
    }

    if (Derecha) 
    {
      digitalWrite(MOTOR_IZQ1, HIGH);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, HIGH);
      digitalWrite(MOTOR_DER2, LOW);
    }
    else {// Establecer dirección y velocidad de los motores
      digitalWrite(MOTOR_IZQ1, HIGH);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, HIGH);
    }

    ledcWrite(ENA, velocidadIzquierda);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
    ledcWrite(ENB, velocidadDerecha);    // Estable
}