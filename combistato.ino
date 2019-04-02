// COMBISTATO PROGRAMABLE, FRIO CALOR O DOBLE TERMOSTATO
// por Facundo Suarez
// 2019

#include <EEPROM.h>
#include <TimeLib.h>
#include <OneWire.h> //temp
#include <DallasTemperature.h> //temp
#include <LiquidCrystal.h> //lcd

#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress termo1 = {0x28, 0xFF, 0xA0, 0xE1, 0xC1, 0x17, 0x05, 0xF8};
DeviceAddress termo2 = {0x28, 0xFF, 0x4D, 0xC3, 0x82, 0x17, 0x04, 0x32};

// definiendo variables de almacenamiento 
float datos[3][5]; // para amacenar la lectura de cada sensor, su maxima y su minima
float trabajo[5]; // para almacenar ajustes de trabajo
float alarmas[5]; // almacenar alarmas.
byte CIERRE=1;

// variables de registro en serial y lcd
// Los siguientes valores seran configurables en nuevas versiones
int serialLog=5;   // sengundos de intervalo de registro de consola
long serialSegPre;  // contador tiempo previo registro en consola
int lcdLog=1;   // sengundos de intervalo de registro en display
long lcdSegPre;  // contador tiempo previo registro en display
byte PS = 0; // control de rutina de regitro en consola serie

//control de retardo de accion de combistato
long ctlRetF;
long ctlRetC;

// variables de control de chiller
int preFrio=15;    //segundos de marcha de ventilador antes que el compresor
int postFrio=200;  // sengundos de ventilador del condensador despues de parar el compresor
int demanda=300;  // sengundos de marcha por demanda antes de parar por temperatura
long preVent;       // para almacenar el tiempo de control de ventilacion pre frio
long postVent;       // para almacenar el tiempo de control de ventilacion post frio
long actDem;      // para almacenar el tiempo de inicio de la marcha por demanda

// para el lector de teclado..
int niveles[16]={12,88,155,218,254,299,340,380,402,432,460,487,502,524,542,563};
int teclas[16]={1,2,3,11,4,5,6,12,7,8,9,13,15,0,16,14};
int codigo=0;

// textos de los menus..
String CFGs[5]={"CONFIGURAR","1.Guardar Vals.","2.Guardar Alarmas","3.Cambiar sensor","4 -> ..."};
String ALs[5]={"ALARMAS","1.Baja: ","2.Baja ?","3.Alta:","4.Alta ?"};
String TRBJ[5]={"TRABAJO","1.Ajuste:","2.Ventan:","3.Demora:","4.Error :"};

/*
para control de pantallas
0 -> pantalla normal
1 -> pantalla de ajustes
2 -> pantalla de alarmas
3 -> pantalla de configurar
4 -> pantalla de Acerca de ..
*/
byte pantalla = 0;

int lcdClear = 120;
long actLCD;

int estado = 0;
String msj="";

int DisInd = 0;
int controlLed = 1;
int cantSens = 2;

int CLAVE;

/*
 The circuit:
 * LCD RS pin to digital pin 2
 * LCD En pin to digital pin 3
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
*/
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

byte sagitario[8] = { 0b00000000,0b00000111,0b00000011,0b00010101,0b00001000,0b00010100,0b00000000,0b00000000 };
byte grado[8] = { 0b00001100,0b00010010,0b00010010,0b00001100,0b00000000,0b00000000,0b00000000, 0b00000000 };
void setup(){ 

  /*  leer datos de la eeprom
    Las direcciones de la eeprom seran:
    AJUSTES:
    -  (1) y (2)  ->  punto de ajuste   [entero x10]
    -        (3)  ->  ancho de la ventana de histeresis [byte x10]
    -        (4)  ->  retardo entre acciones, en minutos [byte x1]
    -  (5) y (6)  ->  desvio o error de sensor [x10, entero]
    
    ALARMAS:
    -  (10)       ->  Alarma de Baja temperatura [x10, entero]
    -  (11)       ->  Alarma de baja activa o no [1 o 0]
    -  (12)       ->  Alarma de Alta temperatura [x10, entero]
    -  (13)       ->  Alarma de Alta activa o no [1 o 0]

    OTROS
    -  (21) y (22)  -> clave para cambiar ajustes

  */
  byte EEdir;
  int EEvalor;

  trabajo[1] = (float)(( EEPROM.read(1) << 8 ) + EEPROM.read(2))/(float)10; //Ajuste
  trabajo[2] = (float) EEPROM.read(3)/(float) 10; //histeresis
  trabajo[3] = (float) EEPROM.read(4); // retardo
  trabajo[4] = (float)((EEPROM.read(5) << 8)  + EEPROM.read(6))/(float)10; //error
  
  alarmas[1] = (float) EEPROM.read(10)/(float) 10; //ajuste
  alarmas[2] = (float) EEPROM.read(11); // activa
  alarmas[3] = (float) EEPROM.read(12)/(float) 10; //ajuste
  alarmas[4] = (float) EEPROM.read(13); // activa
  CLAVE = (int)(( EEPROM.read(21) << 8 ) + EEPROM.read(22));
  //CLAVE=3312;
  // fin de lectura de eeprom

  trabajo[0]=-237; //evitar impresion en lcd
  alarmas[0]=-237; //evitar impresion en lcd

  pinMode(A0,INPUT); // para leer el teclado
  pinMode(8,OUTPUT); // control de ventilador de condensador
  pinMode(9,OUTPUT); // control de motocompresor
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(10,INPUT_PULLUP); // ingreso de evento desde el control de fermentadores

  lcd.begin(16, 2);
  lcd.createChar(2, sagitario);
  impLinea(0,0,"  Sagitario",-237,0);
  impLinea(1,0,"Combistato plus",-237,0);
  
  delay(3000);

  sensors.begin();
  sensors.setResolution (termo1, 12);
  sensors.setResolution (termo2, 12);

} 

void loop() {

  long segAct = now();
  char c = '.' ;
  String comando ="" ;
  String numero ="" ;  

  int tecla = leerTecla();

  if (codigo == 980) buscaSensor();
  if (codigo == 157) {
    Serial.begin(19200);
    PS=2;
    impLinea(1,0,"Serial SI",PS,11);
    codigo=0;
  }
  if (codigo == 156) {
    Serial.begin(19200);
    PS=1;
    impLinea(1,0,"Serial SI",PS,11);
    codigo=0;
  }
  if (codigo == 155) {
    impSerie("Registro en consola desactivado...\n",-237,1);    
    Serial.end();
    PS=0;
    impLinea(1,0,"Serial NO",-237,0);
    codigo=0;
  }
  if (codigo == 914) {
    impLinea(1,0,"",CLAVE,0);
    Serial.println(CLAVE);
    codigo=0;
  }
  if (codigo==915 && CIERRE==0) {
    impLinea(0,0,"nueva:",-237,0);
    CLAVE=ingrVal("clave:");
  }    
  if (codigo == 916 && CIERRE==0) {
    intAeeprom(21,CLAVE);
    impLinea(1,0,"Perfectirijillo",-237,0);
    codigo=0;
  }
  
  if (pantalla==1 && tecla>0 && tecla<5) trabajo[tecla] = ingrVal(TRBJ[tecla]);
  if (pantalla==2 && tecla>0 && tecla<5) alarmas[tecla] = ingrVal(ALs[tecla]);
  if (pantalla==3 && tecla==1) {
    intAeeprom(1,trabajo[1]*10);
    byteAeeprom(3,trabajo[2]*10);
    byteAeeprom(4,trabajo[3]);
    intAeeprom(5,trabajo[4]*10);
    impLinea(1,0,"Grabado a EEPROM",-237,0);
    delay(2000);
  }
  if (pantalla==3 && tecla==2) {
    byteAeeprom(10,alarmas[1]*10);
    byteAeeprom(11,alarmas[2]);
    byteAeeprom(12,alarmas[3]*10);
    byteAeeprom(13,alarmas[4]);
    impLinea(1,0,"Grabado a EEPROM",-237,0);
    delay(2000);
  }

  if (tecla==11) pantalla = 1;
  if (tecla==12) pantalla = 2;
  if (tecla==13) pantalla = 3;
  if (tecla==14) {
    lcd.clear();
    pantalla = 0;
  }

//   = = = = I N I C I O = = = = 
// bucle de impresion en lcd y control

  if(segAct - lcdSegPre >= lcdLog) { 

    sensors.requestTemperatures(); // Send the command to get temperatures
    datos[1][0] = sensors.getTempC(termo1);
    datos[2][0] = sensors.getTempC(termo2);
    byte ind;
    lcdSegPre = segAct;

// rutina para identificar y registrar maximo y minimo
    for (int i=1; i <= cantSens; i++){
      if (datos[i][1] < datos[i][0]){ // T max
        datos[i][1]=datos[i][0];
        datos[i][2]=now();
        impLinea(1,0,"Max:",datos[i][1],5);
      }

      if (datos[i][0] != -127) {
        if ((datos[i][3] > datos[i][0]) || (datos[i][3] == 0)){ // T min
          datos[i][3]=datos[i][0];
          datos[i][4]=now();
          impLinea(1,0,"Min:",datos[i][3],5);
        }
      }
    }

// Pantallas
    if (pantalla==1){
      if (DisInd == 5) DisInd = 0;
      impLinea(0,0,TRBJ[DisInd],trabajo[DisInd],10);
      if (DisInd == 4) ind = 0; else ind=DisInd+1;
      impLinea(1,0,TRBJ[ind],trabajo[ind],10);
      DisInd += 1;
    }

    if (pantalla==2){
      if (DisInd == 5) DisInd = 0;
      impLinea(0,0,ALs[DisInd],alarmas[DisInd],11);
      if (DisInd == 4) ind = 0; else ind=DisInd+1;
      impLinea(1,0,ALs[ind],alarmas[ind],11);
      DisInd += 1;
    }

    if (pantalla==3){
      if (DisInd == 5) DisInd = 0;
      impLinea(0,0,CFGs[DisInd],-237,0);
      if (DisInd == 4) ind = 0; else ind=DisInd+1;
      impLinea(1,0,CFGs[ind],-237,0);
      DisInd += 1;
    }

    if (pantalla==0){
      impLinea(0,0,"T:",datos[1][0],2);
    }

    //  = = = C O M B I S T A T O = = = 
    // control de frio - enfriar
    long TPO=trabajo[3]*60;
    if(datos[1][0]!=-127){
      if (!digitalRead(8)) {
        if (datos[1][0] >= (trabajo[1]+trabajo[2])) {
          if(segAct-ctlRetF < TPO) {
            msj="frio";
            impSerie("Frio demorado...\n",237,2);
             if(controlLed<0) msj="-"+String(TPO+ctlRetF-segAct)+"-";
          }
          if(segAct-ctlRetF >= TPO) {
            msj="FRIO";
            impSerie("Frio iniciado...\n",-237,1);
            digitalWrite(8, HIGH);
          }
        }
      }
      // control de frio - dejar de enfriar
      if ((datos[1][0] <= trabajo[1]) && digitalRead(8)) {
        digitalWrite(8, LOW);
        impSerie("Frio detenido...\n",-237,1);
        ctlRetF = segAct;
      }
      // control de calor - dejar de calentar
      if ((datos[1][0] >= trabajo[1]) && digitalRead(9)) {
        digitalWrite(9, LOW);
        impSerie("Calor detenido...\n",-237,1);
        ctlRetC = segAct;
      }
      // control de calor - calentar
      if(!digitalRead(9)) {    
        if (datos[1][0] <= (trabajo[1]-trabajo[2])) {
          if(segAct-ctlRetC < TPO) {
           msj="calor";
           impSerie("Calor demorado...\n",-237,2);
           if(controlLed<0) msj="-"+String(TPO+ctlRetC-segAct)+"-";
          }
          if(segAct-ctlRetC >= TPO) {
            msj="CALOR";
            impSerie("Calor iniciado...\n",-237,1);
            digitalWrite(9, HIGH);
          }
        }
      }
      if (!digitalRead(8) && !digitalRead(9) &&
         msj!="calor" && msj!="frio" &&
         !msj.startsWith("-")) {
           msj="";
         }
      if(pantalla==0) impLinea(0,11,msj,-237,10);
    }
    else {
      impLinea(0,0,"   ERROR",-237,10);
      impLinea(1,0," en sensor",-237,10);
    }

//    Serial.println(msj);
        //  = = F I N    C O M B I S T A T O = = 

    controlLed = -controlLed;
    lcd.setCursor(9, 0);
    if (controlLed > 0) {
      if(pantalla==0) lcd.print(" ");
      digitalWrite(LED_BUILTIN, LOW);
    }
    else {
      if(pantalla==0) lcd.write(2);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

//   = = = = F I N = = = = 
// de bucle de impresion en lcd y control


 //   = = = = I N I C I O = = = = 
// rutina de registro en consola Serial
   if(segAct - serialSegPre >= serialLog) {

     if(pantalla==0) {
       impLinea(1,0,"M:",datos[1][1],2);
       impLinea(1,8,"m:",datos[1][3],10);
     }

  if(PS>=1) {
    Serial.print(now());
    for (int i=1; i <= cantSens; i++){
      Serial.print(" - Temp");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(datos[i][0]);
      Serial.print("C");
      }
    }
    Serial.println();  
  serialSegPre = segAct;
  }

//   = = = = F I N = = = = 
// fin de rutina registro en consola

}

void impLinea (int lin, int col, String msj, float dato, int col2) {
  lcd.setCursor(col, lin);
  lcd.print("                ");
  lcd.setCursor(col, lin);
  lcd.print(msj);
  if (dato!=-237) {
    lcd.setCursor(col2, lin);
    lcd.print(dato);
  }
}  
// funcion para grabar en eeprom un entero (2 bytes)
void intAeeprom(byte EEdir, int EEvalor) {
  EEPROM.write(EEdir, highByte(EEvalor));
  EEPROM.write(EEdir + 1, lowByte(EEvalor));
}
// funcion para grabar en eeprom un byte
void byteAeeprom(byte EEdir, byte EEvalor) {
  EEPROM.write(EEdir, EEvalor);
}

int leerTecla() {
  int teclado = analogRead(A0); // -- inicio comandos por teclado matricial 4x4 --
  if (teclado < 1000) {
    for(int t=0; t<16; t++) {
      if(abs(teclado - niveles[t]) < 5) {
        Serial.print (teclado);
        Serial.print (" -> ");
        Serial.println (teclas[t]);
        lcd.setCursor(14, 0);
        lcd.print(teclas[t]);
        delay(200);
        
        if (pantalla==0) {
          codigo=codigo*10+teclas[t];
          if (codigo > 999) {
            if(codigo==CLAVE) {
              CIERRE=0;
              impLinea(1,0,"Desbloqueado",-237,7);
            }
            else impLinea(1,0,"Codigo:",codigo,8);
            codigo=0;
          }
        }

        while(analogRead(A0) < 1000) delay(100);
        return teclas[t];
      }
    }
  }// -- fin comandos por teclado matricial 4x4 --
}

float ingrVal(String var) {
  float entrada=0;
  impLinea(0,0,var,-237,0);
  impLinea(1,0,"valor:",entrada,7);
  long SALIR=now();
  while(SALIR>0){
    int tecla=leerTecla();
    if(tecla<10) {
      entrada = entrada*10+tecla/(float)10;
      impLinea(1,0,"valor:",entrada,7);
    }
    if(tecla==15) {
      entrada=-entrada;
      impLinea(1,0,"valor:",entrada,7);
    }
    if(tecla==16) {
      entrada=0;
      impLinea(1,0,"valor:",entrada,7);
    }
    if(tecla==11) impLinea(0,0,"Confirma?",-237,0);
    if(tecla==14) {
      break;
    }
    if(now()-SALIR>30) {
      entrada=-237;
      break;
    }
  }
  return entrada;
}

void guarVals(){
}
// funcion de impresion en consola, para usarlo:
// a es texto, b float y c nivel de registro
void impSerie(String a, float b, byte c){
  if(PS>=c) {
    Serial.print(a);
    if(b!=-237) {
      Serial.print(b);
      Serial.println();
    }
  }
}

void buscaSensor(void) {
  if(PS==0) Serial.begin(19200);
  impLinea(0,0,"Direccion en",-237,7);
  impLinea(1,0,"consola serie",-237,7);
  delay (2000);
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  
  Serial.print("Buscando sensores...\n\r");
  while(oneWire.search(addr)) {
    Serial.print("encontrados con direccion:\n\r");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC no valida!\n");
        return;
    }
  }
  Serial.print("\n\rEs todo.\r\n");
  oneWire.reset_search();
  if(PS==0) Serial.end();
  codigo=0;
  return;
}

