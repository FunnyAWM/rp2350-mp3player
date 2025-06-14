#ifndef HOOKS_H
#define HOOKS_H

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

// 定时器任务内存分配（条件编译）
#if configUSE_TIMERS == 1
void vApplicationGetTimerTaskMemory(StaticTask_t**, StackType_t**, uint16_t*);
#endif

// 被动空闲任务内存分配（条件编译）
#if configUSE_PASSIVE_IDLE_HOOK == 1
    void vApplicationGetPassiveIdleTaskMemory(StaticTask_t**, StackType_t**, uint32_t*, BaseType_t);
#endif

#ifdef __cplusplus
}
#endif

#endif
