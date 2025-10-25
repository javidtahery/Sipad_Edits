#include "main.h"


void NMI_Handler(void)
{
  if( (RCC->CIFR & RCC_CIFR_CSSF) == RCC_CIFR_CSSF)
  {
    // Reconfigure RCC with HSI16
    Sys_cfg_clock_HSI();
    
    crystal_is_failed = SET;
    RCC->CICR |= RCC_CICR_CSSC;
  }
}


void HardFault_Handler(void)
{
  while(1)
  {
    asm("nop");
  }
}


void SysTick_Handler(void)
{
  Tick_decrement();
}


/* -------------------- Timers -------------------- */
void OneSec_Timer_IRQHandler(void)
{
  OneSec_handler();
}

void Fifty_Hz_Timer_IRQHandler(void)
{
  Fifty_Hz_handler();
}

void Twentyfive_Hz_Timer_IRQHandler(void)
{
  Twentyfive_Hz_handler();
}

/* -------------------- USART -------------------- */
void USART2_IRQHandler(void)
{
  // GPS USART RX
  if( (GPS_USART->ISR & USART_ISR_RXNE_RXFNE) == USART_ISR_RXNE_RXFNE )
  {
    GPS_RX_Callback();
  }
  if( (GPS_USART->ISR & USART_ISR_ORE) == USART_ISR_ORE )
  {
    GPS_USART->ICR |= USART_ICR_ORECF;
    GPS_clear_rx_buffer();
  }
  if( (GPS_USART->ISR & USART_ISR_FE) == USART_ISR_FE )
  {
    GPS_USART->ICR |= USART_ICR_FECF;
    //    GPS_clear_rx_buffer();
  }
  if( (GPS_USART->ISR & USART_ISR_IDLE) == USART_ISR_IDLE )
  {
    GPS_USART->ICR |= USART_ICR_IDLECF;
    //    GPS_clear_rx_buffer();
  }
}

void USART3_4_IRQHandler(void)
{
  // GSM USART
  if( (GSM_USART->ISR & USART_ISR_RXNE_RXFNE) == USART_ISR_RXNE_RXFNE )
  {
    GSM_RX_Callback();
  }
  if( (GSM_USART->ISR & USART_ISR_ORE) == USART_ISR_ORE )
  {
    GSM_USART->ICR |= USART_ICR_ORECF;
    GPS_clear_rx_buffer();
  }
  if( (GSM_USART->ISR & USART_ISR_FE) == USART_ISR_FE )
  {
    GSM_USART->ICR |= USART_ICR_FECF;
  }
  if( (GSM_USART->ISR & USART_ISR_IDLE) == USART_ISR_IDLE )
  {
    GSM_USART->ICR |= USART_ICR_IDLECF;
  }
}

/* -------------------- EXTI -------------------- */
void EXTI2_3_IRQHandler(void)
{
//  if( (EXTI->RPR1 & EXTI_RPR1_RPIF3) == EXTI_RPR1_RPIF3 )
//  {
//    GPS_1PPS_SW_EXTI_Callback();
//    
//    EXTI->RPR1 |= EXTI_RPR1_RPIF3;              // Clear pending bit
//  }
}

void EXTI4_15_IRQHandler(void)
{
  // GPS 1PPS
  if( (EXTI->FPR1 & EXTI_FPR1_FPIF4) == EXTI_FPR1_FPIF4 )
  {
    OnePPS_EXTI_Callback();
    
    EXTI->FPR1 |= EXTI_FPR1_FPIF4;              // Clear pending bit
    NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
  }
  
  // GSM RI
  if( (EXTI->FPR1 & EXTI_FPR1_FPIF4) == EXTI_FPR1_FPIF4 )
  {
//    RI_EXTI_Callback();
    
    EXTI->FPR1 |= EXTI_FPR1_FPIF4;              // Clear pending bit
  }
  
  // ACCEL INT 1
  if( (EXTI->RPR1 & EXTI_RPR1_RPIF11) == EXTI_RPR1_RPIF11 )
  {
//    KX023_EXTI1_Callback();
    
    EXTI->RPR1 |= EXTI_RPR1_RPIF11;              // Clear pending bit
  }
}
