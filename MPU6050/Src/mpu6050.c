#include "../Include/mpu6050.h"

void MPU_Init(void)
{
    uint8_t res;
    uint8_t value;
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(MPU_AD0_Port, MPU_AD0_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = MPU_AD0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    IIC_Init();
    value = 0x80;
    IIC_Write_Buffer(MPU_ADDR, MPU_PWR_MGMT1_REG, &value, 1); //复位MPU6050
    HAL_Delay(100);
    value = 0x00;
    IIC_Write_Buffer(MPU_ADDR, MPU_PWR_MGMT1_REG, &value, 1); //唤醒MPU6050
    MPU_Set_Gyroscope_FSR(3);                                 //陀螺仪传感器,±2000dps
    MPU_Set_Accelerator_FSR(0);                               //加速度传感器,±2g
    MPU_Set_Rate(50);                                         //设置采样率50Hz
    IIC_Write_Buffer(MPU_ADDR, MPU_INT_EN_REG, &value, 1);    //关闭所有中断
    IIC_Write_Buffer(MPU_ADDR, MPU_USER_CTRL_REG, &value, 1); //I2C主模式关闭
    IIC_Write_Buffer(MPU_ADDR, MPU_FIFO_EN_REG, &value, 1);   //关闭FIFO
    value = 0x80;
    IIC_Write_Buffer(MPU_ADDR, MPU_INTBP_CFG_REG, &value, 1); //INT引脚低电平有效
    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);
    if (res == MPU_ADDR) //器件ID正确
    {
        value = 0x01;
        IIC_Write_Buffer(MPU_ADDR, MPU_PWR_MGMT1_REG, &value, 1); //设置CLKSEL,PLL X轴为参考
        value = 0x00;
        IIC_Write_Buffer(MPU_ADDR, MPU_PWR_MGMT2_REG, &value, 1); //加速度与陀螺仪都工作
        MPU_Set_Rate(50);                                         //设置采样率为50Hz
    }
    else
        return 1;
    return 0;
}

void MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data = 0;
    if (lpf >= 188)
        data = 1;
    else if (lpf >= 98)
        data = 2;
    else if (lpf >= 42)
        data = 3;
    else if (lpf >= 20)
        data = 4;
    else if (lpf >= 10)
        data = 5;
    else
        data = 6;
    return IIC_Write_Buffer(MPU_ADDR, MPU_CFG_REG, &data, 1); //设置数字低通滤波器
}

void MPU_Set_Gyroscope_FSR(uint8_t fsr)
{
    fsr <<= 3;
    return IIC_Write_Buffer(MPU_ADDR, MPU_GYRO_CFG_REG, &fsr, 1); //设置陀螺仪满量程范围
}

void MPU_Set_Accelerator_FSR(uint8_t fsr)
{
    fsr <<= 3;
    return IIC_Write_Buffer(MPU_ADDR, MPU_ACCEL_CFG_REG, &fsr, 1); //设置加速度传感器满量程范围
}

void MPU_Set_Rate(uint16_t rate)
{
    uint8_t data;
    if (rate > 1000)
    {
        rate = 1000;
    }
    if (rate < 4)
    {
        rate = 4;
    }
    data = 1000 / rate - 1;
    IIC_Write_Buffer(MPU_ADDR, MPU_SAMPLE_RATE_REG, &data, 1);
    return MPU_Set_LPF(rate / 2);
}

uint8_t MPU_Get_Accelerator(uint16_t *accelerator_x, uint16_t *accelerator_y, uint16_t *accelerator_z)
{
    uint8_t buffer[6], res;
    res = IIC_Write_Buffer(MPU_ADDR, MPU_ACCEL_XOUTH_REG, buffer, 6);
    if (res == 0)
    {
        *accelerator_x = ((uint16_t)buffer[0] << 8) | buffer[1];
        *accelerator_y = ((uint16_t)buffer[2] << 8) | buffer[3];
        *accelerator_z = ((uint16_t)buffer[4] << 8) | buffer[5];
    }
    return res;
}

uint8_t MPU_Get_Gyroscope(uint16_t *gyroscope_x, uint16_t *gyroscope_y, uint16_t *gyroscope_z)
{
    uint8_t buffer[6], res;
    res = IIC_Write_Buffer(MPU_ADDR, MPU_GYRO_XOUTH_REG, buffer, 6);
    if (res == 0)
    {
        *gyroscope_x = ((uint16_t)buffer[0] << 8) | buffer[1];
        *gyroscope_y = ((uint16_t)buffer[2] << 8) | buffer[3];
        *gyroscope_z = ((uint16_t)buffer[4] << 8) | buffer[5];
    }
    return res;
}

uint16_t MPU_Get_Temperature()
{
    uint8_t buffer[2];
    uint16_t raw;
    float temp;
    IIC_Read_Buffer(MPU_ADDR, MPU_TEMP_OUTH_REG, buffer, 2);
    raw = ((uint16_t)buffer[0] << 8) | buffer[1];
    temp = 36.53 + ((double)raw) / 340;
    return temp * 100;
}
