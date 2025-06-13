#include "OLED_UI_Driver.h"

// 编码器相关变量
static volatile int32_t encoder_count = 0;  ///< 编码器原始计数值
static volatile uint8_t last_AB = 0;        ///< 上一次AB相的状态
static volatile bool encoder_enabled = true; ///< 编码器使能标志

// 编码器状态表 (四倍频解码)
// 状态表索引: (last_AB << 2) | current_state
static const int8_t enc_states[] = {
    // 00 -> 00, 01, 10, 11
    0, -1, 1, 0,
    // 01 -> 00, 01, 10, 11
    1, 0, 0, -1,
    // 10 -> 00, 01, 10, 11
    -1, 0, 0, 1,
    // 11 -> 00, 01, 10, 11
    0, 1, -1, 0
};

/**
 * @brief 编码器中断处理函数
 * @param gpio 触发中断的GPIO引脚
 * @param events 触发的事件类型
 *
 * @details 当编码器A相或B相引脚状态变化时调用此函数
 * 使用状态机实现四倍频解码，支持旋转方向检测
 * 包含100μs的软件消抖处理
 */
void encoder_isr(uint gpio, uint32_t events) {
    // 如果编码器被禁用，则直接返回
    if (!encoder_enabled) return;

    // 消抖处理：记录上次中断时间，确保至少间隔100μs
    static uint32_t last_time = 0;
    const uint32_t now = time_us_32();
    if (now - last_time < 100) return;
    last_time = now;

    // 读取当前A相和B相状态
    const uint8_t currentA = gpio_get(ENC_A_PIN);
    const uint8_t currentB = gpio_get(ENC_B_PIN);

    // 组合当前状态 (2位二进制：A高位，B低位)
    const uint8_t current_state = currentA << 1 | currentB;

    // 使用状态表解码旋转方向
    // 索引 = (上一次状态 << 2) | 当前状态
    encoder_count += enc_states[last_AB << 2 | current_state];

    // 更新上一次状态
    last_AB = current_state;
}

/**
 * @brief 定时器中断服务函数的初始化函数，用于产生20ms的定时器中断
 * @param callback 定时器中断回调函数
 * @return 无
 *
 * 【RP2040实现说明】：
 * 使用add_repeating_timer_ms创建20ms周期的重复定时器
 * 定时器中断将调用提供的callback函数
 */
void Timer_Init(void (*callback)(void)) {
    // 创建20ms周期的重复定时器
    add_repeating_timer_ms(20, (repeating_timer_callback_t)callback, NULL, NULL);
}

/**
 * @brief 按键初始化函数，用于初始化按键GPIO
 * @param 无
 * @note GPIO被初始化为上拉输入模式
 * @return 无
 */
void Key_Init(void) {
    // 初始化按键GPIO引脚
    gpio_init(ENTER_PIN);
    gpio_init(BACK_PIN);
    gpio_init(UP_PIN);
    gpio_init(DOWN_PIN);

    // 设置引脚方向为输入
    gpio_set_dir(ENTER_PIN, GPIO_IN);
    gpio_set_dir(BACK_PIN, GPIO_IN);
    gpio_set_dir(UP_PIN, GPIO_IN);
    gpio_set_dir(DOWN_PIN, GPIO_IN);

    // 启用内部上拉电阻
    gpio_pull_up(ENTER_PIN);
    gpio_pull_up(BACK_PIN);
    gpio_pull_up(UP_PIN);
    gpio_pull_up(DOWN_PIN);
}

/**
 * @brief 编码器初始化函数，配置编码器引脚和中断
 * @param 无
 * @return 无
 */
void Encoder_Init(void) {
    // 初始化编码器A相和B相引脚
    gpio_init(ENC_A_PIN);
    gpio_init(ENC_B_PIN);

    // 设置引脚方向为输入
    gpio_set_dir(ENC_A_PIN, GPIO_IN);
    gpio_set_dir(ENC_B_PIN, GPIO_IN);

    // 启用内部上拉电阻
    gpio_pull_up(ENC_A_PIN);
    gpio_pull_up(ENC_B_PIN);

    // 保存初始状态
    last_AB = (gpio_get(ENC_A_PIN) << 1) | gpio_get(ENC_B_PIN);

    // 设置双边沿触发中断（上升沿和下降沿都触发）
    gpio_set_irq_enabled_with_callback(ENC_A_PIN,
                                      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                      true,
                                      &encoder_isr);
    gpio_set_irq_enabled(ENC_B_PIN,
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                         true);
}

/**
 * @brief 编码器使能函数
 * @param 无
 * @return 无
 */
void Encoder_Enable(void) {
    encoder_enabled = true;
}

/**
 * @brief 编码器失能函数
 * @param 无
 * @return 无
 */
void Encoder_Disable(void) {
    encoder_enabled = false;
}

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
int16_t Encoder_Get(void) {
    // 静态变量，用于在函数调用间保存未被4整除的余数
    static int32_t remainder = 0;

    // 原子操作：禁用中断期间读取并重置计数器
    irq_set_enabled(IO_IRQ_BANK0, false);
    int32_t count = encoder_count;
    encoder_count = 0;
    irq_set_enabled(IO_IRQ_BANK0, true);

    // 四倍频解码处理
    count += remainder;             // 加上前一次的余数
    int16_t result = count / 4;     // 计算整步数
    remainder = count % 4;          // 保存新的余数

    return result;
}