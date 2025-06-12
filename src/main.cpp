#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "PlayerTF16P.h"

PlayerTF16P player(4, 5, uart1);

SemaphoreHandle_t playerOperation;


[[noreturn]] void startPlayer() {
    constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);
    player.begin(DeviceType::TFCARD);
    xSemaphoreTake(playerOperation, portMAX_DELAY);
    while (true) {}
}

void startDisplay() {

}

[[noreturn]] int main() {
    stdio_init_all();
    TaskHandle_t player_task;
    TaskHandle_t display_task;
    xSemaphoreCreateBinary(playerOperation);
    xTaskCreate(reinterpret_cast<TaskFunction_t>(&startDisplay), "START_DISPLAY", 256, nullptr, 1, &display_task);
    xTaskCreate(reinterpret_cast<TaskFunction_t>(&startPlayer), "START_PLAYER", 256, nullptr, 1, &player_task);
    vTaskCoreAffinitySet(display_task, 1 << 0);
    vTaskCoreAffinitySet(player_task, 1 << 1);
    vTaskStartScheduler();
    while (true) {
    }
}
