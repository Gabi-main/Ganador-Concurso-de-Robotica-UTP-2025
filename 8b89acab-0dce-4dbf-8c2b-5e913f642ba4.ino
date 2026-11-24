/*
 * Robot Seguidor de Líneas con Control IR - VERSIÓN SIMPLIFICADA
 * Arduino Uno
 * 
 * IMPORTANTE: Los sensores IR dan LOW cuando ven BLANCO, HIGH cuando ven NEGRO
 */

#include <IRremote.h> 
//recuerda descargar el driver necesario wink wink

// ===== PINES DE MOTORES =====
#define DIR_A 2    // Motor Izquierdo - Dirección
#define PWM_A 5    // Motor Izquierdo - Velocidad
#define DIR_B 4    // Motor Derecho - Dirección
#define PWM_B 6    // Motor Derecho - Velocidad

// ===== PINES DE SENSORES =====
#define SENSOR_IZQ 7     // Sensor Izquierdo
#define SENSOR_CEN 12    // Sensor Central
#define SENSOR_DER 9     // Sensor Derecho

// ===== PIN IR =====
#define RECV_PIN 3

// ===== CÓDIGOS IR =====
#define CODE_FORWARD     0xFF629D
#define CODE_BACKWARD    0xFFA857
#define CODE_LEFT        0xFF22DD
#define CODE_RIGHT       0xFFC23D
#define CODE_STOP        0xFF02FD
#define CODE_LINE_FOLLOW 0xFF52AD

// ===== VELOCIDADES =====
#define VEL_NORMAL 180
#define VEL_GIRO   140

// ===== VARIABLES GLOBALES =====
IRrecv irrecv(RECV_PIN);
decode_results results;
bool modoSeguidor = false;
unsigned long ultimoReporte = 0;

void setup() {
  Serial.begin(9600);
  
  // Configurar motores
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  
  // Configurar sensores
  pinMode(SENSOR_IZQ, INPUT);
  pinMode(SENSOR_CEN, INPUT);
  pinMode(SENSOR_DER, INPUT);
  
  // Iniciar IR
  irrecv.enableIRIn();
  
  detener();
  
  Serial.println(F("========================================"));
  Serial.println(F("  ROBOT SEGUIDOR DE LINEAS v2.0"));
  Serial.println(F("========================================"));
  Serial.println(F("Sistema listo"));
  Serial.println(F("Esperando comandos..."));
  Serial.println(F("----------------------------------------"));
}

void loop() {
  // Leer comando IR
  if (irrecv.decode(&results)) {
    procesarIR(results.value);
    irrecv.resume();
  }
  
  // Si está en modo seguidor, ejecutar
  if (modoSeguidor) {
    seguirLinea();
  }
  
  delay(10);
}

void procesarIR(unsigned long codigo) {
  Serial.print(F("[IR] Código: 0x"));
  Serial.print(codigo, HEX);
  Serial.print(F(" → "));
  
  switch (codigo) {
    case CODE_FORWARD:
      Serial.println(F("ADELANTE"));
      modoSeguidor = false;
      adelante(VEL_NORMAL);
      break;
      
    case CODE_BACKWARD:
      Serial.println(F("ATRÁS"));
      modoSeguidor = false;
      atras(VEL_NORMAL);
      break;
      
    case CODE_LEFT:
      Serial.println(F("IZQUIERDA"));
      modoSeguidor = false;
      izquierda(VEL_GIRO);
      break;
      
    case CODE_RIGHT:
      Serial.println(F("DERECHA"));
      modoSeguidor = false;
      derecha(VEL_GIRO);
      break;
      
    case CODE_STOP:
      Serial.println(F("DETENER"));
      modoSeguidor = false;
      detener();
      break;
      
    case CODE_LINE_FOLLOW:
      modoSeguidor = !modoSeguidor;
      if (modoSeguidor) {
        Serial.println(F("✓ MODO SEGUIDOR ACTIVADO"));
      } else {
        Serial.println(F("✗ MODO SEGUIDOR DESACTIVADO"));
        detener();
      }
      break;
      
    case 0xFFFFFFFF:
      // Señal repetida, ignorar
      break;
      
    default:
      Serial.println(F("Desconocido"));
      break;
  }
}

void seguirLinea() {
  // Leer sensores (recordar: LOW = blanco, HIGH = negro)
  int izq = digitalRead(SENSOR_IZQ);
  int cen = digitalRead(SENSOR_CEN);
  int der = digitalRead(SENSOR_DER);
  
  // Mostrar estado cada 300ms
  if (millis() - ultimoReporte > 300) {
    Serial.println(F(""));
    Serial.print(F("[SENSORES] "));
    Serial.print(izq == HIGH ? "⚫" : "⚪");
    Serial.print(F(" "));
    Serial.print(cen == HIGH ? "⚫" : "⚪");
    Serial.print(F(" "));
    Serial.println(der == HIGH ? "⚫" : "⚪");
    ultimoReporte = millis();
  }
  
  // LÓGICA SIMPLE: Seguir la línea NEGRA (HIGH)
  
  // Solo el centro ve negro → ADELANTE
  if (izq == LOW && cen == HIGH && der == LOW) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ⬆️ ADELANTE"));
    }
    adelante(VEL_NORMAL);
  }
  
  // Centro e izquierda ven negro → GIRAR IZQUIERDA
  else if (izq == HIGH && cen == HIGH && der == LOW) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ⬅️ IZQUIERDA"));
    }
    izquierda(VEL_GIRO);
  }
  
  // Solo izquierda ve negro → GIRAR IZQUIERDA FUERTE
  else if (izq == HIGH && cen == LOW && der == LOW) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ⬅️⬅️ IZQUIERDA FUERTE"));
    }
    izquierda(VEL_GIRO);
  }
  
  // Centro y derecha ven negro → GIRAR DERECHA
  else if (izq == LOW && cen == HIGH && der == HIGH) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ➡️ DERECHA"));
    }
    derecha(VEL_GIRO);
  }
  
  // Solo derecha ve negro → GIRAR DERECHA FUERTE
  else if (izq == LOW && cen == LOW && der == HIGH) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ➡️➡️ DERECHA FUERTE"));
    }
    derecha(VEL_GIRO);
  }
  
  // Todos ven negro → CRUCE, seguir adelante
  else if (izq == HIGH && cen == HIGH && der == HIGH) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ⬆️ CRUCE - Adelante"));
    }
    adelante(VEL_NORMAL);
  }
  
  // Todos ven blanco → LÍNEA PERDIDA, detener
  else if (izq == LOW && cen == LOW && der == LOW) {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ❌ LÍNEA PERDIDA"));
    }
    detener();
  }
  
  // Cualquier otra combinación → continuar último movimiento
  else {
    if (millis() - ultimoReporte > 300) {
      Serial.println(F("[ACCIÓN] ? Patrón extraño"));
    }
  }
}

// ===== FUNCIONES DE MOVIMIENTO =====

void adelante(int vel) {
  digitalWrite(DIR_A, HIGH);
  analogWrite(PWM_A, vel);
  digitalWrite(DIR_B, LOW);   // Invertido para motor derecho
  analogWrite(PWM_B, vel);
}

void atras(int vel) {
  digitalWrite(DIR_A, LOW);
  analogWrite(PWM_A, vel);
  digitalWrite(DIR_B, HIGH);  // Invertido para motor derecho
  analogWrite(PWM_B, vel);
}

void izquierda(int vel) {
  // Motor izquierdo atrás, motor derecho adelante
  digitalWrite(DIR_A, LOW);
  analogWrite(PWM_A, vel);
  digitalWrite(DIR_B, LOW);   // Invertido para motor derecho
  analogWrite(PWM_B, vel);
}

void derecha(int vel) {
  // Motor izquierdo adelante, motor derecho atrás
  digitalWrite(DIR_A, HIGH);
  analogWrite(PWM_A, vel);
  digitalWrite(DIR_B, HIGH);  // Invertido para motor derecho
  analogWrite(PWM_B, vel);
}

void detener() {
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);
}