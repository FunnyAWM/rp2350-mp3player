#ifndef PLAYER_TF16P_H
#define PLAYER_TF16P_H
#include "pico/stdlib.h"
#include "hardware/uart.h"
#define VOLUME 0x06
#define PLAY 0x03
#define RESET 0x0C
#define NEXT 0x01
#define PREVIOUS 0x02
#define PAUSE 0x0E
#define RESUME 0x0D
#define STOP 0x16
#define STAT 0x42
#define DEVICE 0x09
#define INIT 0x3F

enum class DeviceType {
    UDISK, TFCARD, FLASH
};

enum class ErrorType {
    NO_DEVICE, CHECKSUM_ERR, OUTBOUND, NOTFOUND
};


class PlayerTF16P {
    uint8_t command[10];
    uint8_t received[10]{};
    uint8_t UART_TX_PIN;
    uint8_t UART_RX_PIN;
    uart_inst_t* UART_NUMBER;
    DeviceType device;
    bool ready{};
    bool playing{};

    void sendCommand() {
        command[0] = 0x7E;
        command[1] = 0xFF;
        command[2] = 0x06;
        uint16_t checksum = 0;
        for (int i = 1; i < 7; i++) {
            checksum += command[i];
        }
        checksum = ~checksum + 1;
        const uint8_t lowByte = checksum & 0x00FF;
        const uint8_t highByte = (checksum & 0xFF00) >> 8;
        command[7] = highByte;
        command[8] = lowByte;
        command[9] = 0xEF;
        uart_write_blocking(uart1, command, 10);
        sleep_ms(200);
    }

    void setPlayState(const bool play) {
        playing = play;
    }

public:
    PlayerTF16P(const uint8_t txPin, const uint8_t rxPin, uart_inst_t* uart)
        : command(), UART_TX_PIN(txPin), UART_RX_PIN(rxPin), UART_NUMBER(uart), device() {
    }

    ~PlayerTF16P() = default;

    [[nodiscard]] bool isReady() const {
        return ready;
    }

    [[nodiscard]] bool isPlaying() const {
        return playing;
    }

    void begin(const DeviceType type) {
        sleep_ms(2000);
        uart_init(UART_NUMBER, 9600);
        gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
        command[3] = DEVICE;
        command[4] = 0x00;
        command[5] = 0x01;
        switch (type) {
        case DeviceType::UDISK:
            command[6] = 0x01;
            break;
        case DeviceType::TFCARD:
            command[6] = 0x02;
            break;
        case DeviceType::FLASH:
            command[6] = 0x04;
            break;
        }
        sendCommand();
        uart_read_blocking(UART_NUMBER, received, 10);
        if (received[3] == 0x40) {
            //TODO Error Processing
            return;
        }
        ready = true;
    }

    void setVolume(uint8_t volume) {
        if (volume > 30) {
            volume = 30;
        }
        if (volume < 0) {
            volume = 0;
        }
        command[3] = VOLUME;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = volume;
        sendCommand();
    }

    void playTrack(const uint16_t track) {
        command[3] = PLAY;
        command[4] = 0x00;
        command[5] = (track & 0xFF00) >> 8;
        command[6] = track & 0x00FF;
        sendCommand();
        playing = true;
    }

    void stop() {
        command[3] = STOP;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        sendCommand();
    }

    void pause() {
        command[3] = PAUSE;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        sendCommand();
    }

    void resume() {
        command[3] = RESUME;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        sendCommand();
    }

    void getStats() {
        command[3] = STAT;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        sendCommand();
    }

    uint16_t getTrackTotal() {
        switch (device) {
        case DeviceType::UDISK:
            command[3] = 0x47;
            break;
        case DeviceType::TFCARD:
            command[3] = 0x48;
            break;
        default:
            return 0;
        }
        command[4] = 0x01;
        command[5] = 0x00;
        command[6] = 0x00;
        sendCommand();
        uart_read_blocking(UART_NUMBER, received, 10);
        return received[5] << 2 + received[6];
    }
};

#endif // PLAYER_TF16P_H
