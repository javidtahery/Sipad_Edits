
#ifndef __KX023_H
#define __KX023_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */
//#define KX023_I2Cx_RCC                        RCC_APB1Periph_I2C2
//#define KX023_I2Cx                            I2C2
#define KX023_I2C_ADDR                          0x3C
//#define KX023_DEVID                             0xE5    // 229

//#define KX023_SCL_PORT                        GPIOB
//#define KX023_SCL_Pin                         GPIO_Pin_13
//#define KX023_SCL_RCC                         RCC_AHBPeriph_GPIOB
//#define KX023_SCL_AF_PINSOURCE                GPIO_PinSource13
//#define KX023_SCL_AF                          GPIO_AF_5
//
//#define KX023_SDA_PORT                        GPIOB
//#define KX023_SDA_Pin                         GPIO_Pin_14
//#define KX023_SDA_RCC                         RCC_AHBPeriph_GPIOB
//#define KX023_SDA_AF_PINSOURCE                GPIO_PinSource14
//#define KX023_SDA_AF                          GPIO_AF_5

#define KX023_INT1_PORT                          GPIOB
#define KX023_INT1_Pin                           GPIO_Pin_9
#define KX023_INT1_GPIO_RCC                      RCC_AHBPeriph_GPIOB
#define KX023_INT1_EXTI_PortSource               GPIO_PortSourceGPIOB
#define KX023_INT1_EXTI_PinSource                GPIO_PinSource9
#define KX023_INT1_EXTI_Line                     EXTI_Line9
#define KX023_INT1_EXTI_IRQn                     EXTI9_5_IRQn
#define KX023_INT1_EXTI_TRIG_MODE                EXTI_Trigger_Rising

#define KX023_INT2_PORT                          GPIOB
#define KX023_INT2_Pin                           GPIO_Pin_8
#define KX023_INT2_GPIO_RCC                      RCC_AHBPeriph_GPIOB
#define KX023_INT2_EXTI_PortSource               EXTI_PortSourceGPIOB
#define KX023_INT2_EXTI_PinSource                EXTI_PinSource8
#define KX023_INT2_EXTI_Line                     EXTI_Line8
#define KX023_INT2_EXTI_IRQn                     EXTI4_15_IRQn


#define KX023_REG_DATAX0                         0x06 

#define KX023_REG_WHO_AM_I                       0x0F  
#define KX023_REG_CNTL1                          0x18   
#define KX023_REG_CNTL2                          0x19 
#define KX023_REG_CNTL3                          0x1A 

#define KX023_REG_ODCNTL                         0x1B 

#define KX023_REG_BUF_CNTL1                      0x3A 
#define KX023_REG_BUF_CNTL2                      0x3B

#define KX023_REG_TTH                            0x26
#define KX023_REG_TTL                            0x27
#define KX023_REG_TLT                            0x2A
#define KX023_REG_TWS                            0x2B
#define KX023_REG_FTD                            0x28
#define KX023_REG_TDTRC                          0x24

#define KX023_REG_INS1                           0x12
#define KX023_REG_INS2                           0x13
#define KX023_REG_INS3                           0x14

#define KX023_REG_STATUS                         0x15

#define KX023_REG_INC1                           0x1C   
#define KX023_REG_INC2                           0x1D   
#define KX023_REG_INC3                           0x1E  
#define KX023_REG_INC4                           0x1F   
#define KX023_REG_INC5                           0x20   
#define KX023_REG_INC6                           0x21   

#define KX023_WHO_AM_I_RESP                      0x15

#define KX023_OK				 0
#define KX023_TIMEOUT				 1
#define KX023_FAULT                              2

#define KX023_EXTI_HALT_DEFAULT_TIME             180
#define KX023_EXTI_ON_HALT_TIME                  0

#define KX023_ACCEL_ALPHA                        0.3


/* ---------------- Enum ---------------- */


/* ---------------- Structure ---------------- */
typedef struct{
  float axis_x;
  float axis_y;
  float axis_z;
  float x_offset;
  float y_offset;
  float z_offset;
//  int16_t roll_angle;
//  int16_t pitch_angle;
//  int16_t yaw_angle;
  uint8_t movement_x_status;
  uint8_t movement_y_status;
  uint8_t movement_z_status;
  uint8_t vehicle_moving;
}KX023_acceleration_typeDef;

typedef struct{
  float axis_x;
  float axis_y;
  float axis_z;
}KX023_typeDef;


/* ---------------- Extern ---------------- */
extern uint8_t KX023_EXTI1_Halt_Timer;
extern KX023_acceleration_typeDef   KX023;



/* ---------------- Prototype ---------------- */
void KX023_Config(void);
void KX023_INT1_Handle(void);
void KX023_read_raw_data(int16_t* data_x, int16_t* data_y, int16_t* data_z);
void KX023_EXTI1_Callback(void);
void KX023_startup_calibration(void);
uint8_t Accel_Recovery_I2C(void);

#endif /* __KX023_H */