#include <Thread.h>
#include <ThreadController.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include "ListLib.h"

#define SS_PIN 10
#define RST_PIN 9
#define LED_ROJO 5
#define LED_VERDE 6
#define LED_AZUL 7
#define BUZZER 3
#define DOUT A1
#define CLK A0

// Threads.
ThreadController controlador = ThreadController();
Thread RGB_thread = Thread();
Thread LCD_thread = Thread();
Thread HX711_thread = Thread();
Thread Alarma_thread = Thread();

// Lector RFID RC522.
MFRC522 mfrc522(SS_PIN, RST_PIN);
// LCD.
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7);
// Galga extensiometrica modulo HX711.
HX711 sensor_peso;

// VARIABLES.
String uid; // Uid leido.
String aux_uid;
List<String> uids_leidos(8); // Lista de uids leidos.
float peso_actual = 0.0;
float peso_anterior = 0.0;
float incremento = 2.0;
// Booleanos y semáforos.
bool activacion = false;
bool pagado = false;
bool add_articulo = false;
bool remove_articulo = false;
bool add_list = false;
bool escaneo_rfid = false;
bool peso_ok = false;
bool alarma = false;
bool reintento = false;
// Uids rfid.
const String MASTER_KEY = "2C1DDB6E";
const String USER_KEY = "1A624186";
const String BOTELLA = "5356B63E1FFC0";
const String PIZZA = "5317D61E1E8C0";
const String POLLO = "53579316197C0";
const String ZAPATO = "5397F71610C0";
const String HAMBURGUESA = "5316F71E18FC0";
const String PAN = "5357B71618FC0";
// Importes productos.
float importe_total = 0;
float importe_botella = 0.90;
float importe_pizza = 2.65;
float importe_zapato = 44.99;
float importe_pollo = 5.37;
float importe_hamburguesa = 3.25;
float importe_pan = 0.55;


// FUNCIONES.
// Seleccionar color led RGB.
void setColor(bool rojo, bool verde, bool azul) {
   digitalWrite(LED_ROJO,rojo);
   digitalWrite(LED_VERDE,verde);
   digitalWrite(LED_AZUL,azul);
}

// Comprobar si el uid leido se encuentra en la lista de uids.
int checkUidList() {
  for(int i=0; i<uids_leidos.Count();i++) { // Recorremos la lista.
    if(uids_leidos[i] == uid) { 
      add_list = false; // Si lo encontramos no se añade y se devuelve la posición.
      return i;
    }
  }
  add_list = true; // Si no se encuentra se añade a la lista.
  return 0;
}

// Espera con millis
void wait(int periodo) {
  unsigned long TiempoAhora = millis();
  while(millis() < TiempoAhora+periodo){
  }
}

// Si hay tag lee su uid con el modulo rc522.
void readUID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      uid.toUpperCase(); // Pasamos a mayúsculas.
    }
  }
  mfrc522.PICC_HaltA(); // Dejamos de leer.
}

// Muestra por el lcd el articulo añadido, su importe y el total.
void addArticulo(String item) {
  lcd.clear();
  lcd.setCursor(0,0);
  if(item == BOTELLA) {
    lcd.print("Botella de agua");
    lcd.setCursor(0,1);
    lcd.print(importe_botella);
    lcd.print(" euros");
    importe_total += importe_botella; // Cálculo suma importe total.
    wait(3000);
    getImporteTotal(); // Muestra el importe total.
  }
  else if(item == PIZZA) {
    lcd.print("Pizza jamon");
    lcd.setCursor(0,1);
    lcd.print(importe_pizza);
    lcd.print(" euros");
    importe_total += importe_pizza;
    wait(3000);
    getImporteTotal();
  }
  else if(item == POLLO) {
    lcd.print("Pollo entero");
    lcd.setCursor(0,1);
    lcd.print(importe_pollo);
    lcd.print(" euros");
    importe_total += importe_pollo;
    wait(3000);
    getImporteTotal();
  }
  else if(item == ZAPATO) {
    lcd.print("Zapatilla correr");
    lcd.setCursor(0,1);
    lcd.print(importe_zapato);
    lcd.print(" euros");
    importe_total += importe_zapato;
    wait(3000);
    getImporteTotal();
  }
  else if(item == HAMBURGUESA) {
    lcd.print("Burger vegana");
    lcd.setCursor(0,1);
    lcd.print(importe_hamburguesa);
    lcd.print(" euros");
    importe_total += importe_hamburguesa;
    wait(3000);
    getImporteTotal();
  }
  else if(item == PAN) {
    lcd.print("Barra de pan");
    lcd.setCursor(0,1);
    lcd.print(importe_pan);
    lcd.print(" euros");
    importe_total += importe_pan;
    wait(3000);
    getImporteTotal();
  }
}

// Muestra por el lcd el articulo retirado y el importe total.
void removeArticulo(String item) {
  lcd.clear();
  lcd.setCursor(0,0);
  if(item == BOTELLA) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Botella de agua");
    importe_total -= importe_botella; // Cálculo resta importe total.
    wait(3000);
    getImporteTotal(); // Muestra el importe total.
  }
  else if(item == PIZZA) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Pizza jamon");
    importe_total -= importe_pizza;
    wait(3000);
    getImporteTotal();
  }
  else if(item == POLLO) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Pollo entero");
    importe_total -= importe_pollo;
    wait(3000);
    getImporteTotal();
  }
  else if(item == ZAPATO) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Zapatilla correr");
    importe_total -= importe_zapato;
    wait(3000);
    getImporteTotal();
  }
  else if(item == HAMBURGUESA) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Burger vegana");
    importe_total -= importe_hamburguesa;
    wait(3000);
    getImporteTotal();
  }
  else if(item == PAN) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Barra de pan");
    importe_total -= importe_pan;
    wait(3000);
    getImporteTotal();
  }
}

// Muestra por el lcd el importe total.
void getImporteTotal() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Total:");
  lcd.setCursor(0,1);
  lcd.print(importe_total);
  lcd.print(" euros");
  // Actualiza los semáforos una vez completado el escáner del producto.
  add_articulo = false;
  remove_articulo = false;
  peso_ok = false;
}

// Función que actualiza el led cada ejecución del thread.
void callback_rgb() {
  if(activacion && !pagado){ // Si estamos en el proceso de escaneo (carro activado y no hemos pasado por caja).
    if(uid == BOTELLA) {
      int element = checkUidList(); // Comprobamos si el producto se encuentra en la lista.
      if(add_list) { // Si no se encuentra lo añadimos.
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW); // Encendemos el led de color verde.
        add_articulo = true; // Confirmamos que se ha añadido un producto.
        tone(BUZZER,2000); // Pitido de confirmación.
        wait(150);
        noTone(BUZZER);
      }
      else { // Si se encuentra en la lista lo retiramos.
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW); // Encendemos el led de color rojo.
        remove_articulo = true; // Confirmamos la eliminación de un producto.
        tone(BUZZER,350); // Pitido de eliminación.
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid; // Guardamos el escaneo anterior en una variable auxiliar.
      uid = ""; // Reseteamos el uid.
      escaneo_rfid = true; // Confirmamos que se ha escaneado un producto.
    }
    else if(uid == PIZZA) {
      int element = checkUidList();
      if(add_list) {
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW);
        add_articulo = true;
        tone(BUZZER,2000);
        wait(150);
        noTone(BUZZER);
      }
      else {
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW);
        remove_articulo = true;
        tone(BUZZER,350);
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid;
      uid = "";
      escaneo_rfid = true;
    }
    else if(uid == POLLO) {
      int element = checkUidList();
      if(add_list) {
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW);
        add_articulo = true;
        tone(BUZZER,2000);
        wait(150);
        noTone(BUZZER);
      }
      else {
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW);
        remove_articulo = true;
        tone(BUZZER,350);
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid;
      uid = "";
      escaneo_rfid = true;
    }
    else if(uid == ZAPATO) {
      int element = checkUidList();
      if(add_list) {
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW);
        add_articulo = true;
        tone(BUZZER,2000);
        wait(150);
        noTone(BUZZER);
      }
      else {
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW);
        remove_articulo = true;
        tone(BUZZER,350);
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid;
      uid = "";
      escaneo_rfid = true;
    }
    else if(uid == HAMBURGUESA) {
      int element = checkUidList();
      if(add_list) {
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW);
        add_articulo = true;
        tone(BUZZER,2000);
        wait(150);
        noTone(BUZZER);
      }
      else {
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW);
        remove_articulo = true;
        tone(BUZZER,350);
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid;
      uid = "";
      escaneo_rfid = true;
    }
    else if(uid == PAN) {
      int element = checkUidList();
      if(add_list) {
        uids_leidos.Add(uid);
        setColor(LOW,HIGH,LOW);
        add_articulo = true;
        tone(BUZZER,2000);
        wait(150);
        noTone(BUZZER);
      }
      else {
        uids_leidos.Remove(element);
        setColor(HIGH,LOW,LOW);
        remove_articulo = true;
        tone(BUZZER,350);
        wait(150);
        noTone(BUZZER);
      }
      aux_uid = uid;
      uid = "";
      escaneo_rfid = true;
    }
    else { // Si no leemos ningún producto contemplado el led se mantiene apagado.
      setColor(LOW,LOW,LOW);
    }
  }
}

// Función que actualiza el lcd cada ejecución del thread.
void callback_lcd() {
  if(add_articulo && peso_ok) { // Si hemos añadido un artículo y el peso lo ha detectado lo mostramos por pantalla.
    addArticulo(aux_uid);   
  }
  else if(remove_articulo) { // Si hemos retirado un artículo lo mostramos por pantalla.
    removeArticulo(aux_uid);
  }
}

// Función que mide el peso cada ejecución del thread.
void callback_hx711() {
  peso_actual = -1*sensor_peso.get_units(20); // Calculamos el peso actual.
  if(peso_actual-peso_anterior > incremento && abs(peso_actual-peso_anterior)<20) {
    // Si la diferencia entre el peso actual y el anterior es mayor que lo establecido y menor de 20.
    if(escaneo_rfid) { // Si ha habido un escaneo anteriormente se valida el pesaje y actualizan semáforos.
      peso_ok = true;
      escaneo_rfid = false;
      reintento = false;
    }
    else { // Si no se ha escaneado nada.
      peso_ok = false;
      reintento = true; // Reintentamos el pesaje.
    }
  }
  else if(reintento and abs(peso_actual-peso_anterior) < incremento) { // Si estamos ante un reintento y la diferencia de pesos es menor que lo establecido.
    peso_ok = false;
    alarma = true; // Activamos el booleano de alarma.
    reintento = false;
  }
  else if(reintento) { // Si estamos ante un reintento.
    reintento = false;
  }
  peso_anterior = peso_actual; // Actualizamos el valor del peso anterior.
}

// Función que actualiza el estado de la alarma cada ejecución del thread.
void callback_alarma() {
  if(alarma and activacion and !pagado) { // Entraremos si se ha dado la señal de alarma y si estamos en el proceso de escaneo (carro activado y no hemos pasado por caja).
    while(uid != MASTER_KEY) { // Bucle hasta que no detectemos la llave maestra.
      setColor(HIGH,LOW,LOW); // Encendemos led en rojo.
      tone(BUZZER,800); // Pitido de alarma.
      wait(100);
      setColor(LOW,LOW,LOW);
      noTone(BUZZER);
      wait(100);
      readUID(); // Lectura rfid.  
    }
    alarma = false;
    uid = "";
  }
}


void setup() {
  Serial.begin(9600); // Configuramos el monitor serie.

  RGB_thread.enabled = true; // Activamos el thread.
  RGB_thread.setInterval(50); // Se ejecuta cada 0.05 segundos.
  RGB_thread.onRun(callback_rgb); // Llamamos a la rutina correspondiente.
  controlador.add(&RGB_thread); // Añadimos el thread al controlador.

  LCD_thread.enabled = true;
  LCD_thread.setInterval(100); // Se ejecuta cada 0.1 segundos.
  LCD_thread.onRun(callback_lcd);
  controlador.add(&LCD_thread);

  HX711_thread.enabled = true;
  HX711_thread.setInterval(2000); // Se ejecuta cada 2 segundos.
  HX711_thread.onRun(callback_hx711);
  controlador.add(&HX711_thread);

  Alarma_thread.enabled = true;
  Alarma_thread.setInterval(2000); // Se ejecuta cada 2 segundos.
  Alarma_thread.onRun(callback_alarma);
  controlador.add(&Alarma_thread);
  
  SPI.begin(); // Configuramos la comunicación SPI.
  mfrc522.PCD_Init(); // Inicializamos el lector rfid.

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  lcd.begin(16,2); // Inicializamos lcd, 16 columnas por 2 lineas.
  lcd.clear(); // Limpia pantalla.
  lcd.setBacklightPin(3,POSITIVE); // Configuramos la luz de la pantalla.
  lcd.setBacklight(LOW); // Establecemos la luz apagada.

  sensor_peso.begin(DOUT, CLK); // Inicializamos el sensor de peso.
  sensor_peso.set_scale(239.29054); // Colocamos la escala para el cálculo del peso.
}

void loop() {
  controlador.run(); // Lanzamos el controlador de threads.
  readUID(); // Lectura rfid.
  
  if(uid == USER_KEY && !activacion) { // Si el carro está en stand by y detecta la llave de usuario.
    lcd.setBacklight(HIGH); // Se enciende el lcd.
    lcd.print("Iniciando");
    lcd.setCursor(0,1);
    lcd.print("carrito...");
    sensor_peso.tare(20);  // Iniciamos y taramos el carro.
    wait(200);
    for(int i=0; i<5; i++) { // Parpadeamos el led en color verde a modo de confirmación.
      setColor(LOW,HIGH,LOW);
      wait(100);
      setColor(LOW,LOW,LOW);
      wait(100);
    }
    // Carro activado y listo para escanear artículos.
    activacion = true;
    lcd.clear();
    lcd.print("Bienvenido!");
    lcd.setCursor(0,1);
    lcd.print("Escanee articulo");
    uid = "";
  }
  else if(uid == USER_KEY && pagado) { // Si el carro ya ha sido cobrado y detecta la llave de usuario.
    setColor(LOW,LOW,LOW); // Apagamos el led.
    activacion = false;
    pagado = false;
    lcd.clear();
    lcd.print("Hasta pronto...");
    lcd.setCursor(0,1);
    lcd.print("Apagando carrito");
    delay(10000);
    lcd.clear();
    lcd.setBacklight(LOW); // Apagamos el lcd.
    uid = "";
  }
  else if(uid == MASTER_KEY && activacion) { // Si el carro está en escaneo y detecta la llave maestra.
    setColor(LOW,LOW,HIGH); // Encendemos el led en azul.
    pagado = true;
    lcd.clear();
    if(importe_total > 0.0) { // Si el importe total es mayor que 0.
      lcd.print("Pago confirmado");
      lcd.setCursor(0,1);
      lcd.print("Puede marcharse");
      importe_total = 0.0; // Reseteamos importe total.
      uids_leidos.Clear();
    }
    else { // Si no lleva nada de compra.
      lcd.print("Salida");
      lcd.setCursor(0,1);
      lcd.print("sin compra");
    }
    uid = "";
  }
}
