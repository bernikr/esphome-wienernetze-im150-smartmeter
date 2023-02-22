#include "esphome.h"
#include <FastCRC.h>
#include <Crypto.h>
#include <AES.h>
#include <CTR.h>

static const char* IM150_VERSION = "0.0.1a";
static const char* TAG = "im150";

namespace esphome
{
    namespace im150
    {
        class IM150Meter : public Component, public uart::UARTDevice
        {
            public:
                IM150Meter(uart::UARTComponent *parent);

                void setup() override;
                void loop() override;

                void set_key(byte key[]);
                void set_sensors(
                    sensor::Sensor *active_energy_pos,
                    sensor::Sensor *active_energy_neg,
                    sensor::Sensor *reactive_energy_pos,
                    sensor::Sensor *reactive_energy_neg,
                    sensor::Sensor *active_power_pos,
                    sensor::Sensor *active_power_neg,
                    sensor::Sensor *reactive_power_pos,
                    sensor::Sensor *reactive_power_neg
                    );

            private:
                std::vector<byte> receiveBuffer; // Stores the packet currently being received
                unsigned long lastRead = 0; // Timestamp when data was last read
                int readTimeout = 100; // Time to wait after last byte before considering data complete

                FastCRC16 CRC16;
                CTR<AES128> ctraes128;

                uint8_t key[16]; // Stores the decryption key
                sensor::Sensor *active_energy_pos   = NULL;
                sensor::Sensor *active_energy_neg   = NULL;
                sensor::Sensor *reactive_energy_pos = NULL;
                sensor::Sensor *reactive_energy_neg = NULL;
                sensor::Sensor *active_power_pos    = NULL;
                sensor::Sensor *active_power_neg    = NULL;
                sensor::Sensor *reactive_power_pos  = NULL;
                sensor::Sensor *reactive_power_neg  = NULL;

                int bytes_to_int(byte bytes[], int left, int right);
                void handle_message(std::vector<byte> msg);
        };
    }
}