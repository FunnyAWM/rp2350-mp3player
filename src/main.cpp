#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "PlayerTF16P.h"
#include "boards/pico.h"
#include "../lib/OLED-UI/OLED_UI.h"
#include "../lib/OLED-UI/OLED_UI_MenuData.h"

// 播放器全局对象
PlayerTF16P player(4, 5, uart1);

// 同步机制
SemaphoreHandle_t playerMutex;  // 改为互斥锁
QueueHandle_t playerCommandQueue;  // 命令队列

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

[[noreturn]] void playerTask(void *pvParameters) {
    constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // 初始化播放器
    player.begin(DeviceType::TFCARD);
    
    PlayerCommand cmd;
    while(true) {
        // 等待控制命令（带超时）
        if(xQueueReceive(playerCommandQueue, &cmd, pdMS_TO_TICKS(10))) {
            // 处理播放控制命令
            xSemaphoreTake(playerMutex, portMAX_DELAY);
            switch(cmd) {
                // TODO
                case CMD_PLAY: player.playTrack(1); break;
                case CMD_PAUSE: player.pause(); break;
            default: ;

                // ... 其他命令处理
            }
            xSemaphoreGive(playerMutex);
        }
        
        // 音频解码和播放（受互斥锁保护）
        xSemaphoreTake(playerMutex, portMAX_DELAY);
        // 假设有音频处理循环函数
        xSemaphoreGive(playerMutex);
        
        // 适当延时让出CPU
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

[[noreturn]] void uiTask(void *pvParameters) {
    // 初始化UI系统
    OLED_UI_Init(&MainMenuPage);
    
    while(true) {
        OLED_UI_MainLoop();
        
        // 检测用户输入并发送播放命令
        // TODO
        // if(/* 用户按下播放按钮 */) {
        //     PlayerCommand cmd = CMD_PLAY;
        //     xQueueSend(playerCommandQueue, &cmd, 0);
        // }
        // 其他按钮处理...
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void uiTimerCallback(TimerHandle_t xTimer) {
    OLED_UI_InterruptHandler();
}

[[noreturn]] int main() {
    stdio_init_all();
    
    // 创建同步机制
    playerMutex = xSemaphoreCreateMutex();
    playerCommandQueue = xQueueCreate(10, sizeof(PlayerCommand));
    
    // 调整定时器守护任务优先级
    vTaskPrioritySet(xTimerGetTimerDaemonTaskHandle(), 3);
    
    // 创建任务
    TaskHandle_t playerHandle, uiHandle;
    xTaskCreate(playerTask, "PLAYER", 512, nullptr, 3, &playerHandle);  // 更高优先级
    xTaskCreate(uiTask, "UI", 512, nullptr, 2, &uiHandle);  // 中等优先级
    
    // 创建定时器
    TimerHandle_t uiTimer = xTimerCreate("UI_Timer", pdMS_TO_TICKS(20), 
                                       pdTRUE, nullptr, uiTimerCallback);
    xTimerStart(uiTimer, 0);
    
    // 设置核心亲和性（建议但不强制）
    vTaskCoreAffinitySet(uiHandle, 1 << 0);
    vTaskCoreAffinitySet(playerHandle, 1 << 1);
    
    vTaskStartScheduler();
    
    while(true);  // 不应执行到这里
}