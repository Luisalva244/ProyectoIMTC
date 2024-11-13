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

#define ENA1 17
#define BOMBA_N 25
#define BOMBA_P 26

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
int valorHumedad2;
int valorHumedad3 = 0;
int valorHumedad4 = 0;

bool sensorInfrarojo1 = false;
bool sensorInfrarojo2 = false;
bool sensorInfrarojo3 = false;
bool sensorInfrarojo4 = false;

int sensorDetected1 = 0;
int sensorDetected2 = 0;
int sensorDetected3 = 0;
int sensorDetected4 = 0;


bool actionTriggered = false; // Nueva bandera para rastrear si la acción ya fue activada
bool action3 = false;
bool action2 = false;
bool action1 = false;

void setup() {
    Serial.begin(115200);
       // Configuración de pine
    ledcAttach(ENA, 5000, 12);  // Configura el pin ENA para PWM de 12 bits
    ledcAttach(ENB, 5000, 12);  // Configura el pin ENB para PWM de 12 bits
    ledcAttach(ENA1, 5000, 12);  // Configura el pin ENB para PWM de 12 bits

  //  ledcAttach(ENA1, 5000, 12); // Configura el control de la bomba

    pinMode(MOTOR_DER1, OUTPUT);
    pinMode(MOTOR_DER2, OUTPUT);
    pinMode(MOTOR_IZQ1, OUTPUT);
    pinMode(MOTOR_IZQ2, OUTPUT);
    
    pinMode(BOMBA_N, OUTPUT);
    pinMode(BOMBA_P, OUTPUT);    
    
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
   bool isMoving = false;
    // Leer los valores de los sensores y determinar la dirección
  do 
  {
    Sensores_infrarojos = Direccion(SENSOR_INFRAROJO_IZ, SENSOR_INFRAROJO_CENTRO, SENSOR_INFRAROJO_DR);

    // Ajustar la velocidad de los motores según la dirección detectada
    ajustarVelocidadMotores(Sensores_infrarojos);

    // Verifica si la acción debe ser activada
   if (valorHumedad1 < 8 && sensorInfrarojo1 == true && sensorDetected1 <= 3 && action1 == false) 
    {
      Serial.println("Action triggered based on received data. pl");
      valorHumedad1 = 0; // Reiniciar el valor
      sensorInfrarojo1 = false; // Reiniciar el estado     
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      Serial.println("Contador");
      Serial.println(sensorDetected1);
      sensorDetected1 = 10;
      action1 = true;
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);   
      delay(6000);
      Serial.println("Adios accion desde sensor 1");
    }

   if (valorHumedad1 > 8 && sensorInfrarojo1 == true && sensorDetected1 <= 2 && action1 == false) 
    {
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);  
      Serial.println("Action triggered based on received data.");
      valorHumedad1 = 10; // Reiniciar el valor
      sensorInfrarojo1 = false; // Reiniciar el estado     
      action1 = true;
      prenderBomba();
      Serial.println("Contador");
      Serial.println(sensorDetected1);
      delay(6000);
      Serial.println("Adios accion desde sensor 1");
      }
  
   if (valorHumedad2 < 8 && sensorInfrarojo2 == true && sensorDetected2 <= 3 && action2 == false) 
    {
      Serial.println("Action triggered based on received data.");
      valorHumedad2 = 0; // Reiniciar el valor
      sensorInfrarojo2 = false; // Reiniciar el estado     
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);   
      action2 = true;
      Serial.println("Contador 2 LOW");
      Serial.println(sensorDetected2);      
      delay(6000);
      Serial.println("Adios accion desde sensor 1");
    }

    if (valorHumedad2 > 8 && sensorInfrarojo2 == true && sensorDetected2 <= 3 && action2 == false) 
    {
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);  
      Serial.println("Action triggered based on received data.");
      valorHumedad2 = 10; // Reiniciar el valor
      sensorInfrarojo2 = false; // Reiniciar el estado
      prenderBomba();
      action2 = true;
      Serial.println("Bomba");
      Serial.println("Contador 2 HIGH");
      Serial.println(sensorDetected2);
      delay(6000);
      Serial.println("Adios accion desde sensor 2");
    }
   
   if (valorHumedad3 < 8 && sensorInfrarojo3 == true  && sensorDetected3 == 2 && action3 == false) 
    {
      Serial.println("Action triggered based on received data.");
      valorHumedad3 = 10; // Reiniciar el valor
      sensorInfrarojo1 = true; // Reiniciar el estado     
      action3 = true;
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);  
      Serial.println("Contador 3");
      Serial.println(sensorDetected3);
      sensorDetected3 = 10;
      Serial.println(sensorDetected3);
      Serial.println("SENSOR3 SS");
      delay(6000);
      Serial.println("Adios accion desde sensor 1");
    } 

    if (valorHumedad3 > 8 && sensorInfrarojo3 == true && sensorDetected3 == 2 && action3 == false) 
    {
      ajustarVelocidadMotores(STOP);
      Serial.println("Action triggered based on received data.");
      valorHumedad3 = 10; // Reiniciar el valor
      sensorInfrarojo3 = false; // Reiniciar el estado
      Serial.println("Contador 3");
      Serial.println(sensorDetected3);
      prenderBomba();
      action3 = true;
      ajustarVelocidadMotores(STOP);
      delay(6000);
      Serial.println("Adios accion desde sensor 3");
    }

   if (valorHumedad4 < 8 && sensorInfrarojo4 == true) 
    {
      Serial.println("Action triggered based on received data.");
      valorHumedad4 = 0; // Reiniciar el valor
      sensorInfrarojo1 = false; // Reiniciar el estado  
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, LOW);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, LOW);
      ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
      ledcWrite(ENB, 0);      
      Serial.println("Contador 4");
      Serial.println(sensorDetected4);
      delay(12000);
      Serial.println("Adios accion desde sensor 4");
    }

    if (valorHumedad4 > 8 && sensorInfrarojo4 == true) 
    {
      ajustarVelocidadMotores(STOP);
      Serial.println("Action triggered based on received data.");
      valorHumedad4 = 10; // Reiniciar el valor
      sensorInfrarojo4 = false; // Reiniciar el estado
      Serial.println("Contador 4");
      Serial.println(sensorDetected4);
      prenderBomba();
      ajustarVelocidadMotores(STOP);
      delay(12000);
      Serial.println("Adios accion desde sensor 4");
     }    
    }while (!sensorInfrarojo1 && !sensorInfrarojo2 && !sensorInfrarojo3 && !sensorInfrarojo4);

  
    sensorDetected1 = 0;
    sensorDetected2 = 0;
    sensorDetected3 = 0;
    sensorDetected4 = 0;
}

// Callback que se ejecuta cuando se reciben datos
void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len) {
    // Copia los datos entrantes en la estructura receivedData
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    // Imprime el identificador del sensor y el valor recibid
    // Manejar los datos recibidos
    if (receivedData.sensor == 1 && receivedData.value >= 8) 
    {
      // Datos del Sensor 1
      Serial.print("Sensor 1 average value: ");
      Serial.println(receivedData.value);
      valorHumedad1 = receivedData.value; // Almacenar el valor de humedad
      sensorDetected1++;
    } else if (receivedData.sensor == 1 && receivedData.value < 8)
    {
      Serial.print("Sensor 1 average value: ");
      Serial.println(receivedData.value);
      valorHumedad1 = receivedData.value; // Almacenar el valor de humedad
      sensorDetected1++;    
    }
     
    if (receivedData.sensor == 2 &&  receivedData.value >= 8) 
    {
      // Datos del Sensor 2
      Serial.print("Sensor 2 average value: 10");
      Serial.println(receivedData.value);
      valorHumedad2 = receivedData.value;
      sensorDetected2++;      
    } else if (receivedData.sensor == 2 && receivedData.value < 8)
    {
      Serial.print("Sensor 2 average value: ");
      Serial.println(receivedData.value);
      valorHumedad2 = 10; // Almacenar el valor de humedad
      sensorDetected2++;    
    }

    if (receivedData.sensor == 3 && receivedData.value > 8) 
    {
      // Datos del Sensor 2
      Serial.print("Sensor 3 average value: ");
      Serial.println(receivedData.value);
      valorHumedad3 = receivedData.value;
      sensorDetected3++;
    } else if (receivedData.sensor == 3 && receivedData.value < 8)
    {
      Serial.print("Sensor 3 average value: ");
      Serial.println(receivedData.value);
      valorHumedad3 = receivedData.value; // Almacenar el valor de humedad
      sensorDetected3++;    
    }

    if (receivedData.sensor == 4 && receivedData.value > 8) 
    {
      // Datos del Sensor 4
      Serial.print("Sensor 4 average value: ");
      Serial.println(receivedData.value);
      valorHumedad4 = receivedData.value;
      sensorDetected4++;
    } else if (receivedData.sensor == 4 && receivedData.value < 8)
    {
      Serial.print("Sensor 4 average value: ");
      Serial.println(receivedData.value);
      valorHumedad4 = receivedData.value; // Almacenar el valor de humedad
      sensorDetected4++;    
    }

    // Manejar el estado del sensor infrarrojo
    if (receivedData.infraRojo == 1  && valorHumedad1 > 8) 
    {
      sensorInfrarojo1 = true;
      Serial.println("Infrared sensor 1 detected arrival.");
      sensorDetected1++;
    } else if (receivedData.infraRojo == 1  && valorHumedad1 < 8)
    {
      sensorInfrarojo1 = true;
      Serial.println("Infrared sensor 1 detected arrival.");
      sensorDetected1++;      
    }

    if (receivedData.infraRojo == 2  && valorHumedad2 > 8) 
    {
      sensorInfrarojo2 = true;
      Serial.println("Infrared sensor 2 detected arrival.");
      sensorDetected2++;
    } else if (receivedData.infraRojo == 2  && valorHumedad2 < 8)
    {
      sensorInfrarojo2 = true;
      Serial.println("Infrared sensor 2 detected arrival.");
      sensorDetected2++;      
    }

    if (receivedData.infraRojo == 3  && valorHumedad3 > 8) 
    {
      sensorInfrarojo3 = true;
      Serial.println("Infrared sensor 3 detected arrival.");
      sensorDetected3++;
    } else if (receivedData.infraRojo == 3  && valorHumedad3 < 8)
    {
      sensorInfrarojo3 = true;
      Serial.println("Infrared sensor 3 detected arrival.");
      sensorDetected3++;      
    }

    if (receivedData.infraRojo == 4  && valorHumedad4 > 8) 
    {
      sensorInfrarojo4 = true;
      Serial.println("Infrared sensor 4 detected arrival.");
      sensorDetected4++;
    } else if (receivedData.infraRojo == 4  && valorHumedad4 < 8)
    {
      sensorInfrarojo4 = true;
      Serial.println("Infrared sensor 4 detected arrival.");
      sensorDetected4++;      
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
        return IZQUIERDA;  // Girar a la izquierda
    } 
    if (izquierda == LOW && centro == HIGH && derecha == HIGH) {
        Serial.println("Vuelta Derecha");
        return IZQUIERDA;  // Girar a la derecha
    } 
    if (izquierda == LOW && centro == LOW && derecha == LOW) {
        Serial.println("Parado");
        return DERECHA1;  // Detener
    } 
 /*   if (izquierda == LOW && centro == LOW && derecha == LOW){
        Serial.println("ajuste");
        return DERECHA1;
    }*/
    if (izquierda == LOW && centro == LOW && derecha == HIGH){
        Serial.println("ajuste");
        return DERECHA1;
    }
    if (izquierda == HIGH && centro == LOW && derecha == LOW) {
        Serial.println("Izquierda1");
        return DERECHA;  // Mover hacia adelante
    } 

    return STOP; // Por defecto, si no se cumplen las condiciones
}

// Función para ajustar la velocidad de los motores según la dirección detectada
void ajustarVelocidadMotores(Seguidor direccion) {
    int velocidadBase = 4095;  // Velocidad base para ambos motores (12 bits)
    int velocidadIzquierda;
    int velocidadDerecha;
    bool Derecha = false;

    // Asignación de dirección y velocidad de motores
    switch (direccion) {
        case DERECHA:
            velocidadIzquierda = velocidadBase ;  // Aumenta la velocidad del motor izquierdo
            velocidadDerecha = 995;  // Detiene el motor derecho
            break;
        case IZQUIERDA:
            velocidadIzquierda = 995;  // Detiene el motor izquierdo
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
            velocidadIzquierda = 4095;
            velocidadDerecha = 4095;
            Derecha = true;
            break;    
    }

    if (Derecha) 
    {
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, HIGH);
      digitalWrite(MOTOR_DER1, LOW);
      digitalWrite(MOTOR_DER2, HIGH);
    }
    else {// Establecer dirección y velocidad de los motores
      digitalWrite(MOTOR_IZQ1, LOW);
      digitalWrite(MOTOR_IZQ2, HIGH);
      digitalWrite(MOTOR_DER1, HIGH);
      digitalWrite(MOTOR_DER2, LOW);
    }

    ledcWrite(ENA, velocidadIzquierda);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
    ledcWrite(ENB, velocidadDerecha);    // Estable
}


void prenderBomba()
{
  digitalWrite(MOTOR_IZQ1, LOW);
  digitalWrite(MOTOR_IZQ2, LOW);
  digitalWrite(MOTOR_DER1, LOW);
  digitalWrite(MOTOR_DER2, LOW);
  ledcWrite(ENA, 0);  // Establecer el ciclo de trabajo PWM para el motor izquierdo
  ledcWrite(ENB, 0);   
  ajustarVelocidadMotores(STOP);
  digitalWrite(BOMBA_N, HIGH);
  digitalWrite(BOMBA_P, LOW);
  ajustarVelocidadMotores(STOP);
  Serial.println("hola bomba");
  ledcWrite(ENA1, 4095);  
  delay(8000);
  digitalWrite(BOMBA_N, LOW);
  digitalWrite(BOMBA_P, LOW);
}