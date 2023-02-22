#include "im150.h"


namespace esphome
{
    namespace im150
    {
        IM150Meter::IM150Meter(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

        void IM150Meter::setup()
        {
            ESP_LOGD(TAG, "IM150 smart meter component v%s started", IM150_VERSION);
        }
        
        void IM150Meter::loop()
        {
            
            unsigned long currentTime = millis();

            while(available()) {
                uint8_t c = read();
                this->receiveBuffer.push_back(c);

                this->lastRead = currentTime;
            }
            
            if(!this->receiveBuffer.empty() && currentTime - this->lastRead > this->readTimeout){
                ESP_LOGV(TAG, "raw recieved data: %s", format_hex_pretty(this->receiveBuffer).c_str());
                handle_message(this->receiveBuffer);
                this->receiveBuffer.clear(); // Reset buffer
            }
        }

        void IM150Meter::set_key(byte key[]) {
            memcpy(&this->key[0], &key[0], 16);
        }

        void IM150Meter::set_sensors(
            sensor::Sensor *active_energy_pos,
            sensor::Sensor *active_energy_neg,
            sensor::Sensor *reactive_energy_pos,
            sensor::Sensor *reactive_energy_neg,
            sensor::Sensor *active_power_pos,
            sensor::Sensor *active_power_neg,
            sensor::Sensor *reactive_power_pos,
            sensor::Sensor *reactive_power_neg
        ) {
            this->active_energy_pos   = active_energy_pos  ;
            this->active_energy_neg   = active_energy_neg  ;
            this->reactive_energy_pos = reactive_energy_pos;
            this->reactive_energy_neg = reactive_energy_neg;
            this->active_power_pos    = active_power_pos   ;
            this->active_power_neg    = active_power_neg   ;
            this->reactive_power_pos  = reactive_power_pos ;
            this->reactive_power_neg  = reactive_power_neg ;
        };

        int IM150Meter::bytes_to_int(byte bytes[], int left, int len) {
            int result = 0;
            for (unsigned int i = left; i < left+len; i++) {
                result = result * 256 + bytes[i];
            }
            return result;
        }

        void IM150Meter::handle_message(std::vector<byte> msg) {
            if(msg.size() != 125) {
                ESP_LOGD(TAG, "wrong msg length: %i, expected 125", msg.size());
                return;
            }
            if(msg[0] != 0x7e) {
                ESP_LOGD(TAG, "wrong opening byte: %02x, expected 7e", msg[0]);
                return;
            }
            if(msg[124] != 0x7e) {
                ESP_LOGD(TAG, "wrong closing byte: %02x, expected 7e", msg[124]);
                return;
            }

            // CRC Check
            int crc = this->CRC16.x25(msg.data() + 1, 121);
            int expected_crc = msg[123] * 256 + msg[122];
            if(crc != expected_crc) {
                ESP_LOGD(TAG, "crc mismatch: calculated %04x, expected %04x", crc, expected_crc);
                return;
            }
            
            // Decrypt
            byte message[92] = {0};
            memcpy(message, msg.data() + 30, 92);
            byte nonce[16] = {0};
            memcpy(nonce, msg.data() + 16, 8);
            memcpy(nonce + 8, msg.data() + 26, 4);
            nonce[15] = 0x02;
            this->ctraes128.setKey(this->key, 16);
            this->ctraes128.setIV(nonce, 16);
            this->ctraes128.decrypt(message, message, 92);
            
            ESP_LOGV(TAG, "decrypted data: %s", format_hex_pretty(std::vector<uint8_t>(message, message+92)).c_str());

            float active_energy_pos    = bytes_to_int(message, 53+5*0, 4)/1000.0;
            float active_energy_neg    = bytes_to_int(message, 53+5*1, 4)/1000.0;
            float reactive_energy_pos  = bytes_to_int(message, 53+5*2, 4)/1000.0;
            float reactive_energy_neg  = bytes_to_int(message, 53+5*3, 4)/1000.0;
            float active_power_pos     = bytes_to_int(message, 53+5*4, 4);
            float active_power_neg     = bytes_to_int(message, 53+5*5, 4);
            float reactive_power_pos   = bytes_to_int(message, 53+5*6, 4);
            float reactive_power_neg   = bytes_to_int(message, 53+5*7, 4);

            if(this->active_energy_pos != NULL && this->active_energy_pos->state != active_energy_pos)
                this->active_energy_pos->publish_state(active_energy_pos);
            if(this->active_energy_neg != NULL && this->active_energy_neg->state != active_energy_neg)
                this->active_energy_neg->publish_state(active_energy_neg);
            if(this->reactive_energy_pos != NULL && this->reactive_energy_pos->state != reactive_energy_pos)
                this->reactive_energy_pos->publish_state(reactive_energy_pos);
            if(this->reactive_energy_neg != NULL && this->reactive_energy_neg->state != reactive_energy_neg)
                this->reactive_energy_neg->publish_state(reactive_energy_neg);
            if(this->active_power_pos != NULL && this->active_power_pos->state != active_power_pos)
                this->active_power_pos->publish_state(active_power_pos);
            if(this->active_power_neg != NULL && this->active_power_neg->state != active_power_neg)
                this->active_power_neg->publish_state(active_power_neg);
            if(this->reactive_power_pos != NULL && this->reactive_power_pos->state != reactive_power_pos)
                this->reactive_power_pos->publish_state(reactive_power_pos);
            if(this->reactive_power_neg != NULL && this->reactive_power_neg->state != reactive_power_neg)
                this->reactive_power_neg->publish_state(reactive_power_neg);
        }

    }
}