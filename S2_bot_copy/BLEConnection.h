#ifndef BLE_CONNECTION_H
#define BLE_CONNECTION_H

#include <NimBLEDevice.h>

#define SENSOR_COUNT 8
#define SERVICE_UUID        "496335ec-f73e-46c6-b743-c1ebd052703c"
#define DATA_CHAR_UUID      "6761501f-a33c-446f-aba8-c615b56392ec"
#define CONTROL_CHAR_UUID   "099f102d-d5c3-4a8d-9b0c-36f21f6ed4d9"
#define LINE_CHAR_UUID      "2c7f3f4e-0c8a-4d5f-9f23-4a6b7a1c9b10"
#define TIME_CHAR_UUID      "7b2c1f8a-9453-4b6c-a4f0-22c2df8b0c21"
#define P_CHAR_UUID         "9cec4ad9-5ac2-4bce-a6fc-95807282c60f"
#define I_CHAR_UUID         "f99b0cad-3ebe-46a9-b68a-265fa7be0fe6"
#define D_CHAR_UUID         "d1f23061-e268-4e96-8e56-974711ab37bb"

volatile uint8_t controlFlag = 0;
static uint32_t lastAdvKick;

class ControlCallbacks;
class ControlCallbacksPID;
class ServerCallbacks;

typedef struct __attribute__((packed)) {
  uint16_t sensors[SENSOR_COUNT];
  uint8_t  rpm1;
  uint8_t  rpm2;
  uint8_t  digital_sensors[SENSOR_COUNT];
} TelemetryPacket;

typedef struct __attribute__((packed)) {
  uint16_t distancia;
  uint8_t  linha_tipo;
} LinePacket;

static void startAdv() {
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    if (adv->isAdvertising()) return;
    adv->clearData();
    adv->addServiceUUID(SERVICE_UUID);
    NimBLEDevice::startAdvertising();
    lastAdvKick = millis();
}

class ControlCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) {
    std::string v = c->getValue();
    if (!v.empty()) {
      uint8_t b = static_cast<uint8_t>(v[0]);
      controlFlag = b;
      c->setValue(&b, 1);
      c->notify();
    }
  }
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override { onWrite(c); }
};

class ControlCallbacksPID : public NimBLECharacteristicCallbacks {
  float* ref;
public:
  ControlCallbacksPID(float* r) : ref(r) {}
  void onWrite(NimBLECharacteristic* c) {
    std::string v = c->getValue();
    if (v.size() >= 4) {
      memcpy(ref, v.data(), 4);
      c->setValue((uint8_t*)ref, 4);
      c->notify();
    }
  }
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override { onWrite(c); }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  // void onConnect(NimBLEServer* s) { }
  void onDisconnect(NimBLEServer*) { startAdv(); }
};

class BLEConnection {
public:
    static const uint32_t ADV_KICK_MS = 5000;
 
private:
    NimBLECharacteristic* dataChar;
    NimBLECharacteristic* controlChar;
    NimBLECharacteristic* lineChar;
    NimBLECharacteristic* timeChar;
    NimBLECharacteristic* pChar;
    NimBLECharacteristic* iChar;
    NimBLECharacteristic* dChar;
    TelemetryPacket pkt{};
    LinePacket lp{};

    float gP = 0.0f, gI = 0.0f, gD = 0.0f;

public:
    BLEConnection() {
        
    }

    void setup_ble(){
        NimBLEDevice::init("32ESP");
        NimBLEDevice::setDeviceName("32ESP");
        NimBLEDevice::setPower(ESP_PWR_LVL_P7);
        NimBLEDevice::setMTU(185);
        NimBLEDevice::setSecurityAuth(false, false, false);

        NimBLEServer* server = NimBLEDevice::createServer();
        server->setCallbacks(new ServerCallbacks());

        NimBLEService* svc = server->createService(SERVICE_UUID);

        // DATA
        dataChar = svc->createCharacteristic(DATA_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        dataChar->createDescriptor("2901")->setValue("sensors[8], rpm1, rpm2, digital_sensors[8]");

        lineChar = svc->createCharacteristic(LINE_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        lineChar->createDescriptor("2901")->setValue("distancia(uint16), linha_tipo(uint8)");

        timeChar = svc->createCharacteristic(TIME_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        timeChar->createDescriptor("2901")->setValue("tempo(uint32)");

        // CONTROL
        controlChar = svc->createCharacteristic(CONTROL_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
        controlChar->createDescriptor("2901")->setValue("controlFlag (uint8)");
        controlChar->setCallbacks(new ControlCallbacks());
        uint8_t initB = controlFlag;
        controlChar->setValue(&initB, 1);

        // PID
        pChar = svc->createCharacteristic(P_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
        pChar->createDescriptor("2901")->setValue("P(float32)");
        pChar->setCallbacks(new ControlCallbacksPID(&gP));
        pChar->setValue((uint8_t*)&gP, 4);

        iChar = svc->createCharacteristic(I_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
        iChar->createDescriptor("2901")->setValue("I(float32)");
        iChar->setCallbacks(new ControlCallbacksPID(&gI));
        iChar->setValue((uint8_t*)&gI, 4);

        dChar = svc->createCharacteristic(D_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
        dChar->createDescriptor("2901")->setValue("D(float32)");
        dChar->setCallbacks(new ControlCallbacksPID(&gD));
        dChar->setValue((uint8_t*)&gD, 4);

        svc->start();
        startAdv();
    }
    void handleClientRequests() {
        if (NimBLEDevice::getServer()->getConnectedCount() == 0){
         if(!NimBLEDevice::getAdvertising()->isAdvertising() && millis() - lastAdvKick > ADV_KICK_MS) {
            startAdv();
         }
        }
    }

    void verifyClient(){
        
    }

    void setTelemetryData(uint16_t* sensors, uint8_t rpm1, uint8_t rpm2 , uint8_t* digital_sensors) {
        // Preencher os sensores
        for (int i = 0; i < SENSOR_COUNT; i++) {
            pkt.sensors[i] = sensors[i];
        }
        // Preencher os rpm
        pkt.rpm1 = rpm1;
        pkt.rpm2 = rpm2;

        // Preencher os sensores digitais
        for (int i = 0; i < SENSOR_COUNT; i++) {
            pkt.digital_sensors[i] = digital_sensors[i];
        }
        dataChar->setValue((uint8_t*)&pkt, sizeof(pkt));
        dataChar->notify();
    }


    void setLineData(uint16_t distancia, uint8_t  linha_tipo, uint32_t tempo) {
        lp.distancia = distancia ;
        lp.linha_tipo = linha_tipo;
        lineChar->setValue((uint8_t*)&lp, sizeof(lp));
        lineChar->notify();
        timeChar->setValue((uint8_t*)&tempo, sizeof(tempo));
        timeChar->notify();

    }

};
#endif