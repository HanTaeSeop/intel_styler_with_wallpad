#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <DHT.h>
#include <Servo.h>  // Include the Servo library

#define AP_SSID "embA"
#define AP_PASS "embA1234"
#define SERVER_NAME "10.10.14.51"
#define SERVER_PORT 5001
#define LOGID "HTS_ARD"
#define PASSWD "PASSWD"

#define DHTPIN 4
#define WIFIRX 8  // 8: RX --> ESP8266 TX
#define WIFITX 7  // 7: TX --> ESP8266 RX
#define CMD_SIZE 50
#define ARR_CNT 5
#define DHTTYPE DHT11
#define WATER_SENSOR_PIN A0  // A0 핀을 물 수위 센서로 설정
#define SERVO_PIN 6  // Pin 6 is connected to the servo motor
#define FAN_PIN 9  // Pin for controlling the fan
#define LED_PIN 10

char userPH[10] = "HTS_AND";
char sendId[10] = "ALLMSG";
char sendBuf[CMD_SIZE];

unsigned int secCount;
float temp = 0.0;
float humi = 0.0;
float PRhumi = 0.0;
bool sensorFlag = false;
bool timerIsrFlag = false;
bool servoIsOpen = false;  // Track the state of the servo (open or closed)
bool fanIsOn = false;  // Track the state of the fan (on or off)
bool manualControl = false; // Track if manual control is active
bool sendFlag = false;
int waterThreshold = 500;
int waterLevel = 0;
int LEDMODE = 1;
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;
Servo myServo;  // Create a Servo object to control the servo

void setup() {
  Serial.begin(115200); // DEBUG
  wifi_Setup();

  MsTimer2::set(1000, timerIsr); // 1000ms period (1 second)
  MsTimer2::start();

  dht.begin();
  
  // Initialize fan control pin as output
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Attach the servo to pin 6
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Start the servo at 0 degrees (closed position)

}

void loop() {
  if (client.available()) {
    socketEvent();
  }
  
  
  humi = dht.readHumidity();
  if((humi-PRhumi) >= 5 || (humi-PRhumi) <= -5 )
  {
    sensorFlag = true;
    PRhumi = humi;
  }
  waterLevel = analogRead(WATER_SENSOR_PIN);
  if (waterLevel > waterThreshold) 
  {
    LEDMODE = 0;
    if(timerIsrFlag)
    {
      timerIsrFlag = false;
      if(!(secCount%15))
      {
        sensorFlag = true;
      }
    }
  }
  else  
  {
    LEDMODE = 1;
  }
  switch(LEDMODE){
    case 0 :
      digitalWrite(LED_PIN,HIGH);
    case 1 : 
      digitalWrite(LED_PIN,LOW);
  }

  if (sensorFlag) 
  { // Every 5 seconds, get sensor data
    temp = dht.readTemperature();
    // Instead of converting to string, send integers directly
    sprintf(sendBuf, "[%s]SENSOR@%d@%d@%d\n", sendId, (int)temp, (int)humi, waterLevel);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();
    sensorFlag = false;
    }
  controlFan(humi);  // Fan control based on humidity
}


void socketEvent() {
  char recvBuf[CMD_SIZE] = {0};
  int len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();

  char *pToken;
  char *pArray[ARR_CNT] = {0};
  int i = 0;

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL) {
    pArray[i] = pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }

  if (!strcmp(pArray[1], "MOTOR")) {
    if (!strcmp(pArray[2], "OPEN")) {
      openServo();
    } else if (!strcmp(pArray[2], "CLOSE")) {
      closeServo();
    }
  }

  if (!strcmp(pArray[1], "FAN")) {
    if (!strcmp(pArray[2], "ON")) {
      manualControl = true;  // Enable manual control for the fan
      turnFanOn();  // Ensure fan is turned on
    } else if (!strcmp(pArray[2], "OFF")) {
      manualControl = false; // Disable manual control, fan will be controlled by humidity
      turnFanOff();  // Turn the fan off (if necessary)
    }
  }
  if (!strcmp(pArray[1], "GET")) {
    if (!strcmp(pArray[2], "SENSOR")) {
    sprintf(sendBuf, "[%s]SENSOR@%d@%d@%d\r\n", sendId, (int)temp, (int)humi, waterLevel);
    client.write(sendBuf, strlen(sendBuf));
    client.flush();
    }
  }

}

void timerIsr() {
  timerIsrFlag = true;
  secCount++;
}

void wifi_Setup() {
  wifiSerial.begin(38400);
  wifi_Init();
  server_Connect();
}

void wifi_Init() {
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
    } else {
      break;
    }
  } while (1);

  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
    Serial.println("Attempting to connect to WPA SSID...");
  }

  Serial.println("Connected to WiFi network");
}

int server_Connect() {
  if (client.connect(SERVER_NAME, SERVER_PORT)) {
    Serial.println("Connected to server");
    client.print("[" LOGID ":" PASSWD "]\r\n");
  } else {
    Serial.println("Server connection failed");
  }
}

// Function to send "FULL" message
void sendWaterFullMessage() {
  sprintf(sendBuf, "[%s]FULL\r\n", userPH);
  client.write(sendBuf, strlen(sendBuf));
  client.flush();
}

// Function to control the fan based on humidity
void controlFan(float humidity) {
  // If manual control is not active, control based on humidity
  if (!manualControl) {
    if (humidity > 50 && !fanIsOn) {
      turnFanOn();
    } else if (humidity < 45 && fanIsOn) {
      turnFanOff();
    }
  }
}

// Function to open the servo
void openServo() {
  myServo.write(150);
  servoIsOpen = true;
}

// Function to close the servo
void closeServo() {
  myServo.write(10);
  servoIsOpen = false;
}

// Function to turn the fan on
void turnFanOn() {
  digitalWrite(FAN_PIN, HIGH);
  //analogWrite(FAN_PIN, 1000);
  fanIsOn = true;
  Serial.println("Fan turned ON");
}

// Function to turn the fan off
void turnFanOff() {
  digitalWrite(FAN_PIN, LOW);
  fanIsOn = false;
  Serial.println("Fan turned OFF");
}
