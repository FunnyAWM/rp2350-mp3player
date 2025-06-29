/*

 * 这个头文件是oled库的 [硬件层] 实现文件，移植的时候需要更改这个文件的内容

*/
#include "OLED_Driver.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include <stdint.h>


uint8_t OLED_DisplayBuf[64 / 8][128];

bool OLED_ColorMode = true;

#define SDA 2
#define SCL 3
#define I2C_HW i2c1

/**
  * 函    数：OLED写SCL高低电平
  * 参    数：要写入SCL的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写SCL时，此函数会被调用
  *           用户需要根据参数传入的值，将SCL置为高电平或者低电平
  *           当参数传入0时，置SCL为低电平，当参数传入1时，置SCL为高电平
  */
void OLED_W_SCL(const uint8_t BitValue) {
    /*根据BitValue的值，将SCL置高电平或者低电平*/
    // GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)BitValue);
    gpio_put(SCL, BitValue);
    /*如果单片机速度过快，可在此添加适量延时，以避免超出I2C通信的最大速度*/
    //...
}

/**
  * 函    数：OLED写SDA高低电平
  * 参    数：要写入SDA的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写SDA时，此函数会被调用
  *           用户需要根据参数传入的值，将SDA置为高电平或者低电平
  *           当参数传入0时，置SDA为低电平，当参数传入1时，置SDA为高电平
  */
void OLED_W_SDA(const uint8_t BitValue) {
    /*根据BitValue的值，将SDA置高电平或者低电平*/
    // GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)BitValue);
    gpio_put(SDA, BitValue);
    /*如果单片机速度过快，可在此添加适量延时，以避免超出I2C通信的最大速度*/
    //...
}

/*通信协议*********************/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void OLED_I2C_Start(void) {
    OLED_W_SDA(1); //释放SDA，确保SDA为高电平
    OLED_W_SCL(1); //释放SCL，确保SCL为高电平
    OLED_W_SDA(0); //在SCL高电平期间，拉低SDA，产生起始信号
    OLED_W_SCL(0); //起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void OLED_I2C_Stop(void) {
    OLED_W_SDA(0); //拉低SDA，确保SDA为低电平
    OLED_W_SCL(1); //释放SCL，使SCL呈现高电平
    OLED_W_SDA(1); //在SCL高电平期间，释放SDA，产生终止信号
}


/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void OLED_I2C_SendByte(const uint8_t Byte) {
    /*循环8次，主机依次发送数据的每一位*/
    for (uint8_t i = 0; i < 8; i++) {
        /*使用掩码的方式取出Byte的指定一位数据并写入到SDA线*/
        /*两个!的作用是，让所有非零的值变为1*/
        OLED_W_SDA(!!(Byte & 0x80 >> i));
        OLED_W_SCL(1); //释放SCL，从机在SCL高电平期间读取SDA
        OLED_W_SCL(0); //拉低SCL，主机开始发送下一位数据
    }

    OLED_W_SCL(1); //额外的一个时钟，不处理应答信号
    OLED_W_SCL(0);
}

/**
  * 函    数：OLED写命令
  * 参    数：Command 要写入的命令值，范围：0x00~0xFF
  * 返 回 值：无
  */
void OLED_WriteCommand(const uint8_t Command) {
    OLED_I2C_Start(); //I2C起始
    OLED_I2C_SendByte(0x78); //发送OLED的I2C从机地址
    OLED_I2C_SendByte(0x00); //控制字节，给0x00，表示即将写命令
    OLED_I2C_SendByte(Command); //写入指定的命令
    OLED_I2C_Stop(); //I2C终止
}

/**
 * @brief 设置显示模式
 * @param colormode true: 黑色模式，false: 白色模式
 * @return 无
 */
void OLED_SetColorMode(const bool colormode) {
    OLED_ColorMode = colormode;
}

/**
  * 函    数：OLED写数据
  * 参    数：Data 要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  * 返 回 值：无
  */
void OLED_WriteData(const uint8_t* Data, const uint8_t Count) {
    OLED_I2C_Start(); //I2C起始
    OLED_I2C_SendByte(0x78); //发送OLED的I2C从机地址
    OLED_I2C_SendByte(0x40); //控制字节，给0x40，表示即将写数据
    /*循环Count次，进行连续的数据写入*/
    for (uint8_t i = 0; i < Count; i++) {
        if (OLED_ColorMode) {
            OLED_I2C_SendByte(Data[i]); //依次发送Data的每一个数据
        } else {
            OLED_I2C_SendByte(~Data[i]); //依次发送Data的每一个数据
        }
    }
    OLED_I2C_Stop(); //I2C终止
}

/*********************通信协议*/


/**
  * 函    数：OLED引脚初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：当上层函数需要初始化时，此函数会被调用
  *           用户需要将SCL和SDA引脚初始化为开漏模式，并释放引脚
  */
void OLED_GPIO_Init(void) {
    /*在初始化前，加入适量延时，待OLED供电稳定*/

    i2c_init(I2C_HW, 400 * 1000);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    /*将SCL和SDA引脚初始化为开漏模式*/
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    /*释放SCL和SDA*/
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * 函    数：OLED设置显示光标位置
  * 参    数：Page 指定光标所在的页，范围：0~7
  * 参    数：X 指定光标所在的X轴坐标，范围：0~127
  * 返 回 值：无
  * 说    明：OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
  */
void OLED_SetCursor(const uint8_t Page, const uint8_t X) {
    /*如果使用此程序驱动1.3寸的OLED显示屏，则需要解除此注释*/
    /*因为1.3寸的OLED驱动芯片（SH1106）有132列*/
    /*屏幕的起始列接在了第2列，而不是第0列*/
    /*所以需要将X加2，才能正常显示*/
    //	X += 2;

    /*通过指令设置页地址和列地址*/
    OLED_WriteCommand(0xB0 | Page); //设置页位置
    OLED_WriteCommand(0x10 | (X & 0xF0) >> 4); //设置X位置高4位
    OLED_WriteCommand(0x00 | X & 0x0F); //设置X位置低4位
}

void OLED_Clear(void);
/**
  * 函    数：OLED初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用前，需要调用此初始化函数
  */
void OLED_Init(void) {
    OLED_GPIO_Init();

    OLED_WriteCommand(0xAE); // 打开显示

    OLED_WriteCommand(0xD5); OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);

    OLED_WriteCommand(0xA1); // Segment remap
    OLED_WriteCommand(0xC8); // COM scan direction

    OLED_WriteCommand(0xDA); OLED_WriteCommand(0x12); // COM pins config
    OLED_WriteCommand(0x81); OLED_WriteCommand(0x7F); // 对比度中等亮度
    OLED_WriteCommand(0xD9); OLED_WriteCommand(0xF1); // 预充电周期
    OLED_WriteCommand(0xDB); OLED_WriteCommand(0x40); // 更适合 SSD1315

    OLED_WriteCommand(0xA4); // 全屏显示跟随内存
    OLED_WriteCommand(0xA6); // 正常显示

    // ⚠️ 不再使用充电泵命令：SSD1315 可能无效或导致无显示
    // OLED_WriteCommand(0x8D); OLED_WriteCommand(0x14); // ⚠️ 建议注释掉

    OLED_WriteCommand(0xAF); // 开启显示

    OLED_Clear();
    OLED_Update();
}


/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 参    数：无
  * 返 回 值：无
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Update(void) {
    /*遍历每一页*/
    for (uint8_t j = 0; j < 8; j++) {
        /*设置光标位置为每一页的第一列*/
        OLED_SetCursor(j, 0);
        /*连续写入128个数据，将显存数组的数据写入到OLED硬件*/
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
}

/**
  * 函    数：OLED设置亮度
  * 参    数：Brightness ，0-255，不同显示芯片效果可能不相同。
  * 返 回 值：无
  * 说    明：不要设置过大或者过小。
  */
void OLED_Brightness(int16_t Brightness) {
    if (Brightness > 255) {
        Brightness = 255;
    }
    if (Brightness < 0) {
        Brightness = 0;
    }
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(Brightness);
}

/**
  * 函    数：将OLED显存数组部分更新到OLED屏幕
  * 参    数：X 指定区域左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定区域左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：Width 指定区域的宽度，范围：0~OLED_WIDTH
  * 参    数：Height 指定区域的高度，范围：0~OLED_HEIGHT
  * 返 回 值：无
  * 说    明：此函数会至少更新参数指定的区域
  *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_UpdateArea(const uint8_t X, const uint8_t Y, uint8_t Width, uint8_t Height) {
    /*参数检查，保证指定区域不会超出屏幕范围*/
    if (X > 128 - 1) { return; }
    if (Y > 64 - 1) { return; }
    if (X + Width > 128) { Width = 128 - X; }
    if (Y + Height > 64) { Height = 64 - Y; }

    /*遍历指定区域涉及的相关页*/
    /*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8并向上取整*/
    for (uint8_t j = Y / 8; j < (Y + Height - 1) / 8 + 1; j++) {
        /*设置光标位置为相关页的指定列*/
        OLED_SetCursor(j, X);
        /*连续写入Width个数据，将显存数组的数据写入到OLED硬件*/
        OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
    }
}
