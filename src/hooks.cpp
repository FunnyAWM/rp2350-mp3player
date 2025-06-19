#include "hooks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "public.h"
#ifdef __cplusplus
extern "C" {
#endif
void vApplicationStackOverflowHook(TaskHandle_t xTask, const char* pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    for (;;) {
        constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
        gpio_put(LED_PIN, true);
        sleep_ms(500);
        gpio_put(LED_PIN, false);
        sleep_ms(500);
    }
}

void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint16_t* puxIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if configUSE_TIMERS == 1
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint16_t* pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif

// 修改参数类型为 uint16_t*
void vApplicationGetPassiveIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                          StackType_t** ppxIdleTaskStackBuffer,
                                          uint16_t* puxIdleTaskStackSize,
                                          BaseType_t xPassiveIdleTaskIndex)
{
    static StaticTask_t xPassiveIdleTaskTCB;
    static StackType_t uxPassiveIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xPassiveIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxPassiveIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
void vApplicationMallocFailedHook() {
    panicBlink(6); // 或打印信息
    while (true);
}

#ifdef __cplusplus
}
#endif