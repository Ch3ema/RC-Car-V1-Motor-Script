#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SparkFun_TB6612.h>

//TB6612FNG motor driver pins
const int ain1 = 25;
const int ain2 = 26;
const int pwma = 27;
const int bin1 = 32;
const int bin2 = 33;
const int pwmb = 14;
const int stby = 12;

const int led_pin = 4;
const int buzzer_pin = 5;

const int buzzer_freq = 2000;

//offset of 1 means the motor spins forward on a positive drive value
//flip to -1 if a motor runs backwards after wiring
Motor motor_a = Motor(ain1, ain2, pwma, 1, stby);
Motor motor_b = Motor(bin1, bin2, pwmb, 1, stby);

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

int speed_value = 150;
String last_command = "C";
bool led_state = false;
bool buzzer_state = false;

bool device_connected = false;

void apply_command(String command) {
  if (command == "A") {
    motor_a.drive(speed_value);
    motor_b.drive(speed_value);
  }
  else if (command == "B") {
    motor_a.drive(-speed_value);
    motor_b.drive(-speed_value);
  }
  else if (command == "LEFT") {
    motor_a.drive(-speed_value);
    motor_b.drive(speed_value);
  }
  else if (command == "RIGHT") {
    motor_a.drive(speed_value);
    motor_b.drive(-speed_value);
  }
  else if (command == "C") {
    motor_a.brake();
    motor_b.brake();
  }
}

void stop_all() {
  motor_a.brake();
  motor_b.brake();
  noTone(buzzer_pin);
  buzzer_state = false;
  last_command = "C";
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    device_connected = true;
    Serial.println("Connected");
  }

  //cut the motors on disconnect, then re-advertise so the phone
  //caan reconnect without a reset.
  void onDisconnect(BLEServer *server) {
    device_connected = false;
    stop_all();
    Serial.println("Disconnected, restarting advertising");
    BLEDevice::startAdvertising();
  }
};

class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    String value = characteristic->getValue();
    if (value.length() == 0) return;

    Serial.print("Received: ");
    Serial.println(value);

    //slider sends "Speed_0" through "Speed_100", scaled to the
    //8-bit PWM range the driver expects
    if (value.startsWith("Speed_")) {
      int slider_value = value.substring(6).toInt();
      speed_value = map(slider_value, 0, 100, 0, 255);
      Serial.print("Speed: ");
      Serial.println(speed_value);
    }
    else if (value == "LIGHT") {
      led_state = !led_state;
      digitalWrite(led_pin, led_state ? HIGH : LOW);
    }
    else if (value == "HORN") {
      buzzer_state = !buzzer_state;
      if (buzzer_state) {
        tone(buzzer_pin, buzzer_freq);
      } else {
        noTone(buzzer_pin);
      }
    }
    else {
      last_command = value;
    }

    //re-applied on every write so a speed change takes effect
    //while the car is already moving
    apply_command(last_command);
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(stby, OUTPUT);
  digitalWrite(stby, HIGH);

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, LOW);

  BLEDevice::init("RC_CAR");

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *control = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  control->setCallbacks(new ControlCallbacks());

  service->start();
  BLEDevice::getAdvertising()->start();

  Serial.println("Advertising as RC_CAR, waiting for connection");
}

void loop() {
  //all control happens in the BLE write callback
}