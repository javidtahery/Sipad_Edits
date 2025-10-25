#ifndef __GPS_USART_H
#define __GPS_USART_H

/* --------------------- Include --------------------- */
#include "main.h"

/* --------------------- Define --------------------- */
#define GPS_USART                               USART2

#define GPS_USART_RX_GPIO                       GPIOA
#define GPS_USART_RX_PIN                        GPIO_Pin_3

#define GPS_USART_TX_GPIO                       GPIOA
#define GPS_USART_TX_PIN                        GPIO_Pin_2

#define GPS_GNSS_EN_GPIO                        GPIOA
#define GPS_GNSS_EN_PIN                         GPIO_BSRR_BS5

#define GPS_1PPS_GPIO                           GPIOA
#define GPS_1PPS_PIN                            GPIO_Pin_4

#define GPS_USART_RX_BUFFER_SIZE                350

#define GPS_SET_OFF                             GPS_GNSS_EN_GPIO->BSRR = GPS_GNSS_EN_PIN
#define GPS_SET_ON                              GPS_GNSS_EN_GPIO->BRR = GPS_GNSS_EN_PIN
#define GPS_DISABLE_1PPS_EXTI                   EXTI->IMR1 &= ~EXTI_IMR1_IM4
#define GPS_ENABLE_1PPS_EXTI                    EXTI->IMR1 |= EXTI_IMR1_IM4
/* --------------------- Structure --------------------- */


/* --------------------- Enum --------------------- */


/* --------------------- Extern --------------------- */
extern uint8_t GPS_rx_buffer[GPS_USART_RX_BUFFER_SIZE];
extern uint16_t GPS_rx_wr_index;
extern uint16_t GPS_rx_rd_index;
extern uint16_t GPS_rx_counter;

/* --------------------- Prototype --------------------- */
void GPS_cfg_GPIO(void);
void GPS_cfg_1PPS_EXTI(FunctionalState state);
void GPS_cfg_USART(void);
void GPS_reset_module(void);
void GPS_RX_Callback(void);
void GPS_Send_Usart(uint8_t *tx_data, uint16_t len);


#endif  /* __GPS_USART_H */