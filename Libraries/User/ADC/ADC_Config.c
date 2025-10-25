#include "ADC_Config.h"

analog_typedef       ADC_Values;

uint16_t Calibration_factor;
uint8_t average_iteration = 0;
float main_sumation_value = 0;
float Internal_Battery_sumation_value = 0;



void ADC_Config(void)
{
  /* ADC Pins */
  GPIOB->MODER &= (~(GPIO_MODER_MODE2 | GPIO_MODER_MODE12));                                        // Main Power ADC pin
  GPIOB->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE2_0) | (GPIO_MODER_MODE12_1 | GPIO_MODER_MODE12_0) ;                   
  
  GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED12);                                    
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD12);                                                                                              
  
  // Disable ADC DMA request
  ADC1->CFGR1 &= ~(ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG);
  DMA1_Channel3->CCR &= ~DMA_CCR_EN;
  
  /* Config ADC and DMA Peripheral */
  DMA_Config_for_ADC();
  
  // ADC Clock seletion
  // PCLK divided by 4 
  ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;
  ADC1->CFGR2 |= ADC_CFGR2_CKMODE_0 | ADC_CFGR2_CKMODE_1;
  //ADC1_COMMON->CCR |= ADC_CCR_PRESC_0 | ADC_CCR_PRESC_1 | ADC_CCR_PRESC_3;
  
  // Stop Any Conversion
  if( (ADC1->CR & ADC_CR_ADSTART) == ADC_CR_ADSTART)
  {
    // Stop ADC
    ADC1->CR |= ADC_CR_ADSTP;
    while ((ADC1->CR & ADC_CR_ADSTP) == ADC_CR_ADSTP);
  }
  
  // Disable ADC
  if( (ADC1->CR & ADC_CR_ADEN) == ADC_CR_ADEN)
  {
    // Disable ADC
    ADC1->CR |= ADC_CR_ADDIS;
    while ((ADC1->CR & ADC_CR_ADEN) == ADC_CR_ADEN);
    ADC1->ISR |= ADC_ISR_ADRDY;                          // Optional
  }
  
  ADC1->CFGR1 &= ~ADC_CFGR1_RES;                        // Data Resolution: 12 Bit
  ADC1->CFGR1 &= ~ADC_CFGR1_SCANDIR;                    // Scan Direction: Upward
  ADC1->CFGR1 &= ~ADC_CFGR1_CHSELRMOD ;                 // Sequencer not fully configurable:
  ADC1->CHSELR |= (ADC_CHSELR_CHSEL10 | ADC_CHSELR_CHSEL12 | ADC_CHSELR_CHSEL13 | ADC_CHSELR_CHSEL16);
  while ((ADC1->ISR & ADC_ISR_CCRDY) != ADC_ISR_CCRDY );// ADC channel configuration ready
  
  ADC1->CFGR1 |= ADC_CFGR1_CONT;                                                //Continuous conversion mode (CONT = 1)
  ADC1->CFGR1 &= ~ADC_CFGR1_DISCEN ; 
  ADC1->CFGR1 |= ADC_CFGR1_OVRMOD;                      // Enable OverRun
//  ADC1->CFGR1 |= ADC_CFGR1_WAIT;
  ADC1->CFGR1 &= ~(ADC_CFGR1_EXTEN | ADC_CFGR1_EXTSEL); // Hardware Trigger detection disable
  
  // Programmable sampling time - 10.8125 us 
  ADC1->SMPR &= ~ADC_SMPR_SMP2;
  ADC1->SMPR &= ~ADC_SMPR_SMP1;
  ADC1->SMPR |= (ADC_SMPR_SMP1_0 | ADC_SMPR_SMP1_1 | ADC_SMPR_SMP1_2) ;
  ADC1->SMPR |= ~(ADC_SMPR_SMPSEL10 |ADC_CHSELR_CHSEL12 | ADC_CHSELR_CHSEL13 | ADC_CHSELR_CHSEL16);  // Choose sampling times configured in SMP1
  
  // Enable temprature and Vref
  ADC1_COMMON->CCR |= (ADC_CCR_TSEN | ADC_CCR_VREFEN); 
  
  // Enable ADC internal voltage regulator
  ADC1->CR |= ADC_CR_ADVREGEN;
  while( (ADC1->CR & ADC_CR_ADVREGEN ) != ADC_CR_ADVREGEN);
  
  Delay(1);
  // Software Calibration procedure
  ADC1->CR |= ADC_CR_ADCAL;
  while( (ADC1->CR & ADC_CR_ADCAL ) == ADC_CR_ADCAL);
  Calibration_factor = ADC1->DR;
  
  
  // DMA Should be enabled after ADC calibration
  ADC1->CFGR1 |= ADC_CFGR1_DMAEN;
  ADC1->CFGR1 |= ADC_CFGR1_DMACFG;                              // DMA circular mode (DMACFG = 1)
  
  /* Enable adc */
  ADC1->CR |= ADC_CR_ADEN;
  while((ADC1->ISR & ADC_ISR_ADRDY ) != ADC_ISR_ADRDY);
  
  /* Start of ADC Conversion */
  if((ADC1->CR & ADC_CR_ADEN ) == ADC_CR_ADEN )
  {
    ADC1->CR = ADC1->CR | ADC_CR_ADSTART;
  }
}


void DMA_Config_for_ADC(void)
{  
  DMAMUX1_Channel2->CCR |= (DMAMUX_CxCR_DMAREQ_ID_0 | DMAMUX_CxCR_DMAREQ_ID_2); // DMAMUX request line multiplexer = 5 -> ADC
  DMAMUX1_Channel2->CCR &= ~DMAMUX_CxCR_SE;                                     // Synchronization mode and channel event generation
  // DMAMUX1_Channel0->CCR |= DMAMUX_CxCR_SYNC_ID_0; //Synchronization mode and channel event generation
  
  DMA1_Channel3->CPAR =  0x40012440 ;                                           //&ADC1->DR ;  //DMA channel x peripheral address register --The start address used for the first single transfer 
  DMA1_Channel3->CMAR = (uint32_t)&ADC_Values;                                  // DMA channel x memory address register -- The start address used for the first single transfer 
  DMA1_Channel3->CNDTR = 4;                                                     // DMA channel x number of data register  --This register contains the remaining number of data items to transfer 
  
  DMA1_Channel3->CCR |= DMA_CCR_PL_1 ;                                          //Channel Priority level to high
  
  //----Before enabling a channel in circular mode (CIRC = 1), the software must clear the MEM2MEM bit ---//
  DMA1_Channel3->CCR &= ~DMA_CCR_MEM2MEM ;                                      // Disable memory to memory mode    
  DMA1_Channel3->CCR |=  DMA_CCR_CIRC ;                                         // Circular mode  
  
  //------The peripheral and memory pointers may be automatically incremented after each transfer,----///
  DMA1_Channel3->CCR &= ~DMA_CCR_DIR ;                                          //Data transfer direction -- defines typically a peripheral-to-memory transfer
  
  DMA1_Channel3->CCR &= ~DMA_CCR_PINC ;                                         // Disable Source -> Peripheral increment mode  
  DMA1_Channel3->CCR |= DMA_CCR_MINC ;                                          // Enable Destination -> Memory increment mode
  
  DMA1_Channel3->CCR |= DMA_CCR_PSIZE_0 ;                                       // Source -> Peripheral data size -> 16 bit
  DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0 ;                                       // Destination -> Memory data size -> 16 bit
  
  DMA1_Channel3->CCR |= DMA_CCR_EN ;                                            // Channel enable--Activate the channel by setting the EN bit in the DMA_CCRx register.
}


void ADC_calculate_average(void)
{
  if(++average_iteration >= MAX_ADC_ITERATION_COUNT)
  {
    ADC_Values.Vref_voltage = VREFINT * VREFINT_CAL / ADC_Values.vref;
    main_sumation_value /= MAX_ADC_ITERATION_COUNT;
    Internal_Battery_sumation_value /= MAX_ADC_ITERATION_COUNT;
    ADC_Values.Average_Main_Power_Voltage = 2.6862 * main_sumation_value * ADC_Values.Vref_voltage + MAIN_VOLTAGE_BIAS_VOLTAGE;                     // (13.2/1.2)*1000/4095
    ADC_Values.Average_Internal_BAT = 0.322693 * Internal_Battery_sumation_value * ADC_Values.Vref_voltage;             // (7.4/5.6)*1000/4095  (1794, 5565)
    
    main_sumation_value = 0;
    Internal_Battery_sumation_value = 0;
    average_iteration = 0;
    
    ADC_Values.Average_Temperture = ((uint16_t)((float)(ADC_Values.Temperture - TS_CAL1)*(80 / AVG_SLOPE)) + 35);
  }
  
  main_sumation_value += ADC_Values.Main_power;
  Internal_Battery_sumation_value += ADC_Values.Internal_Battery;
  
  if( ADC_Values.Average_Internal_BAT < LOW_BATTERY_ALARM_THRESHOLD )
  {
    if(low_battery_detection == RESET)
    {
      if(repetition_low_battery_counter == 0)
      {
        low_battery_detection = SET;
        
        Force_Produce_Event();
        Event_Flags.low_internal_battery = ENABLE;
        repetition_low_battery_counter = REPETITION_LOW_BAT_EVENT_THRESHOLD;
      }
    }
  }
  else
  {
    if(low_battery_detection == SET)
    {
      low_battery_detection = RESET;
    }
  }
}

