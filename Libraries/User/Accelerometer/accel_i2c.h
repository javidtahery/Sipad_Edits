#ifndef __ACCEL_I2C_H
#define __ACCEL_I2C_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */
#define ACCEL_I2C                               I2C1
#define ACCEL_I2C_SPEED                         300000

#define ACCEL_SCL_GPIO                          GPIOB
#define ACCEL_SCL_PIN                           GPIO_BSRR_BS6

#define ACCEL_SDA_GPIO                          GPIOB
#define ACCEL_SDA_PIN                           GPIO_BSRR_BS7

#define ACCEL_INT1_GPIO                         GPIOB
#define ACELL_INT1_PIN                          GPIO_BSRR_BS8

#define ACELL_INT2_GPIO                         GPIOB
#define ACELL_INT2_PIN                          GPIO_BSRR_BS8

#define ACELL_INT1_DISABLE_EXTI                 EXTI->IMR1 &= ~EXTI_IMR1_IM8
#define ACELL_INT1_ENABLE_EXTI                  EXTI->IMR1 |= EXTI_IMR1_IM8

#define I2C_BUS_TMIEOUT_TIME                    1000
#define I2C_BUS_FAULT                           SET
#define I2C_BUS_OK                              RESET


#define ACCEL_I2C_ADDR                          KX023_I2C_ADDR

/* ---------------- Enum ---------------- */

/* ---------------- Structure ---------------- */

/* ---------------- Extern ---------------- */

/* ---------------- Prototype ---------------- */
void Accel_I2C_config(void);
uint16_t Accel_read_byte(uint8_t* read_data, uint8_t reg);
uint16_t Accel_read_nbytes(uint8_t* read_data, uint8_t nbytes, uint8_t reg);
uint16_t Accel_read_n_bytes_DMA(uint8_t* read_data, uint8_t nbytes, uint8_t reg);
uint16_t Accel_write_byte(uint8_t data, uint8_t reg);
void I2C_generate_stop_condition_manually(void);


#endif  /* __ACCEL_I2C_H */