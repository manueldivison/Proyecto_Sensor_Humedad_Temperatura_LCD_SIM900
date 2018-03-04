#include <DHT11.h>  //Librería utilizada para Sensor DHT11.
#include <LiquidCrystal.h> //Libreria para Pantalla LCD.
#include <SoftwareSerial.h> //Libreria para Objetos SoftwareSerial.



SoftwareSerial SIM900(7, 8); //Configura puerto serial para SIM900. RX, TX.


char incoming_char = 'L';  //Variable para guardar caracter recibido por SIM900.


LiquidCrystal lcd(5, 6, 9, 10, 11, 12); //Configura pines para LCD. RS, E, D4-D7.


float temp, hum; //Variables para lecturas de temperatura y humedad.


/*Declaración de array de 8 bytes donde se 
  "traza" el caracter 'º' que se utilizará
  para representar los grados en la pantalla LCD. */
byte grado[8] = 
 {
 0b00001100,
 0b00010010,
 0b00010010,
 0b00001100,
 0b00000000,
 0b00000000,
 0b00000000,
 0b00000000
 };

 
bool EnviandoSMS = false;
bool SMSenviado = false;


int pin=2; //Variable para representar el pin 2 del Arduino.
DHT11 dht11(pin); //Configura el pin para el Sensor DHT11.

void setup()
{      
  lcd.begin(16, 2); //Configura el tipo de Matriz de 16 Columnas y 2 Filas.
  lcd.createChar(1, grado); //Crea el caracter 'º' para la pantalla.
  SIM900.begin(19200); //Configura velocidad del puerto serie para el SIM900.
  Serial.begin(19200); //Configura velocidad del puerto serie del Arduino.
  
  Serial.println("OK");
  delay (1000);
  SIM900.println("AT + CPIN = \"1234\""); //Comando AT para introducir el PIN de la tarjeta.
  delay(5000); //Tiempo para que se encuentre una Red.
  Serial.println("PIN OK");
  SIM900.print("AT+CLIP=1\r"); //Activa la identificación de llamadas.
  delay(1000);
  SIM900.print("AT+CMGF=1\r"); //Configura el modo texto para enviar o recibir mensajes.
  delay(1000);
  SIM900.print("AT+CNMI=2,2,0,0,0\r"); //Configura el módulo para mostrar SMS recibidos por comunicación serie.
  delay(1000);

  pinMode(4,OUTPUT); //Configura el pin 4 como salida (pin del LED).
}

void loop()
{  
  int err; //Variable para comprobar si hubo error en las lecturas del Sensor.
 
  if (!EnviandoSMS)
  {
     if((err = dht11.read(hum, temp)) == 0)    // Si devuelve 0 es que ha leído bien.
     {
       Serial.print("Temperatura: "); //Imprime en Monitor Serie.
       Serial.print(temp); //Imprime lectura de Temperatura en Monitor Serie.
       Serial.print(" Humedad: "); //Imprime en Monitor Serie.
       Serial.print(hum); //Imprime lectura de Humedad en Monitor Serie.
       Serial.println(); //Imprime cambio de línea en Monitor Serie.

       lcd.clear(); //Limpia LCD
       lcd.setCursor(0,0); //Coloca cursor en Columna 0, Fila 0.
       lcd.print("Temp = "); //Imprime Texto en LCD.
       lcd.print(temp); //Imprime lectura de Temperatura en LCD.
       lcd.write(1); //Imprime caracter 'º' en LCD.
       lcd.setCursor(0,1); //Coloca cursoe en Columna 0, Fila 1.
       lcd.print("Humedad= "); //Imprime Texto en LCD.
       lcd.print(hum); //Imprime lectura de Humedad en LCD.
       lcd.print(" %"); //Imprime caracter '%' en LCD.
      }                    
  }//!EnviandoSMS
      
  if (SIM900.available() > 0)
  {
    incoming_char = SIM900.read(); //Guardamos el caracter que llega desde el SIM900y
    Serial.print(incoming_char); //Mostramos el caracter en el monitor serie

  }

  /*Toma de decisiones dependiente del
    caracter recibido en un SMS por 
    el Módulo GSM/GPRS SIM900.        */
  if (incoming_char == '?')
  {
    mensaje_sms();
  }
  if (incoming_char == 'H')
  {
  digitalWrite(9,HIGH); //Enciende el LED. 
  mensaje_smsON();
  Serial.println(incoming_char);
  }
  if (incoming_char == 'L')
  {
  digitalWrite(9,LOW); //Apaga el LED. 
  mensaje_smsOFF(); 
  }

  /*Toma de decisiones dependiente de la
    lectura de humedad conseguida por el
    Sensor DHT11.
    Si la humedad es mayor a 89%, se 
    envia un mensaje con las lecturas
    obtenidas en ese instante.          */
  if(hum < 89) 
  {
    SMSenviado = false;
  }
  if (hum > 89 && !SMSenviado )     
  {
    mensaje_sms();
    SMSenviado = true;
  }
  
}

/*Función mensaje_sms. Envia un mensaje
  conteniendo los ultimos valores leídos 
  por el Sensor DHT11.                  */
void mensaje_sms() 
{
  EnviandoSMS = true;
  
  Serial.println("Enviando SMS...");
  SIM900.print("AT+CMGF=1\r"); //Comando AT para mandar un SMS
  delay(1000);
  SIM900.println("AT + CMGS = \"8294596552\""); //Numero al que vamos a enviar el mensaje. Cambiar asteriscos po nº correcto.
  delay(1000);
  SIM900.println("Informacion " );// Texto del SMS
  delay(100);
  SIM900.print("Temperatura: " );// Texto del SMS
  delay(100);
  SIM900.print(temp);// Texto del SMS
  delay(100);
   SIM900.println(" Gr. Celsius, " );// Texto del SMS
  delay(100);
  SIM900.print("Humedad: " );// Texto del SMS
  delay(100);
  SIM900.print(hum);// Texto del SMS
  delay(100);
  SIM900.println("%" );// Texto del SMS
  delay(100);
  SIM900.println((char)26);//Comando de finalizacion ^Z
  delay(100);
  SIM900.println();
  delay(5000); // Esperamos un tiempo para que envíe el SMS
  Serial.println("SMS enviado");
  incoming_char = '0';
   EnviandoSMS = false;
}


/*Función mensaje_smsOFF. Envia un mensaje
  garantizando que el LED se ha encendido.*/
void mensaje_smsON()
{
  Serial.println("Enviando SMS...");
  SIM900.print("AT+CMGF=1\r"); //Comando AT para mandar un SMS.
  delay(1000);
  SIM900.println("AT + CMGS = \"8294596552\""); //Número al que vamos a enviar el mensaje.
  delay(1000);
  SIM900.println("SISTEMA CONECTADO"); //Texto del SMS.
  delay(100);
  SIM900.println((char)26); //Comando de finalizacion ^Z.
  delay(100);
  SIM900.println();
  delay(5000); //Esperamos un tiempo para que envíe el SMS.
  Serial.println("SMS enviado");
  incoming_char = '0'; //Se asigna el valor cero.
}

/*Función mensaje_smsOFF. Envia un mensaje
  garantizando que el LED se ha apagado.*/
void mensaje_smsOFF() 
{
  Serial.println("Enviando SMS...");
  SIM900.print("AT+CMGF=1\r"); //Comando AT para mandar un SMS.
  delay(1000);
  SIM900.println("AT + CMGS = \"8294596552\""); //Numero al que vamos a enviar el mensaje.
  delay(1000);
  SIM900.println("SISTEMA DESCONECTADO"); // Texto del SMS.
  delay(100);
  SIM900.println((char)26); //Comando de finalizacion ^Z.
  delay(100);
  SIM900.println();
  delay(5000); //Esperamos un tiempo para que envíe el SMS.
  Serial.println("SMS enviado");
  
  incoming_char = '0'; //Se asigna el valor cero.
}



