#ifndef PUBLIC_H
#define PUBLIC_H
#include "hardware/gpio.h"
#include "pico/time.h"

inline void panicBlink(const int times) {
    constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    for (int i = 0; i < times; i++) {
        gpio_put(LED_PIN, true);
        sleep_ms(500);
        gpio_put(LED_PIN, false);
        sleep_ms(500);
    }
}
#endif //PUBLIC_H
