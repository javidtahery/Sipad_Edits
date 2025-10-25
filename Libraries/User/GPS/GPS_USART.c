#include "GPS_USART.h"

uint8_t GPS_rx_buffer[GPS_USART_RX_BUFFER_SIZE];
uint16_t GPS_rx_wr_index = 0;
uint16_t GPS_rx_rd_index = 0;
uint16_t GPS_rx_counter = 0;

void GPS_cfg_GPIO(void)
{
  /* ------------- GNSS EN ------------- */ 
  GPS_GNSS_EN_GPIO->MODER = (GPS_GNSS_EN_GPIO->MODER & (~GPIO_MODER_MODE5)) | GPIO_MODER_MODE5_0;       // Set as Output
  GPS_GNSS_EN_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED5;
  GPS_GNSS_EN_GPIO->PUPDR = (GPS_GNSS_EN_GPIO->PUPDR & (~GPIO_PUPDR_PUPD5)) | GPIO_PUPDR_PUPD5_1;       // Pulled Down
  
  /* ------------- 1PPS ------------- */
  GPS_1PPS_GPIO->MODER = (GPS_1PPS_GPIO->MODER & (~GPIO_MODER_MODE4));                                  // Set as Input
  GPS_1PPS_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;
  GPS_1PPS_GPIO->PUPDR = (GPS_1PPS_GPIO->PUPDR & (~GPIO_PUPDR_PUPD4)) | GPIO_PUPDR_PUPD4_1;             // Pulled Down
  
  // 1PPS EXTI
  EXTI->FTSR1 |= EXTI_FTSR1_FT4;                                               // Falling trigger cfg
  EXTI->FPR1 |= EXTI_FPR1_FPIF4;                                               // Clear Falling pending bit
  EXTI->EXTICR[1] = (EXTI->EXTICR[1] & (~EXTI_EXTICR2_EXTI4));                 // Port Source: GPIOA
  EXTI->IMR1 |= EXTI_IMR1_IM4;                                                 // Enable EXTI Interrupt
  
  // Software Interrupt (Triggered in Timer to read the GPS data)                                                        // Software trigger cfg
  /* !!! Important
     Configure the Pin3 of GPIOB as EXTI without edge detection.
     When pin is triggered with software a rising edge signal in detected
  */
//  EXTI->RPR1 |= EXTI_RPR1_RPIF3;              // Clear pending bit
//  EXTI->EXTICR[0] = (EXTI->EXTICR[0] & (~EXTI_EXTICR1_EXTI3)) | EXTI_EXTICR1_EXTI3_0;   // Port Source: GPIOB
//  EXTI->IMR1 |= EXTI_IMR1_IM3;                                                 // Enable EXTI Interrupt
//  
//  NVIC_EnableIRQ(EXTI2_3_IRQn);                                                // Enable 1PPS NVIC
//  NVIC_SetPriority(EXTI2_3_IRQn, 2);                                           // Set Priority
//  NVIC_ClearPendingIRQ(EXTI2_3_IRQn);

  
  NVIC_EnableIRQ(EXTI4_15_IRQn);                                                // Enable 1PPS NVIC
  NVIC_SetPriority(EXTI4_15_IRQn, 2);                                           // Set Priority
  NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
  
  GPS_SET_ON;
}

void GPS_cfg_USART(void)
{
  /* .............. GPIO Init ..............*/
  GPS_USART_TX_GPIO->MODER = (GPS_USART_TX_GPIO->MODER & (~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3))) | GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1;       // Set as Alternative
  GPS_USART_TX_GPIO->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
  GPS_USART_TX_GPIO->PUPDR = (GPS_USART_TX_GPIO->PUPDR & (~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3))) | GPIO_PUPDR_PUPD2_1 | GPIO_PUPDR_PUPD3_1;       // Pulled Down
  
  GPS_USART_TX_GPIO->AFR[0] = (GPS_USART_TX_GPIO->AFR[0] & 0xFFFF00FF) | 0x00001100;            // Alternative Function 4
  
  /* .............. USART Init ..............*/
  GPS_USART->BRR = SystemCoreClock / 115200;
  GPS_USART->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE_RXFNEIE;
  GPS_USART->CR1 |= USART_CR1_UE;
  
  /* .............. IRQ Init ..............*/
  NVIC_EnableIRQ(USART2_IRQn);                                                // Enable GPS USART NVIC
  NVIC_SetPriority(USART2_IRQn, 1);                                           // Set Priority
  
  NVIC_ClearPendingIRQ(USART2_IRQn);
  
  GPS_USART->ICR |= USART_ICR_TCCF;
}

void GPS_RX_Callback(void)
{
  GPS_rx_buffer[GPS_rx_wr_index++] = *(__IO uint8_t *)&GPS_USART->RDR;
  if(GPS_rx_wr_index == GPS_USART_RX_BUFFER_SIZE)
    GPS_rx_wr_index = 0;
  if(GPS_rx_counter < GPS_USART_RX_BUFFER_SIZE)
    GPS_rx_counter++;
}

void GPS_Send_Usart(uint8_t *tx_data, uint16_t len)
{
  while( (GPS_USART->ISR & USART_ISR_BUSY) == USART_ISR_BUSY);
  
  for(uint16_t iterator = 0; iterator < len; iterator++)
  {
    *(__IO uint8_t *)&GPS_USART->TDR = *(tx_data+iterator);
    while( (GPS_USART->ISR & USART_ISR_TC) != USART_ISR_TC);
  }
}
