#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Soil sensor setup
const int AirValue = 900;
const int WaterValue = 100;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;


// Update these with values suitable for your network.

const char* ssid = "SSID";
const char* password = "PASSWORDWIFI";
const char* mqtt_server = "mymqtt.server.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(60)
char msg[MSG_BUFFER_SIZE];
int value = 0;

boolean lightState = false;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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
  soilMoistureValue = analogRead(A0);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println(soilMoistureValue);
  Serial.println(soilmoisturepercent);
  snprintf (msg, MSG_BUFFER_SIZE, "{\"value\": %ld}", soilMoistureValue);
  client.publish("return/sensor1/soil/value", msg);
  snprintf (msg, MSG_BUFFER_SIZE, "{\"value\": %ld}", soilmoisturepercent);
  client.publish("return/sensor1/soil/percentage", msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("announcement/newdevice/soil/sensor1", "I\'m a Soil Moisture Sensor :D");
      // ... and resubscribe
      client.subscribe("give/sensor1/soil/value");
      client.subscribe("give/+/soil");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, LOW);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 600000) {
    lastMsg = now;
    ++value;
    digitalWrite(BUILTIN_LED, lightState);
    lightState = !lightState;
    snprintf (msg, MSG_BUFFER_SIZE, "Alive #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("announcement/alive/soil/sensor1", msg);
  }
}
