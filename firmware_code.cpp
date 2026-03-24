#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BUZZER_PIN 23
#define LED_PIN 2

#define SERVICE_UUID  "12345678-1234-1234-1234-1234567890ab"
#define CONTROL_UUID  "12345678-1234-1234-1234-1234567890ac"
#define STATUS_UUID   "12345678-1234-1234-1234-1234567890ad"

BLEServer* pServer = nullptr;
BLECharacteristic* controlChar = nullptr;
BLECharacteristic* statusChar = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

enum RangeBand {
  RANGE_IDLE = 0,
  RANGE_FAR,
  RANGE_MID,
  RANGE_NEAR,
  RANGE_VERY_CLOSE
};

RangeBand currentRange = RANGE_IDLE;
bool manualAlert = false;

unsigned long lastPatternMs = 0;
bool toneOn = false;

String rangeToString(RangeBand r) {
  switch (r) {
    case RANGE_FAR: return "FAR";
    case RANGE_MID: return "MID";
    case RANGE_NEAR: return "NEAR";
    case RANGE_VERY_CLOSE: return "VERY_CLOSE";
    default: return "IDLE";
  }
}

void stopTone() {
  ledcWriteTone(0, 0);
  toneOn = false;
}

void setTone(int freq) {
  ledcWriteTone(0, freq);
  toneOn = true;
}

void updateAlertPattern() {
  unsigned long now = millis();

  if (!manualAlert && currentRange == RANGE_IDLE) {
    stopTone();
    digitalWrite(LED_PIN, LOW);
    return;
  }

  if (manualAlert) {
    digitalWrite(LED_PIN, HIGH);
    setTone(2600);
    return;
  }

  switch (currentRange) {
    case RANGE_FAR:
      digitalWrite(LED_PIN, LOW);
      if (!toneOn && now - lastPatternMs >= 1600) {
        setTone(900);
        lastPatternMs = now;
      }
      if (toneOn && now - lastPatternMs >= 120) {
        stopTone();
        lastPatternMs = now;
      }
      break;

    case RANGE_MID:
      digitalWrite(LED_PIN, LOW);
      if (!toneOn && now - lastPatternMs >= 900) {
        setTone(1300);
        lastPatternMs = now;
      }
      if (toneOn && now - lastPatternMs >= 150) {
        stopTone();
        lastPatternMs = now;
      }
      break;

    case RANGE_NEAR:
      digitalWrite(LED_PIN, HIGH);
      if (!toneOn && now - lastPatternMs >= 350) {
        setTone(1900);
        lastPatternMs = now;
      }
      if (toneOn && now - lastPatternMs >= 180) {
        stopTone();
        lastPatternMs = now;
      }
      break;

    case RANGE_VERY_CLOSE:
      digitalWrite(LED_PIN, HIGH);
      setTone(2800);
      break;

    default:
      stopTone();
      digitalWrite(LED_PIN, LOW);
      break;
  }
}

void publishStatus() {
  String s = "STATE=" + rangeToString(currentRange) +
             ",CONNECTED=" + String(deviceConnected ? "1" : "0") +
             ",MANUAL=" + String(manualAlert ? "1" : "0");
  statusChar->setValue(s.c_str());
  if (deviceConnected) statusChar->notify();
  Serial.println(s);
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("Client connected");
    publishStatus();
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("Client disconnected");
  }
};

class ControlCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) override {
    std::string rx = c->getValue();
    if (rx.empty()) return;

    String val = String(rx.c_str());
    val.trim();
    val.toUpperCase();

    Serial.print("Received: ");
    Serial.println(val);

    manualAlert = false;

    if (val == "PING") {
      statusChar->setValue("PONG");
      if (deviceConnected) statusChar->notify();
      return;
    }

    if (val == "ALERT=ON") {
      manualAlert = true;
      publishStatus();
      return;
    }

    if (val == "ALERT=OFF") {
      manualAlert = false;
      currentRange = RANGE_IDLE;
      stopTone();
      digitalWrite(LED_PIN, LOW);
      publishStatus();
      return;
    }

    if (val == "RANGE=FAR") {
      currentRange = RANGE_FAR;
    } else if (val == "RANGE=MID") {
      currentRange = RANGE_MID;
    } else if (val == "RANGE=NEAR") {
      currentRange = RANGE_NEAR;
    } else if (val == "RANGE=VERY_CLOSE") {
      currentRange = RANGE_VERY_CLOSE;
    } else if (val == "RANGE=IDLE") {
      currentRange = RANGE_IDLE;
      stopTone();
      digitalWrite(LED_PIN, LOW);
    } else {
      statusChar->setValue("ERR:UNKNOWN_CMD");
      if (deviceConnected) statusChar->notify();
      return;
    }

    publishStatus();
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  stopTone();

  BLEDevice::init("Opti-Find");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* service = pServer->createService(SERVICE_UUID);

  controlChar = service->createCharacteristic(
    CONTROL_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  controlChar->setCallbacks(new ControlCB());

  statusChar = service->createCharacteristic(
    STATUS_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  statusChar->addDescriptor(new BLE2902());
  statusChar->setValue("READY");

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("Opti-Find advertising");
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(100);
    BLEDevice::startAdvertising();
    oldDeviceConnected = deviceConnected;
    Serial.println("Advertising restarted");
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  updateAlertPattern();
  delay(10);
}
