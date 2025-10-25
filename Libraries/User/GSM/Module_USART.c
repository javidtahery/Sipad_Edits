#include "Module_USART.h"


uint8_t GSM_rx_buffer[GSM_USART_RX_BUFFER_SIZE];
uint16_t GSM_rx_wr_index = 0;
uint16_t GSM_rx_rd_index = 0;
uint16_t GSM_rx_counter = 0;

void GSM_cfg_USART(void)
{
  /* .............. GPIO Init ..............*/
  GSM_USART_TX_GPIO->MODER = (GSM_USART_TX_GPIO->MODER & (~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11))) | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1;       // Set as Alternative
  GSM_USART_TX_GPIO->OSPEEDR |= (GPIO_OSPEEDR_OSPEED10 | GPIO_OSPEEDR_OSPEED11);
  GSM_USART_TX_GPIO->PUPDR = (GSM_USART_TX_GPIO->PUPDR & (~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11))) | GPIO_PUPDR_PUPD10_1 | GPIO_PUPDR_PUPD11_1;
  
  GSM_USART_TX_GPIO->AFR[1] = (GSM_USART_TX_GPIO->AFR[1] & 0xFFFF00FF) | 0x00004400;            // Alternative Function 1
  
  /* .............. USART Init ..............*/
  GSM_USART->BRR = SystemCoreClock / 115200;
  GSM_USART->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE_RXFNEIE;
  GSM_USART->CR1 |= USART_CR1_UE;
  
  /* .............. IRQ Init ..............*/
  NVIC_EnableIRQ(USART3_4_IRQn);                                                  // Enable GSM USART NVIC
  NVIC_SetPriority(USART3_4_IRQn, 1);                                             // Set Priority
  
  NVIC_ClearPendingIRQ(USART3_4_IRQn);
  
  GSM_USART->ICR |= USART_ICR_TCCF;
}


void GSM_cfg_GPIO(void)
{
  /* ------------- Status ------------- */ 
  GSM_STATUS_GPIO->MODER = GSM_STATUS_GPIO->MODER & (~GPIO_MODER_MODE0);                                // Set as Input
  GSM_STATUS_GPIO->PUPDR = (GSM_STATUS_GPIO->PUPDR & (~GPIO_PUPDR_PUPD0)) | GPIO_PUPDR_PUPD0_1;         // Pulled Down
  
  /* ------------- Power Key ------------- */
  GSM_PWR_KEY_GPIO->MODER = (GSM_PWR_KEY_GPIO->MODER & (~GPIO_MODER_MODE6)) | GPIO_MODER_MODE6_0;       // Set as Output
  GSM_PWR_KEY_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
  GSM_PWR_KEY_GPIO->PUPDR = (GSM_PWR_KEY_GPIO->PUPDR & (~GPIO_PUPDR_PUPD6)) | GPIO_PUPDR_PUPD6_1;       // Pulled Down
  
  GSM_PWR_KEY_GPIO->BRR = GSM_PWR_KEY_PIN;
  
  /* ------------- Power Control ------------- */
  GSM_PWR_EN_GPIO->MODER = (GSM_PWR_EN_GPIO->MODER & (~GPIO_MODER_MODE1)) | GPIO_MODER_MODE1_0;       // Set as Output
  GSM_PWR_EN_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED1;
  GSM_PWR_EN_GPIO->PUPDR = (GSM_PWR_EN_GPIO->PUPDR & (~GPIO_PUPDR_PUPD1)) | GPIO_PUPDR_PUPD1_1;       // Pulled Down
  
  GSM_DIS_POWER;
  
  /* ------------- RI ------------- */
//  GSM_RI_GPIO->MODER = (GSM_RI_GPIO->MODER & (~GPIO_MODER_MODE4));                                  // Set as Input
//  GSM_RI_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
//  GSM_RI_GPIO->PUPDR = (GSM_RI_GPIO->PUPDR & (~GPIO_PUPDR_PUPD4)) | GPIO_PUPDR_PUPD4_0;             // Pulled Up
//  
//  EXTI->FTSR1 |= EXTI_FTSR1_FT4;                                                // Rising trigger cfg
//  EXTI->FPR1 |= EXTI_FPR1_FPIF4;                                                // Clear pending bit
//  EXTI->EXTICR[1] = (EXTI->EXTICR[1] & (~EXTI_EXTICR2_EXTI4));                  // Port Source: GPIOA
//  EXTI->IMR1 &= ~EXTI_IMR1_IM4;                                                 // Disable EXTI Interrupt
//  
//  NVIC_SetPriority(EXTI4_15_IRQn, 2);                                           // Set Priority
//  NVIC_EnableIRQ(EXTI4_15_IRQn);                                                // Enable 1PPS NVIC
//  
//  NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
}


void GSM_RX_Callback(void)
{
  GSM_rx_buffer[GSM_rx_wr_index++] = *(__IO uint8_t *)&GSM_USART->RDR;
  if(GSM_rx_wr_index == GSM_USART_RX_BUFFER_SIZE)
    GSM_rx_wr_index = 0;
  if(GSM_rx_counter < GSM_USART_RX_BUFFER_SIZE)
    GSM_rx_counter++;
}


void GSM_Send_Usart(uint8_t* tx_data, uint16_t len)
{
  while( (GSM_USART->ISR & USART_ISR_BUSY) == USART_ISR_BUSY);
  
  for(uint16_t iterator = 0; iterator < len; iterator++)
  {
    *(__IO uint8_t *)&GSM_USART->TDR = *(tx_data+iterator);
    while( (GSM_USART->ISR & USART_ISR_TC) != USART_ISR_TC);
  }
}