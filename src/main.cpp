#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pico/stdlib.h"
#include "PlayerTF16P.h"
#include "../lib/OLED-UI/OLED_UI.h"
#include "../lib/OLED-UI/OLED_UI_MenuData.h"
#include "public.h"
#include "stream_decoder.h"

// extern "C" void vLaunch(void);
// 播放器全局对象
PlayerTF16P player(4, 5, uart1);

// 同步机制
SemaphoreHandle_t playerMutex; // 互斥锁
QueueHandle_t playerCommandQueue; // 命令队列

// 播放器控制命令枚举
enum PlayerCommand {
    CMD_PLAY,
    CMD_PAUSE,
    CMD_STOP,
    CMD_NEXT,
    CMD_PREV,
    CMD_VOL_UP,
    CMD_VOL_DOWN
};

void openLED(void* pvParameters) {
    constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true); // 打开LED指示灯
    vTaskDelete(nullptr); // 任务完成后删除自身
}

[[noreturn]] void playerTask(void* pvParameters) {
    // 初始化播放器
    player.begin(DeviceType::TFCARD);

    PlayerCommand cmd;
    while (true) {
        // 等待控制命令（带超时）
        if (xQueueReceive(playerCommandQueue, &cmd, pdMS_TO_TICKS(10))) {
            // 带超时的互斥锁获取
            if (xSemaphoreTake(playerMutex, pdMS_TO_TICKS(20))) {
                switch (cmd) {
                case CMD_PLAY:
                    player.playTrack(1);
                    break;
                case CMD_PAUSE:
                    player.pause();
                    break;
                case CMD_STOP:
                    player.stop();
                    break;
                case CMD_NEXT:
                    player.playTrack(player.getTrack() + 1);
                    break;
                case CMD_PREV:
                    player.playTrack(player.getTrack() - 1);
                    break;
                case CMD_VOL_UP:
                    player.setVolume(player.getVolume() + 1);
                    break;
                case CMD_VOL_DOWN:
                    player.setVolume(player.getVolume() - 1);
                    break;
                }
                xSemaphoreGive(playerMutex);
            }
        }

        // 带超时的互斥锁获取用于音频处理
        if (xSemaphoreTake(playerMutex, pdMS_TO_TICKS(10))) {
            FLAC__StreamDecoder decoder;

            // 音频解码和播放处理
            // 例如: player.audioProcess();
            xSemaphoreGive(playerMutex);
        }

        // 让出CPU
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

[[noreturn]] void uiTask(void* pvParameters) {
    // 初始化UI系统
    OLED_UI_Init(&MainMenuPage);

    while (true) {
        OLED_UI_MainLoop();

        // 检测用户输入并发送播放命令
        if (Key_GetEnterStatus()) {
            PlayerCommand cmd = CMD_PLAY;
            xQueueSend(playerCommandQueue, &cmd, 0);
        } else if (Key_GetBackStatus()) {
            PlayerCommand cmd = CMD_PAUSE;
            xQueueSend(playerCommandQueue, &cmd, 0);
        }
        // 其他按钮处理...

        // 栈溢出检测
        // ReSharper disable once CppLocalVariableMayBeConst
        // ReSharper disable once CppTooWideScopeInitStatement
        UBaseType_t wm = uxTaskGetStackHighWaterMark(nullptr);
        if (wm < 10) {
            // 处理栈溢出
            panicBlink(7);
            for (;;) {
                // 错误处理
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void uiTimerCallback(TimerHandle_t xTimer) {
    OLED_UI_InterruptHandler();
}

// 启动任务用于初始化调度器后的操作
void startupTask(void* pvParameters) {
    // 创建任务
    TaskHandle_t playerHandle, uiHandle;
    TaskHandle_t ledHandle;
    BaseType_t ret[3];
    ret[0] = xTaskCreate(playerTask, "PLAYER", 4096, nullptr, 2, &playerHandle);
    ret[1] = xTaskCreate(uiTask, "UI", 1536, nullptr, 3, &uiHandle); // 栈增加到1536
    ret[2] = xTaskCreate(openLED, "LED", 256, nullptr, 4, &ledHandle);
    for (const BaseType_t val : ret) {
        if (val != pdPASS) {
            panicBlink(5);
            while (true) {
            }
        }
    }
    // 创建并启动定时器
    // ReSharper disable once CppLocalVariableMayBeConst
    TimerHandle_t uiTimer = xTimerCreate("UI_Timer", pdMS_TO_TICKS(20),
                                         pdTRUE, nullptr, uiTimerCallback);
    ret[0] = xTimerStart(uiTimer, 0);
    vTaskPrioritySet(xTimerGetTimerDaemonTaskHandle(), configMAX_PRIORITIES - 1);
    if (ret[0] == pdFAIL) {
        panicBlink(3);
        while (true) {
        }
    }
    // 启动任务完成后删除自身
    vTaskDelete(nullptr);
}

[[noreturn]] int main() {
    stdio_init_all();

    // 硬件初始化
    gpio_init(PICO_DEFAULT_LED_PIN);
    uart_init(uart1, 9600);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    // 创建同步机制
    playerMutex = xSemaphoreCreateMutex();
    playerCommandQueue = xQueueCreate(10, sizeof(PlayerCommand));
    if (!playerMutex || !playerCommandQueue) {
        // 提示初始化失败，比如点亮LED或打印错误信息
        panicBlink(2);
    }


    // 提高定时器守护任务优先级


    // 创建启动任务
    const BaseType_t ret = xTaskCreate(startupTask, "STARTUP", 1024, nullptr, 5, nullptr);

    if (ret == pdFAIL) {
        panicBlink(5);
        while (true) {
        }
    }
    // 启动调度器
    vTaskStartScheduler();
    panicBlink(4);
    while (true);
}
