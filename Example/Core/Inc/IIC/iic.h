#ifndef _IIC_H
#define _IIC_H

#include "stm32f1xx_hal.h"

#define IIC_SCL_Port GPIOA
#define IIC_SDA_Port GPIOA
#define IIC_SCL_Pin GPIO_PIN_1
#define IIC_SDA_Pin GPIO_PIN_2

void IIC_Init(void);

void IIC_Start(void);

void IIC_Stop(void);

uint8_t IIC_Waite_Ack();

uint8_t IIC_Write_Buffer(uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t len);

uint8_t IIC_Read_Buffer(uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t len);

void IIC_Ack();

void IIC_NoAck();

#endif