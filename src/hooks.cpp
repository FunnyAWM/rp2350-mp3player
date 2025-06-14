#include "hooks.h"
#include "FreeRTOS.h"


#ifdef __cplusplus
extern "C" {
#endif
// 栈溢出检测钩子函数
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    (void)xTask;
    (void)pcTaskName;

    // 这里添加栈溢出处理逻辑
    // 例如：记录错误、重启系统或进入安全状态
    for (;;); // 死循环，实际项目中应根据需求处理
}

// 空闲任务内存分配
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint16_t* puxIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

// 定时器任务内存分配
#if configUSE_TIMERS == 1
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint16_t* pulTimerTaskStackSize) {
    // 静态分配定时器任务的TCB和栈内存
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif // configUSE_TIMERS

// 被动空闲任务内存分配
void vApplicationGetPassiveIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                          StackType_t** ppxIdleTaskStackBuffer,
                                          uint16_t* puxIdleTaskStackSize, // 改为uint16_t
                                          BaseType_t xPassiveIdleTaskIndex) // 添加核心ID参数
{
    static StaticTask_t xPassiveIdleTaskTCB;
    static StackType_t uxPassiveIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xPassiveIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxPassiveIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#ifdef __cplusplus
}
#endif
