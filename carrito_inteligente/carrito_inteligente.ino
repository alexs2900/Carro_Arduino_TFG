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

//Threads
ThreadController controlador = ThreadController();
Thread RGB_thread = Thread();
Thread LCD_thread = Thread();
Thread HX711_thread = Thread();
Thread Alarma_thread = Thread();

//Lector RFID RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
//LCD
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7);
//Galga extensiometrica modulo HX711
HX711 sensor_peso;

//VARIABLES
String uid; //Uid leido
String aux_uid;
List<String> uids_leidos(8); //Lista de uids leidos
float peso_actual = 0.0;
float peso_anterior = 0.0;
float incremento = 2.0;
bool activacion = false;
bool pagado = false;
bool add_articulo = false;
bool remove_articulo = false;
bool add_list = false;
bool escaneo_rfid = false;
bool peso_ok = false;
bool alarma = false;
bool reintento = false;
//Uids rfid
const String MASTER_KEY = "2C1DDB6E";
const String USER_KEY = "1A624186";
const String BOTELLA = "5356B63E1FFC0";
const String PIZZA = "5317D61E1E8C0";
const String POLLO = "53579316197C0";
const String ZAPATO = "5397F71610C0";
const String HAMBURGUESA = "5316F71E18FC0";
const String PAN = "5357B71618FC0";
//Importes productos
float importe_total = 0;
float importe_botella = 0.90;
float importe_pizza = 2.65;
float importe_zapato = 44.99;
float importe_pollo = 5.37;
float importe_hamburguesa = 3.25;
float importe_pan = 0.55;


//FUNCIONES
//Seleccionar color led RGB
void setColor(bool rojo, bool verde, bool azul) {
   digitalWrite(LED_ROJO,rojo);
   digitalWrite(LED_VERDE,verde);
   digitalWrite(LED_AZUL,azul);
}

//Comprobar si el uid leido se encuentra en la lista de uids
int checkUidList() {
  for(int i=0; i<uids_leidos.Count();i++) {
    if(uids_leidos[i] == uid) {
      add_list = false;
      return i;
    }
  }
  add_list = true;
  return 0;
}

//Espera con millis
void wait(int periodo) {
  unsigned long TiempoAhora = millis();
  while(millis() < TiempoAhora+periodo){
  }
}

//Si hay tag lee su uid con el modulo rc522
void readUID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      uid.toUpperCase();
    }
  }
  mfrc522.PICC_HaltA();
}

//Muestra por el lcd el articulo añadido, su importe y el total
void addArticulo(String item) {
  lcd.clear();
  lcd.setCursor(0,0);
  if(item == BOTELLA) {
    lcd.print("Botella de agua");
    lcd.setCursor(0,1);
    lcd.print(importe_botella);
    lcd.print(" euros");
    importe_total += importe_botella;
    wait(3000);
    getImporteTotal();
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

//Muestra por el lcd el articulo retirado y el importe total
void removeArticulo(String item) {
  lcd.clear();
  lcd.setCursor(0,0);
  if(item == BOTELLA) {
    lcd.print("Elem retirado");
    lcd.setCursor(0,1);
    lcd.print("Botella de agua");
    importe_total -= importe_botella;
    wait(3000);
    getImporteTotal();
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

//Muestra por el lcd el importe total
void getImporteTotal() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Total:");
  lcd.setCursor(0,1);
  lcd.print(importe_total);
  lcd.print(" euros");
  add_articulo = false;
  remove_articulo = false;
  peso_ok = false;
}

void callback_rgb() {
  if(activacion && !pagado){
    if(uid == BOTELLA) {
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
    else {
      setColor(LOW,LOW,LOW);
    }
  }
}

void callback_lcd() {
  if(add_articulo && peso_ok) {
    addArticulo(aux_uid);   
  }
  else if(remove_articulo) {
    removeArticulo(aux_uid);
  }
}

void callback_hx711() {
  peso_actual = -1*sensor_peso.get_units(20);
  if(peso_actual-peso_anterior > incremento && abs(peso_actual-peso_anterior)<15) {
    if(escaneo_rfid) {
      peso_ok = true;
      escaneo_rfid = false;
      reintento = false;
    }
    else {
      peso_ok = false;
      reintento = true;
    }
  }
  else if(reintento and abs(peso_actual-peso_anterior) < incremento) {
    peso_ok = false;
    alarma = true;
    reintento = false;
  }
  else if(reintento) {
    reintento = false;
  }
  peso_anterior = peso_actual;
}

void callback_alarma() {
  if(alarma and activacion and !pagado) {
    while(uid != MASTER_KEY) {
      setColor(HIGH,LOW,LOW);
      tone(BUZZER,800);
      wait(100);
      setColor(LOW,LOW,LOW);
      noTone(BUZZER);
      wait(100);
      readUID();      
    }
    alarma = false;
    uid = "";
  }
}


void setup() {
  Serial.begin(9600);

  RGB_thread.enabled = true; //Activamos el thread
  RGB_thread.setInterval(50); //Se ejecuta cada 1 segundo
  RGB_thread.onRun(callback_rgb); //Llamamos a la rutina correspondiente
  controlador.add(&RGB_thread); //Añadimos el thread al controlador

  LCD_thread.enabled = true; //Activamos el thread
  LCD_thread.setInterval(100); //Se ejecuta cada 1 segundo
  LCD_thread.onRun(callback_lcd); //Llamamos a la rutina correspondiente
  controlador.add(&LCD_thread); //Añadimos el thread al controlador

  HX711_thread.enabled = true; //Activamos el thread
  HX711_thread.setInterval(2000); //Se ejecuta cada 1 segundo
  HX711_thread.onRun(callback_hx711); //Llamamos a la rutina correspondiente
  controlador.add(&HX711_thread); //Añadimos el thread al controlador

  Alarma_thread.enabled = true; //Activamos el thread
  Alarma_thread.setInterval(2000); //Se ejecuta cada 1 segundo
  Alarma_thread.onRun(callback_alarma); //Llamamos a la rutina correspondiente
  controlador.add(&Alarma_thread); //Añadimos el thread al controlador
  
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  lcd.begin(16,2);     // 16 columnas por 2 lineas para LCD 1602A
  lcd.clear();      // limpia pantalla
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(LOW);

  sensor_peso.begin(DOUT, CLK);
  sensor_peso.set_scale(239.29054);
}

void loop() {
  controlador.run();
  readUID();  
  
  if(uid == USER_KEY && !activacion) {
    lcd.setBacklight(HIGH);
    lcd.print("Iniciando");
    lcd.setCursor(0,1);
    lcd.print("carrito...");
    sensor_peso.tare(20);  //El peso actual es considerado Tara.
    wait(200);
    for(int i=0; i<5; i++) {
      setColor(LOW,HIGH,LOW);
      wait(100);
      setColor(LOW,LOW,LOW);
      wait(100);
    }
    activacion = true;
    lcd.clear();
    lcd.print("Bienvenido!");
    lcd.setCursor(0,1);
    lcd.print("Escanee articulo");
    uid = "";
  }
  else if(uid == USER_KEY && pagado) {
    setColor(LOW,LOW,LOW);
    activacion = false;
    pagado = false;
    lcd.clear();
    lcd.print("Hasta pronto...");
    lcd.setCursor(0,1);
    lcd.print("Apagando carrito");
    delay(10000);
    lcd.clear();
    lcd.setBacklight(LOW);
    uid = "";
  }
  else if(uid == MASTER_KEY && activacion) {
    setColor(LOW,LOW,HIGH);
    pagado = true;
    lcd.clear();
    if(importe_total > 0.0) {
      lcd.print("Pago confirmado");
      lcd.setCursor(0,1);
      lcd.print("Puede marcharse");
      importe_total = 0.0;
      uids_leidos.Clear();
    }
    else {
      lcd.print("Salida");
      lcd.setCursor(0,1);
      lcd.print("sin compra");
    }
    uid = "";
  }
}
