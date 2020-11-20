#include "iic.h"

#define GPIO_Init()                                               \
    GPIO_InitTypeDef GPIO_InitStruct;                             \
    __HAL_RCC_GPIOA_CLK_ENABLE();                                 \
    HAL_GPIO_WritePin(IIC_SCL_Port, IIC_SCL_Pin, GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(IIC_SDA_Port, IIC_SDA_Pin, GPIO_PIN_RESET); \
    GPIO_InitStruct.Pin = IIC_SCL_Pin | IIC_SDA_Pin;              \
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                   \
    GPIO_InitStruct.Pull = GPIO_PULLUP;                           \
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                 \
    HAL_GPIO_Init(IIC_SCL_Port, &GPIO_InitStruct);

#define IIC_SCL_High() HAL_GPIO_WritePin(IIC_SCL_Port, IIC_SCL_Pin, GPIO_PIN_SET)
#define IIC_SCL_Low() HAL_GPIO_WritePin(IIC_SCL_Port, IIC_SCL_Pin, GPIO_PIN_RESET)
#define IIC_SDA_High() HAL_GPIO_WritePin(IIC_SDA_Port, IIC_SDA_Pin, GPIO_PIN_SET)
#define IIC_SDA_Low() HAL_GPIO_WritePin(IIC_SDA_Port, IIC_SDA_Pin, GPIO_PIN_RESET)
#define IIC_SDA_Read() HAL_GPIO_ReadPin(IIC_SDA_Port, IIC_SDA_Pin)
#define IIC_Delay(x)                     \
    for (uint8_t i = 0; i < x; i++)      \
        for (uint8_t j = 0; j < 32; j++) \
            ;

void IIC_SDA_Read_Mode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = IIC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IIC_SDA_Port, &GPIO_InitStruct);
}

void IIC_SDA_Write_Mode(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = IIC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_WritePin(IIC_SDA_Port, IIC_SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_Init(IIC_SDA_Port, &GPIO_InitStruct);
}

void IIC_Init()
{
    GPIO_Init();
    IIC_SDA_High();
    IIC_SCL_High();
}

void IIC_Start(void)
{
    IIC_SDA_Write_Mode();
    IIC_SDA_High();
    IIC_SCL_High();
    IIC_Delay(2);
    IIC_SDA_Low();
    IIC_Delay(2);
    // 准备接受或者发送数据
    IIC_SCL_Low();
}

void IIC_Stop(void)
{
    IIC_SDA_Write_Mode();
    IIC_SCL_Low();
    IIC_SDA_Low();
    IIC_Delay(2);
    IIC_SCL_High();
    IIC_SDA_High();
    IIC_Delay(2);
}

uint8_t IIC_Waite_Ack(void)
{
    uint8_t ucErrorTime = 0;
    IIC_SDA_Read_Mode();
    IIC_SDA_High();
    IIC_Delay(2);
    IIC_SCL_High();
    IIC_Delay(2);
    while (IIC_SDA_Read())
    {
        ucErrorTime++;
        if (ucErrorTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_Low();
    return 0;
}

void IIC_Ack(void)
{
    IIC_SCL_Low();
    IIC_SDA_Write_Mode();
    IIC_SDA_Low();
    IIC_Delay(2);
    IIC_SCL_High();
    IIC_Delay(2);
    IIC_SCL_Low();
}

void IIC_NoAck()
{
    IIC_SCL_Low();
    IIC_SDA_Write_Mode();
    IIC_SDA_High();
    IIC_Delay(2);
    IIC_SCL_High();
    IIC_Delay(2);
    IIC_SCL_Low();
}

void IIC_Write_Data(uint8_t data)
{
    IIC_SDA_Write_Mode();
    IIC_SCL_Low();
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((data & 0x80) >> 7)
        {
            IIC_SDA_High();
        }
        else
        {
            IIC_SDA_Low();
        }
        data <<= 1;
        IIC_SCL_High();
        IIC_Delay(2);
        IIC_SCL_Low();
        IIC_Delay(2);
    }
}

uint8_t IIC_Read_Data(uint8_t ack_enable)
{
    uint8_t receive = 0;
    IIC_SDA_Read_Mode();

    for (uint8_t i = 0; i < 8; i++)
    {
        IIC_SCL_Low();
        IIC_Delay(2);
        IIC_SCL_High();
        receive <<= 1;
        if (IIC_SDA_Read())
            receive++;
        IIC_Delay(2);
    }
    if (ack_enable)
    {
        IIC_Ack();
    }
    else
    {
        IIC_NoAck();
    }
    return receive;
}

uint8_t IIC_Write_Buffer(uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t len)
{
    IIC_Start();
    IIC_Write_Data((address << 1) | 0);
    if (IIC_Waite_Ack())
    {
        IIC_Stop();
        return 1;
    }
    IIC_Write_Data(reg);
    IIC_Waite_Ack();
    // 写完所有数据再等待Ack应答
    for (uint8_t i = 0; i < len; i++)
    {
        IIC_Write_Data(buffer[i]);
        if (IIC_Waite_Ack())
        {
            IIC_Stop();
            return 1;
        }
    }

    IIC_Stop();
    return 0;
}

uint8_t IIC_Read_Buffer(uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t len)
{
    IIC_Start();
    // dummy write
    IIC_Write_Data((address << 1) | 0);
    if (IIC_Waite_Ack())
    {
        IIC_Stop();
        return 1;
    }
    IIC_Write_Data(reg);
    IIC_Waite_Ack();

    // real wirte
    IIC_Start();
    IIC_Write_Data((address << 1) | 1);
    if (IIC_Waite_Ack())
    {
        IIC_Stop();
        return 1;
    }
    while (len)
    {
        if (len == 1)
        {
            *buffer = IIC_Read_Data(0);
        }
        else
        {
            *buffer = IIC_Read_Data(1);
        }
        len--;
        buffer++;
    }
    IIC_Stop();
    return 0;
}