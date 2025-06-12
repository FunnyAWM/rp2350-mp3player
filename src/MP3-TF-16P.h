#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#define VOLUME 0x06
#define PLAY 0x03
#define RESET 0x0C
#define NEXT 0x01
#define PREVIOUS 0x02
#define PAUSE 0x0E
#define RESUME 0x0D
#define STOP 0x16
struct MP3PlayerControl {
    uint8_t command[10];
    uint8_t UART_TX_PIN;
    uint8_t UART_RX_PIN;
};

void playerBegin(struct MP3PlayerControl *init) {
    sleep_ms(2000);
    uart_init(uart1, 9600);
    gpio_set_function(init->UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(init->UART_RX_PIN, GPIO_FUNC_UART);
}

void playerSendCommand(struct MP3PlayerControl* control) {
    control->command[0] = 0x7E;
    control->command[1] = 0xFF;
    control->command[2] = 0x06;
    uint16_t checksum = 0;
    for (int i = 1; i < 7; i++) {
        checksum += control->command[i];
    }
    checksum = ~checksum + 1;
    uint8_t lowByte, highByte;
    lowByte = checksum & 0x00FF;
    highByte = (checksum & 0xFF00) >> 8;
    control->command[7] = highByte;
    control->command[8] = lowByte;
    control->command[9] = 0xEF;
    uart_write_blocking(uart1, control->command, 10);
    sleep_ms(200);
}

void playerSetVolume(struct MP3PlayerControl *control, uint8_t volume) {
    if (volume > 30) {
        volume = 30;
    }
    if (volume < 0) {
        volume = 0;
    }
    control->command[3] = VOLUME;
    control->command[4] = 0x00;
    control->command[5] = 0x00;
    control->command[6] = volume;
    playerSendCommand(control);
}

void playerPlayTrack(struct MP3PlayerControl *control, uint16_t track) {
    control->command[3] = PLAY;
    control->command[4] = 0x00;
    control->command[5] = (track & 0xFF00) >> 8;
    control->command[6] = track & 0x00FF;
    playerSendCommand(control);
}

void stopPlayer(struct MP3PlayerControl *control) {
    control->command[3] = STOP;
    control->command[4] = 0x00;
    control->command[5] = 0x00;
    control->command[6] = 0x00;
    playerSendCommand(control);
}

void pausePlayer(struct MP3PlayerControl *control) {
    control->command[3] = PAUSE;
    control->command[4] = 0x00;
    control->command[5] = 0x00;
    control->command[6] = 0x00;
    playerSendCommand(control);
}

void resumePlayer(struct MP3PlayerControl *control) {
    control->command[3] = RESUME;
    control->command[4] = 0x00;
    control->command[5] = 0x00;
    control->command[6] = 0x00;
    playerSendCommand(control);
}