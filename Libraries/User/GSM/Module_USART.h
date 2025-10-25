#ifndef __MODULE_USART_H
#define __MODULE_USART_H

// ----------------- Include ----------------- //
#include "main.h"

// ----------------- Define ----------------- //
#define GSM_USART                               USART3
#define GSM_USART_IRQn                          USART3_4_IRQn

#define GSM_USART_RX_GPIO                       GPIOB
#define GSM_USART_RX_PIN                        GPIO_Pin_11

#define GSM_USART_TX_GPIO                       GPIOB
#define GSM_USART_TX_PIN                        GPIO_Pin_10

#define GSM_STATUS_GPIO                         GPIOB
#define GSM_STATUS_PIN                          GPIO_IDR_ID0

#define GSM_PWR_KEY_GPIO                        GPIOA
#define GSM_PWR_KEY_PIN                         GPIO_BSRR_BS6

#define GSM_PWR_EN_GPIO                         GPIOB
#define GSM_PWR_EN_PIN                          GPIO_BSRR_BS1

#define GSM_RI_GPIO                             GPIOA
#define GSM_RI_PIN                              GPIO_IDR_ID4

#define GSM_ENABLE_RI_EXTI                      EXTI->IMR1 |= EXTI_IMR1_IM4
#define GSM_DISABLE_RI_EXTI                     EXTI->IMR1 &= ~EXTI_IMR1_IM4

#define GSM_USART_RX_BUFFER_SIZE                540

#define GSM_EN_POWER                            GSM_PWR_EN_GPIO->BSRR = GSM_PWR_EN_PIN
#define GSM_DIS_POWER                           GSM_PWR_EN_GPIO->BRR = GSM_PWR_EN_PIN

// ----------------- Extern ----------------- //
extern uint8_t GSM_rx_buffer[GSM_USART_RX_BUFFER_SIZE];
extern uint16_t GSM_rx_wr_index ;
extern uint16_t GSM_rx_rd_index;
extern uint16_t GSM_rx_counter;


// ----------------- Prototype ----------------- //
void GSM_cfg_USART(void);
void GSM_cfg_GPIO(void);
void GSM_RX_Callback(void);
void GSM_Send_Usart(uint8_t *tx_data, uint16_t len);

#endif /* __MODULE_USART_H */