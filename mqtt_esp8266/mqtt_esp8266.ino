#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// DECLARACION VARIABLES MOVER CARRO
int tr = 13; // Envia informacion arduino al sensor para controlar el sensor (PIN D7)
int ec = 14; // Envia la informacion para leerla con el arduino (PIN D5)

int in1 = 2;  // Pin 1 que controla el sentido de giro Motor A (PIN D4)
int in2 = 4;  // Pin 2 que controla el sentido de giro Motor A (PIN D2)

int cn = 0; // Variable para guardar datos del contador de objetos dentro del
// rango donde se encuentran los objetos a revisar

// tr = trigger
// ec = echo

// DECLARACION RED A LA CUAL SE VA A CONECTAR EL ESP8266 Y EL SERVIDOR MQTT.

const char* ssid = "NDavila";
const char* password = "davila0109";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[80];
int value = 0;

////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

void setup_wifi() {


  delay(10);
  // Orden para iniciacion de la conexion con la red WIFI
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Orden para encender el led BUiltin si se recibio el primer 1 de verificacion de conexion
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW); // se deja el BUILTIN LED en estado bajo o en 0
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Apaga led haciendo que el voltaje sea alto
  }

}

void reconnect() {
  // Bucle que se realiza hasta que se establezca la conexion MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Orden para crear una ID de cliente aleatoria
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Intento de establecimiento de conexion
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Orden para que una vez este conectado publique un mensaje
      // en este caso el mensaje "INICIO ACCION"
      client.publish("Salida0109", "INICIO ACCION");
      // Orden para empezar a recibir suscripciones en este ejemplo
      // no se aplica la recepcion de mensajes por lo cual se puede 
      // omitir esto
      client.subscribe("Salida");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Orden para que espere 5 segundo mas para ejecutar el bucle
      // de intento de establecer conexion nuevamente.
      delay(5000);
    }
  }
}

void setup() {

{   
  // DECLARACION DE ACCIONES QUE REALIZA EL CARRO SEGUN LAS CONDICIONES DADAS
  
  Serial.begin(115200);
  pinMode(tr,OUTPUT);
  pinMode(ec,INPUT);

      
  pinMode(in1, OUTPUT); // Configura  los pines como salida MOTOR A
  pinMode(in2, OUTPUT);   
}

  pinMode(BUILTIN_LED, OUTPUT);     // Indica por medio del led builtin que esta listo para el envio 
  Serial.begin(115200);             // el MQTT server
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

   // CONDICIONES PARA REALIZACION DE ACCIONES DEL CARRO

  long duracion;
  long distancia;

  digitalWrite(tr,LOW);
  delayMicroseconds(5);    // Se apaga el trigger para capturar una onda de sonido limpia
  digitalWrite(tr,HIGH);
  delayMicroseconds(10);   // Se activa el trigger para envio onda de sonido para ver distacion con el objeto
  digitalWrite(tr,LOW);

  duracion = pulseIn(ec,HIGH); // Activa Echo para conteo de onda sonica enviada por trigger.
  duracion = duracion/2;  // Se hace para tomar medida entre el sensor y el objeto en un solo sentido.
  
  distancia = duracion/29; // Ecuacion sencilla para hallar distancia en CM desde el sensor hasta los objetos

  Serial.println(distancia);

  delayMicroseconds(50);

  if( distancia <= 25) 
  {
  digitalWrite(in1, LOW);  // PARA
  digitalWrite(in2, LOW);
//  digitalWrite(in3, LOW);
//  digitalWrite(in4, LOW);
  Serial.println("Detener Carro");
  delay(1500);
  }
  else if ( distancia > 41)
  {
  digitalWrite(in1, HIGH);  // ANDA
  digitalWrite(in2, HIGH);
//  digitalWrite(in3, HIGH);
//  digitalWrite(in4, HIGH);

  Serial.println("Siga buscando");
  delay(1500);
  }
    else if ( distancia > 25 < 40)
  {
  cn++; // Añade un valor al contador
  Serial.println("Objetos dentro del rango detectado");
  delay(10);
  Serial.println(cn);// Imprime el valor del contador en consola
  digitalWrite(in1, LOW);  // PARA TEMPORALMENTE
  digitalWrite(in2, LOW);
//  digitalWrite(in3, LOW); 
//  digitalWrite(in4, LOW); 
  delay(1500);
  digitalWrite(in1, HIGH);  // ANDA
  digitalWrite(in2, HIGH);
//  digitalWrite(in3, HIGH);  
//  digitalWrite(in4, HIGH);
  delay(100);
  }
  
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

  // PUBLICACION DE LA PUBLICACION DEL MENSAJE MQTT HACIA NODE-RED
   
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50,"Distancia: %ld No Dato: %ld" ,distancia, value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("Salida0109", msg);
  }
}
