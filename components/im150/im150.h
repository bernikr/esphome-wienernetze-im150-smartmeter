#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include <FastCRC.h>
#include <Crypto.h>
#include <AES.h>
#include <CTR.h>

#define IM150_SENSOR(name)  \
protected:                  \
    sensor::Sensor *name{}; \
                            \
public:                     \
    void set_##name(sensor::Sensor *name) { this->name = name; }

#define IM150_TEXT_SENSOR(name)      \
protected:                           \
    text_sensor::TextSensor *name{}; \
                                     \
public:                              \
    void set_##name(text_sensor::TextSensor *name) { this->name = name; }

namespace esphome
{
    namespace im150
    {
        static const char *IM150_VERSION = "0.1.3-beta.1";
        static const char *TAG = "im150";

        class IM150 : public Component, public uart::UARTDevice
        {
            IM150_SENSOR(active_energy_pos)
            IM150_SENSOR(active_energy_neg)
            IM150_SENSOR(reactive_energy_pos)
            IM150_SENSOR(reactive_energy_neg)
            IM150_SENSOR(active_power_pos)
            IM150_SENSOR(active_power_neg)
            IM150_SENSOR(reactive_power_pos)
            IM150_SENSOR(reactive_power_neg)

            IM150_TEXT_SENSOR(active_energy_pos_raw)
            IM150_TEXT_SENSOR(active_energy_neg_raw)
            IM150_TEXT_SENSOR(reactive_energy_pos_raw)
            IM150_TEXT_SENSOR(reactive_energy_neg_raw)

        public:
            void dump_config() override;
            void loop() override;

            void set_key(const uint8_t *key) { this->key = key; }

        private:
            std::vector<uint8_t> receiveBuffer; // Stores the packet currently being received
            unsigned long lastRead = 0;         // Timestamp when data was last read
            int readTimeout = 100;              // Time to wait after last byte before considering data complete

            FastCRC16 CRC16;
            CTR<AES128> ctraes128;

            const uint8_t *key; // Stores the decryption key

            int bytes_to_int(uint8_t bytes[], int left, int right);
            void handle_message(std::vector<uint8_t> msg);
        };
    }
}