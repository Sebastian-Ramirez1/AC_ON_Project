#include <stdio.h>
#include <PubSubClient.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <math.h>

// WiFi and MQTT parameters
const char *SSID = "NETGEAR17";
const char *PWD = "chummybutter668";
// const char *SSID = "Samsung Galaxy S8_3090"; // WiFi Name
// const char *PWD = "bsrf7877";                // WiFi Password
const char *mqttServer = "3.226.110.83";        // Server Address
const int mqttPort = 1883;                      // Server Port
const char *mqttUser = nullptr;                 // Broker username
const char *mqttPWD = nullptr;                  // Broker PWD

// Gobal general parameters
char state = 'a';
bool calibrated = false;
float cal_v[2];
float cal_t[2];
int first_second = 0;
float voltageOut;
float voltageReadings[100];
float temperatureC;
float temperatureF;
float temperatureK;
char tempK[50];
char tempC[50];
char tempF[50];

// Miscalleneous
float byteP_to_float(byte *s, int length)
{
    float result = 0;
    int radix_point = 0;
    int sign = 1;

    for (int i = 0; i < length; i++)
    {
      if ((char)s[i] == '.')
      {
        break;
      }
      radix_point++;
    }

    for (int i = 0; i < length; i++)
    {
      if((char)s[i] == '-')
      {
        sign *= -1;
        i++;
      }

      if (i < radix_point)
      {
        result += pow(10, radix_point - (i+1)) * (((char)s[i]) - 48);
      }
      else if (i > radix_point)
      {
        result += pow(10, (radix_point - i)) * (((char)s[i]) - 48);
      }
        
    }

    return sign * result;
}

// Struct
struct func_param{
  float m;
  float b;
};

WiFiClient wifiClient;

PubSubClient mqttClient(wifiClient);

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected WiFi.");
  Serial.println();
}

//MQTT Parameter, Setup Function and callback


void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("MQTT Connected.");

        // Annouce Connection
        mqttClient.publish("Connection notifier", "Successfull Connection!");  
        mqttClient.subscribe("State");
        mqttClient.subscribe("testP");
        
      }

      else {delay(1000);}
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println();
  Serial.print("Message has arrived in topic: ");
  Serial.println(topic);  
  Serial.print("Message:");

  if (!strcmp(topic,"testP"))
  {
    float incoming = byteP_to_float(payload, length);
    cal_t[first_second] = incoming;
    cal_v[first_second] = read_voltage();
    Serial.println();
    first_second++;
  }
  else if (!strcmp(topic, "State"))
  {
    char incoming = (char) *payload;
    if (incoming == 'c')
    {
      Serial.println("Calibration State");
      first_second = 0;
    }
    state = incoming;
  }
  // for (int i = 0; i < length; i++)
  // {
  //   Serial.print((char)payload[i]);
  // }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int inputs = 0;
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);

  connectToWiFi(); //initialize WiFi connection
  setupMQTT(); //initialize MQTT
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    reconnect();
  }
  mqttClient.loop();

  // float sum = 0;
  float sensorVoltage;
  // int i;

  // Sensor value cleaning
  // for (i = 0; i < 100; ++i)
  // {
  //   // Read analog value 
  //   int sensorValue = adc1_get_raw(ADC1_CHANNEL_0);
  //   voltageOut = (sensorValue * 3300) / 4095;
  //   sum = sum + voltageOut;
  //   delay(5);
  // }

  sensorVoltage = read_voltage() /*sum / 100*/;

  switch(state)
  {
    case 'c':
      struct func_param parameters;
      if (!calibrated)
      {
        if (first_second > 1)
        {
          Serial.println(cal_t[0]);
          Serial.println(cal_t[1]);
          parameters = calibrate(cal_v, cal_t);
          calibrated = !calibrated;
          
        }
        
        return;
      }
      Serial.println("Hello");
      temperatureC = voltage_to_temp_conC(sensorVoltage, parameters);
      Serial.println(temperatureC);
      break;

    case 'a':
      Serial.println("Automated State");

      // calculate temperature
      // temperatureK = sensorVoltage / 10;
      // sprintf(tempK, "%.2f", temperatureK);
      temperatureC = (3800 * 5.5)/sensorVoltage;
      sprintf(tempC, "%.2f", temperatureC);
      temperatureF = (temperatureC * 1.8) + 32;
      sprintf(tempF, "%.2f", temperatureF);

      // Print temperature values
      Serial.print("Temperature(ºC): ");
      Serial.print(temperatureC);
      Serial.print("  Temperature(ºF): ");
      Serial.print(temperatureF);
      Serial.print("  Voltage(mV): ");
      Serial.println(sensorVoltage);
      mqttClient.publish("test", tempC);
      break;
  }


  // delay(10000);
}

float voltage_to_temp_conC(float voltage, struct func_param parameter)
{
  return (voltage*parameter.m) + parameter.b;
}

struct func_param calibrate(float *v, float *t)
{
  float m = t[1] - t[0]/(v[1] - v[0]);
  float b = -(m * v[0]) + t[0];

  struct func_param param = {m, b};

  return param;
}

float read_voltage()
{
  float sum = 0;
  float result;

  // Sensor value cleaning
  for (int i = 0; i < 100; ++i)
  {
    // Read analog value 
    int sensorValue = adc1_get_raw(ADC1_CHANNEL_0);
    voltageOut = (sensorValue * 3800) / 4095;
    sum = sum + voltageOut;
    delay(5);
  }

  result = sum/100;
  return result;
}

