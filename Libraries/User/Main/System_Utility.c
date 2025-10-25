#include "System_Utility.h"

__IO uint32_t sysTick_decrement = 0;
__IO uint16_t cfg_timeout_counter = 0;
__IO uint32_t GPS_rx_timeout = 0;
uint8_t crystal_is_failed = RESET;

const char Software_version[] = "V1463 - 2025/04/26";
const char Hardware_Version[] = "SIP_HW v1.3.0.0 - 2022/11/09";


void Sys_cfg_clock_HSE(void);
void IWDG_Init(void);
void SYS_cfg_Peripherals_RCC(void);


void System_Config(void)
{
  // Application Protection
  if(Bootloader_Password != 0xF2D7)
    __NVIC_SystemReset();
  
  /* ----------- Systick and Clock ----------- */
  Sys_cfg_clock_HSE();

  /* --------------- IWDG Config --------------- */
  IWDG_Init();

  /* ------------- Peripherals RCC ------------- */
  SYS_cfg_Peripherals_RCC();

  /* ------------- Timers ------------- */
  OneSec_Timer_Init();
  timers_does_not_work = SET;

  /* ------------- Digital IOs ------------- */
  IO_cfg_Digital_IOs();

  /* ------------- SPIF ------------- */
  SPIF_cfg_spi();
  SPIF_cfg_chip();
  if(SPIF_device_ID != SPIF_MANF_ID_32 && SPIF_device_ID != SPIF_MANF_ID_64)
    __NVIC_SystemReset();
  SPIF_read_setting();
  SPIF_read_system_IMEI();
  SPIF_read_system_serial();
  SPIF_Read_sys_temp_params();
  FCB_startup_record_analyze();

  /* ------------- Accelerometer ------------- */
  I2C_generate_stop_condition_manually();
  KX023_Config();

  /* ------------- GSM ------------- */
  GSM_cfg_GPIO();
  GSM_cfg_USART();
  //GSM_power_on();

  /* ------------- GPS ------------- */
  GPS_cfg_GPIO();
  GPS_cfg_USART();
  GPS_Config_Module();
  gps_activity = SET;

  /* ------------- ADC ------------- */
  ADC_Config();

  /* ------------- Timers ------------- */
  Twentyfive_Hz_Timer_Init();
  Fifty_Hz_Timer_Init();
  
  /* ------------- OdO Meter ------------- */
#ifdef ENABLE_ODOMETER
  Odo_Timer1_Init();
#endif

  /* General System Configuration */
  System_Startup_Initiate();
  
  // Retry to fix GPS error
  if(system_error.gps_error == SET)
    reconfigure_GPS = SET;
  
  check_startup_server_ok();
}


// System Clock = HSE / PLLM * PLLN / PLLR
void Sys_cfg_clock_HSE(void)
{
  uint8_t HSE_failed = RESET;
  SCB->VTOR = FLASH_BASE | APPLICATION_ADDRESS;
  
  if( SysTick_Config(SystemCoreClock / 1000) )
    NVIC_SystemReset();

  // Set MCU voltage range to: 'Range 1'
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  if( (PWR->CR1 & PWR_CR1_VOS) != PWR_CR1_VOS_0 )
  {
    PWR->CR1 = (PWR->CR1 & (~PWR_CR1_VOS)) | PWR_CR1_VOS_0;

    cfg_timeout_counter = 20;
    while( (PWR->SR2 & PWR_SR2_VOSF) == PWR_SR2_VOSF)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Set Flash LATENCY to '2 WS'
  if( (FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2 )
  {
    FLASH->ACR = (FLASH->ACR & (~FLASH_ACR_LATENCY)) | FLASH_ACR_LATENCY_2;

    cfg_timeout_counter = 40;
    while( (FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Enable Flash Prefetch
  if( (FLASH->ACR & FLASH_ACR_PRFTEN) != FLASH_ACR_PRFTEN )
  {
    FLASH->ACR |= FLASH_ACR_PRFTEN;
  }
  
  if( (RCC->CR & RCC_CR_PLLON) == RCC_CR_PLLON )
  {
    if( (RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_1 )
    {
      RCC->CFGR &= ~RCC_CFGR_SW_1;     // Reset from PLLSRC and set to HSISYS
    }
  }
  
  // Enable CSS
  RCC->CR |= RCC_CR_CSSON;

  // Enable HSE
  if( (RCC->CR & RCC_CR_HSEON) != RCC_CR_HSEON )
  {
    RCC->CR |= RCC_CR_HSEON;

    cfg_timeout_counter = 40;
    while( (RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY || HSE_failed == RESET)
    {
      if(cfg_timeout_counter == 0)
      {
        HSE_failed = SET;
        crystal_is_failed = SET;
        
        break;
      }
    }
  }

  // Enable PLL
  // System Clock = HSE / PLLM * PLLN / PLLR
  if( (RCC->CR & RCC_CR_PLLON) != RCC_CR_PLLON )
  {
    if(HSE_failed == RESET)
    {
      RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC)) | RCC_PLLCFGR_PLLSRC_HSE;
      RCC->PLLCFGR = (RCC->PLLCFGR & (~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR)))       // PLLM -> 1 (000), PLLN -> 16(0001000)
                  | (RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLR_0);                                          // PLLR -> 2 (001)
    }
    else
    {
      RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC)) | RCC_PLLCFGR_PLLSRC_HSI;
      RCC->PLLCFGR = (RCC->PLLCFGR & (~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR)))       // PLLM -> 2 (001), PLLN -> 16(0001000)
                  | (RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLR_0);                     // PLLR -> 2 (001)
    }
    
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->CR |= RCC_CR_PLLON;

    cfg_timeout_counter = 40;
    while( (RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Config HCLK, SYSCLK, AHP and APBs
  RCC->CFGR = (RCC->CFGR & (~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE)));                           // PRPE -> 1 (000), HPRE -> 1 (000)
  RCC->CFGR |= RCC_CFGR_SW_1;                                                                           // System Clock switch to PLLSRC

  cfg_timeout_counter = 50;
  while( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1)
  {
    if(cfg_timeout_counter == 0)
      NVIC_SystemReset();
  }

  // Config Systick
  SystemCoreClock = 64000000UL;
  if( SysTick_Config(SystemCoreClock / 1000) )
    NVIC_SystemReset();
  NVIC_SetPriority(SysTick_IRQn, 0);
}


// System Clock = HSI / PLLM * PLLN / PLLR
void Sys_cfg_clock_HSI(void)
{
  SystemCoreClock = 16000000UL;
  if( SysTick_Config(SystemCoreClock / 1000) )
    NVIC_SystemReset();
  
  // Set MCU voltage range to: 'Range 1'
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  if( (PWR->CR1 & PWR_CR1_VOS) != PWR_CR1_VOS_0 )
  {
    PWR->CR1 = (PWR->CR1 & (~PWR_CR1_VOS)) | PWR_CR1_VOS_0;

    cfg_timeout_counter = 20;
    while( (PWR->SR2 & PWR_SR2_VOSF) == PWR_SR2_VOSF)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Set Flash LATENCY to '2 WS'
  if( (FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2 )
  {
    FLASH->ACR = (FLASH->ACR & (~FLASH_ACR_LATENCY)) | FLASH_ACR_LATENCY_2;

    cfg_timeout_counter = 40;
    while( (FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Enable Flash Prefetch
  if( (FLASH->ACR & FLASH_ACR_PRFTEN) != FLASH_ACR_PRFTEN )
  {
    FLASH->ACR |= FLASH_ACR_PRFTEN;
  }

  // Enable PLL
  if( (RCC->CR & RCC_CR_PLLON) != RCC_CR_PLLON )
  {
    RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC)) | RCC_PLLCFGR_PLLSRC_HSI;
    RCC->PLLCFGR = (RCC->PLLCFGR & (~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR)))         // PLLM -> 2 (001), PLLN -> 16(0001000)
                  | (RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLR_0);                     // PLLR -> 2 (001)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->CR |= RCC_CR_PLLON;

    cfg_timeout_counter = 40;
    while( (RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
    {
      if(cfg_timeout_counter == 0)
        NVIC_SystemReset();
    }
  }

  // Config HCLK, SYSCLK, AHP and APBs
  RCC->CFGR = (RCC->CFGR & (~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE)));                           // PRPE -> 1 (000), HPRE -> 1 (000)
  RCC->CFGR |= RCC_CFGR_SW_1;                                                                           // System Clock switch to PLLSRC

  cfg_timeout_counter = 50;
  while( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1)
  {
    if(cfg_timeout_counter == 0)
      NVIC_SystemReset();
  }

  // Config Systick
  SystemCoreClock = 64000000UL;
  if( SysTick_Config(SystemCoreClock / 1000) )
    NVIC_SystemReset();
  NVIC_SetPriority(SysTick_IRQn, 0);
}


void IWDG_Init(void)
{
  // Enable LSI
  RCC->CSR |= RCC_CSR_LSION;

  cfg_timeout_counter = 50;
  while((RCC->CSR & RCC_CSR_LSIRDY) != RCC_CSR_LSIRDY)
  {
    if(cfg_timeout_counter == 0)
      NVIC_SystemReset();
  }

  IWDG->KR = 0x0000CCCC;        // Enable the IWDG
  IWDG->KR = 0x00005555;        // Enable register write access
  IWDG->PR = 0x05;              // Set the prescaler to 128 (32KHz / 128 = 250 Hz)
  IWDG->RLR = 0x0BB8;           // 3000(12 sec = 3000 @ 128 prescaler) // The register is 12 bits long

  cfg_timeout_counter = 50;
  while ( IWDG->SR != 0x00000000)
  {
    if(cfg_timeout_counter == 0)
      NVIC_SystemReset();
  }

  IWDG_ReloadCounter();
}


void SYS_cfg_Peripherals_RCC(void)
{
  // Enable DMA clock
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;

  // Enable GPIO clock
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIODEN;

  // Enable Timers clock
  RCC->APBENR1 |= RCC_APBENR1_TIM3EN | RCC_APBENR1_TIM6EN | RCC_APBENR1_TIM7EN;

  // Enable USART clock
  RCC->APBENR1 |= RCC_APBENR1_USART2EN | RCC_APBENR1_USART3EN | RCC_APBENR1_USART4EN;

  // Enable SPI2 clock
  RCC->APBENR1 |= RCC_APBENR1_SPI2EN;

  // Enable I2C1 clock
  RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

  // Enable ADC clock
  RCC->APBENR2 |= RCC_APBENR2_ADCEN;

  // ADCCLK = System Clock = 64 MHz
  RCC->CCIPR &= ~RCC_CCIPR_ADCSEL;
}

void Set_zero(uint8_t* data, uint16_t data_size)
{
  for(int iterator = 0; iterator < data_size; iterator++)
  {
    *(data+iterator) = 0;
  }
}

uint8_t Check_bit(uint8_t data, uint8_t bit)
{
  if((data & 1<<bit) > 0)
    return 1;
  else
    return 0;
}

uint8_t Set_bit(uint8_t data, uint8_t bit)
{
  return (data |= 1<<bit);
}

uint8_t Reset_bit(uint8_t data, uint8_t bit)
{
  return (data &= ( 0xFF^(1<<bit) ));
}

void Tick_decrement(void)
{
  if(sysTick_decrement != 0)
    sysTick_decrement--;
  if(cfg_timeout_counter != 0)
    cfg_timeout_counter--;
  if(GPS_rx_timeout != 0x00)
    GPS_rx_timeout--;
  if(gsm_pt_timeout != 0x00)
    gsm_pt_timeout--;
  if(gsm_sr_timeout != 0x00)
    gsm_sr_timeout--;
  if(SPIF_spi_counter != 0x00)
    SPIF_spi_counter--;
  if(systick_check_IO_counter > 0x00)
  {
    systick_check_IO_counter--;
    if(systick_check_IO_counter == 0x00)
    {
      IO_Check_IOs();
      systick_check_IO_counter = SYSTICK_CHECK_IO_INTERVAL;
    }
  }
}

void Delay(uint32_t time)
{
  sysTick_decrement = time;

  while(sysTick_decrement > 0);
}