#include "DHT.h"

#define LEDSTATUS 5  // led de status conectado al pin digital D1 (GPIO5)
#define PINTEMPERATURA 4  // sensor de temperatura conectado al pin digital D2 (GPIO4)
#define PINBOMBA 12 // bomba de agua conectada al pin digital D6 (GPIO12)
#define PINHUMEDAD 0  // sensor de humedad conectado al pin analógico A0
#define INTERVALO 60  // intervalo entre mediciones cuando NO hay que regar (segundos)
#define CONSTANTECAUDAL 50 // constante experimental según la potencia de la bomba y la superficie de riego
#define CONSTANTEABSORCION 10 // constante experimental según el tamaño de la maceta y la evapotranspiración de la planta

DHT dht(PINTEMPERATURA, DHT22);  // inicializo sensor de temperatura ambiente

const int humedadAire = 840;  // valor de humedad calibrado en aire
const int humedadAgua = 470;  // valor de humedad calibrado en agua
int rangoHumedad = humedadAire - humedadAgua;  // rango de humedad calibrado

float temperaturaMedida = 0;
int humedadMedida = 0;  // valor medido de resistividad, inversamente proporcional a la humedad absoluta
int humedadMedidaAbsoluta = 0;  // valor medido de humedad absoluta
int humedadMedidaRelativa = 0;  // valor medido de humedad relativa (porcentaje entre humedadAire y humedadAgua)
int humedadDeseadaRelativa =  80;  // valor esperado de humedad en % siendo 0% = seco (humedadAire) y 100% = humedadAgua)
int diferenciaHumedad = 0;  // valor de error entre humedad medida y humedad deseada

int proximaMedicion = 0;
int duracionBomba = 0;

void esperarTiempo(int tiempo){
  Serial.print("Esperando por ");
  Serial.print(tiempo);
  Serial.print(" segundos hasta la próxima medición...");

  int tiempoTranscurrido = 0;
  while (tiempoTranscurrido < tiempo){
    Serial.print(".");
    delay(1000);
    tiempoTranscurrido++;
  } 
}

void setup() {
  pinMode(LEDSTATUS, OUTPUT);
  pinMode(PINTEMPERATURA, INPUT);
  pinMode(PINBOMBA, OUTPUT);

  Serial.begin(9600);  // abro puerto serial con tasa 9600 bps
  
  Serial.println("");
  Serial.println("");
  Serial.println("    << Sistema de Riego >>    ");
  Serial.println("  Teoría de Control - UTN - FRBA  ");
  Serial.println("Nahuel Oyhanarte <noyhanarte@gmail.com>");
  Serial.println("Hernán Domingo <hernan.domingo.22@gmail.com>");
  Serial.println("Rodrigo De Luca <cdelucagallego@est.frba.utn.edu.ar>");
  Serial.println("Juan Manuel Constenla <juanconstenla@est.frba.utn.edu.ar>");

  Serial.println("");
  
  dht.begin();
  delay(2000);
}

void loop() {
  Serial.println("");
  digitalWrite(LEDSTATUS, HIGH);
  
  // mido temperatura ambiente
  temperaturaMedida = dht.readTemperature();

  // registro temperatura ambiente
  if (!isnan(temperaturaMedida)) {
    Serial.print(F("Temperatura: "));
    Serial.print(temperaturaMedida);
    Serial.println(F("°C"));
  } else {
    Serial.println(F("Error al leer del sensor de temperatura."));
    temperaturaMedida = 20;  // valor default en caso de error de medición
  }  

  // mido humedad en suelo
  humedadMedida = analogRead(PINHUMEDAD); 
  humedadMedidaAbsoluta = 1023 - humedadMedida;
  humedadMedidaRelativa = (humedadMedidaAbsoluta - (1023 - humedadAire)) * 100 / rangoHumedad;

  // registro humedad en suelo
  Serial.print("Humedad medida absoluta: ");
  Serial.println(humedadMedidaAbsoluta);  
  Serial.print("Humedad medida relativa: ");
  Serial.println(humedadMedidaRelativa);
  Serial.print("Humedad deseada relativa: ");
  Serial.println(humedadDeseadaRelativa);

  // calculo señal de error
  diferenciaHumedad = humedadDeseadaRelativa - humedadMedidaRelativa;

  if (diferenciaHumedad > 0) {
    Serial.println("Indicación: Hay q regar!");
    
    // calculo duración de accionamiento de la bomba (segundos)
    duracionBomba = diferenciaHumedad * temperaturaMedida / CONSTANTECAUDAL;

    // activo la bomba (delay requiere milisegundos)
    Serial.print("Regando por ");
    Serial.print(duracionBomba);
    Serial.print(" segundos...");
    
    digitalWrite(PINBOMBA, HIGH);

    int tiempoRegado = 0;
    while (tiempoRegado < duracionBomba){
      Serial.print(".");
      delay(1000);
      tiempoRegado++;
    }
    Serial.println("");

    digitalWrite(PINBOMBA, LOW);
    
    // calculo tiempo hasta la próxima medición
    proximaMedicion = duracionBomba * CONSTANTEABSORCION;
    
    // espero hasta la próxima medición (delay requiere milisegundos)
    esperarTiempo(proximaMedicion);
      
  } else {
    Serial.println("Indicación: NO hay que regar."); 
    
    // espero intervalo pre-seteado antes de volver a chequear (delay requiere milisegundos)
    esperarTiempo(INTERVALO);
  
  }
  Serial.println("");
}
