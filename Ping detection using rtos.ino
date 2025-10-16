#include <WiFi.h>
#include <ESPping.h>
#include "ThingSpeak.h"
#include <QMC5883LCompass.h>

QMC5883LCompass compassSensor;
// CompassData data;
struct CompassData {
  float x;
  float y;
  float z;
  float d;
  
	};
  CompassData data;

const char* ssid = "OnePlus34";
const char* password = "abcdefgh";

unsigned long myChannelID = 2504650;
const char * myWriteAPIKey = "IZ0IC04CV0RA5HNY";
//QueueHandle_t Sensor2Mailbox; // Mailbox for QMC5883L sensor
SemaphoreHandle_t semaphore;
// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

int sensorValueToSend = 1;

WiFiClient client;
TaskHandle_t pingtaskHandle;
float avg_time_ms;

void pingtask(void* pvParameters) {
  // int ping = 0 ;
  while (1) {
    xSemaphoreTake(semaphore, portMAX_DELAY);
    // IPAddress ip (8,8,4,4);
    bool pingBool = Ping.ping("www.google.com");
    if(pingBool)
    {
      Serial.println("Ping success at start");
    }
    else{
      Serial.println("Ping failed at start");
    }
    avg_time_ms = Ping.averageTime();
    xSemaphoreGive(semaphore);
    //long pingTime = ping(WiFi.192,168,208,185);
    if (avg_time_ms > 0) {
    Serial.print("Ping to gateway: ");
    Serial.print(avg_time_ms);
    Serial.println(" ms");
    } 
    else {
    Serial.println("Ping failed!");
    }
    vTaskDelay(pdMS_TO_TICKS(3000)); // Delay between readings (3 seconds)
  }
}
void CompTask(void *pvParameters) {
  // CompassData data;
  int a,b;
  while (1) {
    if(xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE){
      compassSensor.read();
      data.x = compassSensor.getX();
      data.y = compassSensor.getY();
      data.z = compassSensor.getZ();
      xSemaphoreGive(semaphore);
      Serial.print("X= ");
      Serial.println(data.x);
      Serial.print("Y= ");
      Serial.println(data.y);
      Serial.print("Z= ");
      Serial.println(data.z);
    }
    vTaskDelay(3000); // Adjust the delay as needed
    }
}

void thingSpeaktask(void* pvParameters) {
while (1) {
  if ((millis() - lastTime) > timerDelay) {
    if(xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE){
    ThingSpeak.setField(1,avg_time_ms);
    ThingSpeak.setField(2,data.x);
    ThingSpeak.setField(3,data.y);
    ThingSpeak.setField(4,data.z);
    int x = ThingSpeak.writeFields(myChannelID,myWriteAPIKey);
    xSemaphoreGive(semaphore);
    if(x == 200){
    Serial.println("Successfuly sent to Thingspeak channel.");
    }
    else{
    Serial.println("Problem updating channel. HTTP error code: "+ String(x));
    }
    }
    sensorValueToSend++;
    lastTime = millis();
    } 
  }
}
void setup() {
Serial.begin(115200);
Wire.begin(25,26);
//compassSensor.init();
compassSensor.init();
  semaphore = xSemaphoreCreateBinary();

WiFi.begin(ssid, password);
Serial.print("Connecting to WiFi: ");
Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.print("Connected, IP address: ");
Serial.println(WiFi.localIP());
xTaskCreate(CompTask, "CompTask", 2048, NULL, 1, NULL);
xTaskCreate(pingtask, "PINGTASK", 2048, NULL, 1, NULL);
xTaskCreate(thingSpeaktask, "thingSpeaktask", 2048, NULL, 1, NULL);
ThingSpeak.begin(client); 
xSemaphoreGive(semaphore);
}
void loop() {
// loop is not used in this example as tasks handle continuous operation
}