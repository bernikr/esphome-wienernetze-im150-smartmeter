#include "im150.h"

namespace esphome {
    namespace im150 {
        void IM150::setup() {
            ESP_LOGD(TAG, "IM150 smart meter component v%s started", IM150_VERSION);
        }
        
        void IM150::loop() {
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

        int IM150::bytes_to_int(uint8_t bytes[], int left, int len) {
            int result = 0;
            for (unsigned int i = left; i < left+len; i++) {
                result = result * 256 + bytes[i];
            }
            return result;
        }

        void IM150::handle_message(std::vector<uint8_t> msg) {
            uint8_t datalen = msg.size();

            if(datalen != 125) {
                ESP_LOGD(TAG, "wrong msg length: %i, expected 125. Functunality is untested with this smartmeter.", msg.size());
            }

            if(msg[0] != 0x7e) {
                ESP_LOGD(TAG, "wrong opening byte: %02x, expected 7e", msg[0]);
                return;
            }
            if(msg[datalen-1] != 0x7e) {
                ESP_LOGD(TAG, "wrong closing byte: %02x, expected 7e", msg[124]);
                return;
            }

            // CRC Check
            int crc = this->CRC16.x25(msg.data() + 1, 121);
            int expected_crc = msg[datalen-2] * 256 + msg[datalen-3];
            if(crc != expected_crc) {
                ESP_LOGD(TAG, "crc mismatch: calculated %04x, expected %04x", crc, expected_crc);
                return;
            }
            
            // Decrypt
            uint8_t msglen = datalen - 33;
            uint8_t message[msglen] = {0};
            memcpy(message, msg.data() + 30, msglen);
            uint8_t nonce[16] = {0};
            memcpy(nonce, msg.data() + 16, 8);
            memcpy(nonce + 8, msg.data() + 26, 4);
            nonce[15] = 0x02;
            this->ctraes128.setKey(this->key, 16);
            this->ctraes128.setIV(nonce, 16);
            this->ctraes128.decrypt(message, message, msglen);
            ESP_LOGV(TAG, "decrypted data: %s", format_hex_pretty(std::vector<uint8_t>(message, message+msglen)).c_str());

            uint32_t active_energy_pos_raw    = bytes_to_int(message, msglen-4-5*7, 4);
            uint32_t active_energy_neg_raw    = bytes_to_int(message, msglen-4-5*6, 4);
            uint32_t reactive_energy_pos_raw  = bytes_to_int(message, msglen-4-5*5, 4);
            uint32_t reactive_energy_neg_raw  = bytes_to_int(message, msglen-4-5*4, 4);

            // use modulo 1000kwh for the energy sensors, because esphome sensors are only 32bit floats
            // values larger than that would suffer from precision errors
            // because the sensors are defined as total_increasing, home assistant will still correctly display consuption
            float active_energy_pos    = (active_energy_pos_raw%1000000)/1000.0;
            float active_energy_neg    = (active_energy_neg_raw%1000000)/1000.0;
            float reactive_energy_pos  = (reactive_energy_pos_raw%1000000)/1000.0;
            float reactive_energy_neg  = (reactive_energy_neg_raw%1000000)/1000.0;
            float active_power_pos     = bytes_to_int(message, msglen-4-5*3, 4);
            float active_power_neg     = bytes_to_int(message, msglen-4-5*2, 4);
            float reactive_power_pos   = bytes_to_int(message, msglen-4-5*1, 4);
            float reactive_power_neg   = bytes_to_int(message, msglen-4-5*0, 4);

            if(this->active_energy_pos != nullptr && this->active_energy_pos->state != active_energy_pos)
                this->active_energy_pos->publish_state(active_energy_pos);
            if(this->active_energy_neg != nullptr && this->active_energy_neg->state != active_energy_neg)
                this->active_energy_neg->publish_state(active_energy_neg);
            if(this->reactive_energy_pos != nullptr && this->reactive_energy_pos->state != reactive_energy_pos)
                this->reactive_energy_pos->publish_state(reactive_energy_pos);
            if(this->reactive_energy_neg != nullptr && this->reactive_energy_neg->state != reactive_energy_neg)
                this->reactive_energy_neg->publish_state(reactive_energy_neg);
            if(this->active_power_pos != nullptr && this->active_power_pos->state != active_power_pos)
                this->active_power_pos->publish_state(active_power_pos);
            if(this->active_power_neg != nullptr && this->active_power_neg->state != active_power_neg)
                this->active_power_neg->publish_state(active_power_neg);
            if(this->reactive_power_pos != nullptr && this->reactive_power_pos->state != reactive_power_pos)
                this->reactive_power_pos->publish_state(reactive_power_pos);
            if(this->reactive_power_neg != nullptr && this->reactive_power_neg->state != reactive_power_neg)
                this->reactive_power_neg->publish_state(reactive_power_neg);
            

            char buffer[16];

            if(this->active_energy_pos_raw != nullptr)
                this->active_energy_pos_raw->publish_state(itoa(active_energy_pos_raw, buffer, 10));
            if(this->active_energy_neg_raw != nullptr)
                this->active_energy_neg_raw->publish_state(itoa(active_energy_neg_raw, buffer, 10));
            if(this->reactive_energy_pos_raw != nullptr)
                this->reactive_energy_pos_raw->publish_state(itoa(reactive_energy_pos_raw, buffer, 10));
            if(this->reactive_energy_neg_raw != nullptr)
                this->reactive_energy_neg_raw->publish_state(itoa(reactive_energy_neg_raw, buffer, 10));
        }
    }
}