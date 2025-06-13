#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H

/**
 * 【文件说明】：[硬件抽象层]
 * 此文件包含按键与编码器的驱动程序，如果需要移植此项目，请根据实际情况修改相关代码。
 * 当你确保oled屏幕能够正常点亮，并且能够正确地运行基础功能时（如显示字符串等），就可以开始移植
 * 有关按键与编码器等的驱动程序了。
 *
 * 【移植说明】：
 * 此版本已适配Raspberry Pi Pico (RP2040)平台
 * 使用Pico SDK的GPIO、定时器和中断系统实现
 */

#include "pico/stdlib.h"

// 根据实际硬件连接修改这些引脚定义
#define ENTER_PIN 12  // 确认按键引脚
#define BACK_PIN  13  // 取消/返回按键引脚
#define UP_PIN    14  // 上按键引脚
#define DOWN_PIN  15  // 下按键引脚
#define ENC_A_PIN 16  // 编码器A相引脚
#define ENC_B_PIN 17  // 编码器B相引脚

// 获取确认，取消，上，下按键状态的函数
// 【Q：为什么使用宏定义而不是函数？A：因为这样可以提高效率，减少代码量】
// 注意：按键按下时返回0（低电平），释放时返回1（高电平）
#define Key_GetEnterStatus()    (!gpio_get(ENTER_PIN))  // 获取确认按键状态
#define Key_GetBackStatus()     (!gpio_get(BACK_PIN))   // 获取返回按键状态
#define Key_GetUpStatus()       (!gpio_get(UP_PIN))     // 获取上按键状态
#define Key_GetDownStatus()     (!gpio_get(DOWN_PIN))   // 获取下按键状态

/**
 * @brief 定时器中断服务函数的初始化函数，用于产生20ms的定时器中断
 * @param callback 定时器中断回调函数
 * @return 无
 *
 * 【RP2040实现说明】：
 * 使用Pico SDK的重复定时器实现，每20ms调用一次回调函数
 */
void Timer_Init(void (*callback)(void));

/**
 * @brief 按键初始化函数，用于初始化按键GPIO
 * @param 无
 * @note GPIO被初始化为上拉输入模式
 * @return 无
 */
void Key_Init(void);

/**
 * @brief 编码器初始化函数，配置编码器引脚和中断
 * @param 无
 * @return 无
 *
 * 【RP2040实现说明】：
 * 使用软件解码实现编码器功能，通过GPIO中断检测旋转
 */
void Encoder_Init(void);

/**
 * @brief 编码器使能函数
 * @param 无
 * @return 无
 */
void Encoder_Enable(void);

/**
 * @brief 编码器失能函数
 * @param 无
 * @return 无
 */
void Encoder_Disable(void);

/**
 * @brief 获取编码器的增量计数值（四倍频解码）
 *
 * @details 该函数通过读取编码器计数值，对编码器信号进行四倍频解码处理。
 *          使用静态变量累积计数，并通过除法和取模运算去除多余的增量部分，
 *          确保返回精确的增量值。主要用于电机控制、位置检测等应用场景。
 *
 * @note   函数内部会自动清零计数器，确保下次读取的准确性
 *
 * @return int16_t 返回解码后的编码器增量值
 */
int16_t Encoder_Get(void);

// 延时函数
#define Delay_us(xus) busy_wait_us(xus)  ///< 微秒级延时
#define Delay_ms(xms) busy_wait_ms(xms)  ///< 毫秒级延时
#define Delay_s(xs)   busy_wait_ms((xs)*1000)  ///< 秒级延时

#endif