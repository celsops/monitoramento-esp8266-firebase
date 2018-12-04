#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Ticker.h>
#include "DHT.h"

// Set these to run example.
#define FIREBASE_HOST "monitoramento-27722.firebaseio.com"
#define FIREBASE_AUTH "9bjCUuILoz0YUCV3i25T4aTBdcpmuLdKpOrt2UdH"
#define WIFI_SSID "IFAL-INFO"
#define WIFI_PASSWORD "ifal-1nf0"

#define LAMP_PIN D3
#define LED_PIN D4
#define DHT_PIN D5
#define DHTTYPE DHT11

// Publique a cada 5 min
#define PUBLISH_INTERVAL 1000*60*5

DHT dht(DHT_PIN, DHTTYPE);
Ticker ticker;
bool publishNewState = true;

void publish(){
  publishNewState = true;
}

void setupPins(){

  pinMode(LAMP_PIN, OUTPUT);
  digitalWrite(LAMP_PIN, LOW);
  digitalWrite(LED_PIN,HIGH);
  pinMode(LED_PIN, OUTPUT);

  dht.begin();  
}

void setupWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setBool("lamp", false);
  Firebase.setBool("led", false);
}

void setup() {
  Serial.begin(9600);

  setupPins();
  setupWifi();    

  setupFirebase();

  // Registra o ticker para publicar de tempos em tempos
  ticker.attach_ms(PUBLISH_INTERVAL, publish);
}

void loop() {

  // Apenas publique quando passar o tempo determinado
  if(publishNewState){
    Serial.println("Publish new State");
    // Obtem os dados do sensor DHT 
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if(!isnan(humidity) && !isnan(temperature)){
      // Manda para o firebase
      Firebase.pushFloat("temperature", temperature);
      Firebase.pushFloat("humidity", humidity);    
      publishNewState = false;
    }else{
      Serial.println("Error Publishing");
    }
  }

  // Verifica o valor do led interno
  // LOW apagado
  // HIGH acesso
  bool ledValue = Firebase.getBool("led");
  digitalWrite(LED_PIN, ledValue ? LOW : HIGH );

  // Verifica o valor da lampada no firebase 
  bool lampValue = Firebase.getBool("lamp");
  digitalWrite(LAMP_PIN, lampValue ? LOW : HIGH );
  
  delay(200);
}
