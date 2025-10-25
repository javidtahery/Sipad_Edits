#include "accel_i2c.h"


void Accel_I2C_config(void)
{
  /* ----------------- I2C GPIO Config ----------------- */ 
  ACCEL_SCL_GPIO->MODER &= (~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7));
  ACCEL_SCL_GPIO->MODER |= GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;             // Set as Alternative
  ACCEL_SCL_GPIO->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
  ACCEL_SCL_GPIO->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;                  // Open Drain
  
  ACCEL_SCL_GPIO->AFR[0] = (ACCEL_SCL_GPIO->AFR[0] & 0x00FFFFFF) | 0x66000000;          // Alternative Function 6
  
  /* ----------------- Interrupt 1 Config ----------------- */ 
  ACCEL_INT1_GPIO->MODER = (ACCEL_INT1_GPIO->MODER & (~GPIO_MODER_MODE8));                                  // Set as Input
  ACCEL_INT1_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;
  ACCEL_INT1_GPIO->PUPDR = (ACCEL_INT1_GPIO->PUPDR & (~GPIO_PUPDR_PUPD8)) | GPIO_PUPDR_PUPD8_1;            // Pulled Down
  
  EXTI->FTSR1 |= EXTI_RTSR1_RT8;                                                // Raising trigger cfg
  EXTI->FPR1 |= EXTI_RPR1_RPIF8;                                                // Clear Raising pending bit
  EXTI->EXTICR[2] = (EXTI->EXTICR[2] & (~EXTI_EXTICR3_EXTI8));                  // Port Source: GPIOA
  EXTI->IMR1 |= EXTI_IMR1_IM8;                                                  // Enable EXTI Interrupt
  
  //  NVIC_EnableIRQ(EXTI4_15_IRQn);                                                // Enable 1PPS NVIC
  //  NVIC_SetPriority(EXTI4_15_IRQn, 2);                                           // Set Priority
  //  
  //  NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
  
  ACELL_INT1_DISABLE_EXTI;
  
  
  /* ----------------- I2C Independent Clock Config ----------------- */ 
  //  RCC->CCIPR = (RCC->CCIPR & (~RCC_CCIPR_I2C1SEL)) | RCC_CCIPR_I2C1SEL_0;       // Sysclk as I2C1 Clock
  
  /* ----------------- I2C Config ----------------- */
  //  By default, an analog noise filter is present on the SDA and SCL inputs.
  // Digital filer is disabled and stretch is on.
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;                                                // Disable I2C
  ACCEL_I2C->TIMINGR = (uint32_t)0x006A2297;                                    // 300KHz, Rise Time = 0ns, Fall Time = 250ns
  
  //  ACCEL_I2C->CR1 |= I2C_CR1_PE;                                               // Enable I2C
  
  /* ----------------- I2C DMA Config ----------------- */
  //  DMA_DeInit(ACCEL_I2C_DMA_RX_CHANNEL);
  //  I2C_DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ACCEL_I2C->DR;
  //  I2C_DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)0;
  //  I2C_DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  //  I2C_DMA_InitStruct.DMA_BufferSize = 0x0000;
  //  I2C_DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //  I2C_DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //  I2C_DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  //  I2C_DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  //  I2C_DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
  //  I2C_DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  //  I2C_DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
  //  DMA_Init(ACCEL_I2C_DMA_RX_CHANNEL, &I2C_DMA_InitStruct);
}


void I2C_Software_reset(void)
{
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  if( (ACCEL_I2C->CR1 & I2C_CR1_PE) != I2C_CR1_PE )
    ACCEL_I2C->CR1 |= I2C_CR1_PE;
}


void SCL_Delay(void)
{
  for(uint8_t i = 0; i < 20; i++)
    asm("nop");
}

// This function will reset the device I2C locked up and reconfigure the I2C
void I2C_generate_stop_condition_manually(void)
{
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  //  DMA_DeInit(ACCEL_I2C_DMA_RX_CHANNEL);
  
  /* ----------------- GPIO Config ----------------- */
  ACCEL_SCL_GPIO->MODER &= (~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7));
  ACCEL_SCL_GPIO->MODER |= GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0;            // Set as Output
  ACCEL_SCL_GPIO->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
  ACCEL_SCL_GPIO->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;  
  
  ACCEL_SCL_GPIO->BRR = ACCEL_SCL_PIN;                                          // Reset SCL
  ACCEL_SDA_GPIO->BRR = ACCEL_SDA_PIN;                                          // Reset SDA
  
  ACCEL_SCL_GPIO->BSRR = ACCEL_SCL_PIN;                                         // Set SCL
  SCL_Delay();
  ACCEL_SDA_GPIO->BSRR = ACCEL_SDA_PIN;                                         // Set SDA
  
  Accel_I2C_config();
  Delay(5);
}


uint16_t Accel_read_byte(uint8_t* read_data, uint8_t reg)
{
  uint16_t timeout = I2C_BUS_TMIEOUT_TIME;
  
  while( (ACCEL_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  /* _-_-_-_-_-_-_-_ Initial Write _-_-_-_-_-_-_-_ */
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  
  ACCEL_I2C->CR2 &= ~I2C_CR2_SADD;												// Clear the Salve Address
  ACCEL_I2C->CR2 |= ACCEL_I2C_ADDR;												// Program Slave Address
  ACCEL_I2C->CR2 &= ~I2C_CR2_RD_WRN;                                            // Write transfer (sending the register address)
  ACCEL_I2C->CR2 = (ACCEL_I2C->CR2 & ~I2C_CR2_NBYTES) | (I2C_CR2_NBYTES & (1 << I2C_CR2_NBYTES_Pos));
  ACCEL_I2C->CR2 &= ~I2C_CR2_AUTOEND;                                           // Software End mode selected
  
  // Enable I2C
  ACCEL_I2C->CR1 |= I2C_CR1_PE;
  
  // Generate Start
  ACCEL_I2C->CR2 |= I2C_CR2_START;
  
  // Wait for sending the slave address
  while( (ACCEL_I2C->CR2 & I2C_CR2_START) == I2C_CR2_START );
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Send Register address
  *(__IO uint8_t *)&ACCEL_I2C->TXDR = reg;
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Initial Read
  ACCEL_I2C->CR2 |= ACCEL_I2C_ADDR;
  ACCEL_I2C->CR2 = (ACCEL_I2C->CR2 & ~I2C_CR2_NBYTES) | (I2C_CR2_NBYTES & (1 << I2C_CR2_NBYTES_Pos));
  // This will generate stop
  // ACCEL_I2C->CR2 |= I2C_CR2_AUTOEND;                                            // Auto End mode selected
  ACCEL_I2C->CR2 |= I2C_CR2_RD_WRN;                                             // Read transfer
  
  // Generate Start
  ACCEL_I2C->CR2 |= I2C_CR2_START;
  
  // Wait for sending the slave address
  while( (ACCEL_I2C->CR2 & I2C_CR2_START) == I2C_CR2_START );
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Read Data
  *read_data = *(__IO uint8_t*)&ACCEL_I2C->RXDR;
  
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Generate Stop
  ACCEL_I2C->CR2 |= I2C_CR2_AUTOEND;                                                 // Enable Auto end
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  
  return I2C_BUS_OK;
}


// Read less than 256 bytes
uint16_t Accel_read_nbytes(uint8_t* read_data, uint8_t nbytes, uint8_t reg)
{
  uint16_t timeout = I2C_BUS_TMIEOUT_TIME;
  
  while( (ACCEL_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  
  // Initial Write
  ACCEL_I2C->CR2 &= ~I2C_CR2_SADD;
  ACCEL_I2C->CR2 |= ACCEL_I2C_ADDR;
  ACCEL_I2C->CR2 &= ~I2C_CR2_RD_WRN;                                            // Write transfer
  ACCEL_I2C->CR2 = (ACCEL_I2C->CR2 & ~I2C_CR2_NBYTES) | (I2C_CR2_NBYTES & (1 << I2C_CR2_NBYTES_Pos));
  ACCEL_I2C->CR2 &= ~I2C_CR2_AUTOEND;                                           // Software End mode selected
  
  // Enable I2C
  ACCEL_I2C->CR1 |= I2C_CR1_PE;
  
  // Generate Start
  ACCEL_I2C->CR2 |= I2C_CR2_START;
  
  // Wait for sending the slave address
  while( (ACCEL_I2C->CR2 & I2C_CR2_START) == I2C_CR2_START );
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Send Register address
  *(__IO uint8_t *)&ACCEL_I2C->TXDR = reg;
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Initial Read
  ACCEL_I2C->CR2 |= ACCEL_I2C_ADDR;
  ACCEL_I2C->CR2 = (ACCEL_I2C->CR2 & ~I2C_CR2_NBYTES) | (I2C_CR2_NBYTES & (nbytes << I2C_CR2_NBYTES_Pos));
  // This will generate stop
  // ACCEL_I2C->CR2 |= I2C_CR2_AUTOEND;                                            // Auto End mode selected
  ACCEL_I2C->CR2 |= I2C_CR2_RD_WRN;                                             // Read transfer
  
  // Generate Start
  ACCEL_I2C->CR2 |= I2C_CR2_START;
  
  // Wait for sending the slave address
  while( (ACCEL_I2C->CR2 & I2C_CR2_START) == I2C_CR2_START );
  
  uint8_t data_count = 0;
  while(data_count != nbytes)
  {
    timeout = I2C_BUS_TMIEOUT_TIME;
    while( (ACCEL_I2C->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE )
    {
      timeout--;
      if(timeout == 0)
      {
        *read_data = 0;
        
        ACCEL_I2C->CR2 |= I2C_CR2_STOP;
        return I2C_BUS_FAULT;
      }
    }
    
    // Read Data
    *(read_data+data_count) = *(__IO uint8_t *)&ACCEL_I2C->RXDR;
    data_count++;
  }
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TC) != I2C_ISR_TC )
  {
    timeout--;
    if(timeout == 0)
    {
      *read_data = 0;
      
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Generate Stop
  ACCEL_I2C->CR2 |= I2C_CR2_AUTOEND;                                                 // Enable Auto end
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  asm("nop");
  
  return I2C_BUS_OK;
}

uint16_t Accel_read_n_bytes_DMA(uint8_t* read_data, uint8_t nbytes, uint8_t reg)
{
  //  uint16_t timeout = I2C_BUS_TMIEOUT;
  //  
  //  while(I2C_GetFlagStatus(ACCEL_I2C, I2C_FLAG_BUSY) == SET)
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  // DMA cfg
  //  I2C_DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)read_data;
  //  I2C_DMA_InitStruct.DMA_BufferSize = nbytes;
  //  DMA_DeInit(ACCEL_I2C_DMA_RX_CHANNEL);
  //  DMA_Init(ACCEL_I2C_DMA_RX_CHANNEL, &I2C_DMA_InitStruct);
  //  I2C_DMALastTransferCmd(ACCEL_I2C, ENABLE);
  //  
  //  I2C_AcknowledgeConfig(ACCEL_I2C, ENABLE);
  //  I2C_GenerateSTART(ACCEL_I2C, ENABLE);
  //  
  //  timeout = I2C_BUS_TMIEOUT;
  //  while(!I2C_CheckEvent(ACCEL_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  I2C_Send7bitAddress(ACCEL_I2C, I2C_ADDR, I2C_Direction_Transmitter);
  //  timeout = I2C_BUS_TMIEOUT;
  //  while(!I2C_CheckEvent(ACCEL_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  I2C_SendData(ACCEL_I2C, reg);
  //  timeout = I2C_BUS_TMIEOUT;
  //  while((!I2C_GetFlagStatus(ACCEL_I2C,I2C_FLAG_TXE) && (!I2C_GetFlagStatus(ACCEL_I2C, I2C_FLAG_BTF))) )
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  I2C_GenerateSTART(ACCEL_I2C, ENABLE);
  //  
  //  timeout = I2C_BUS_TMIEOUT;
  //  while(!I2C_CheckEvent(ACCEL_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  I2C_Send7bitAddress(ACCEL_I2C, I2C_ADDR, I2C_Direction_Receiver);
  //  
  //  timeout = I2C_BUS_TMIEOUT;
  //  while(!I2C_CheckEvent(ACCEL_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
  //  {
  //    timeout--;
  //    if(timeout == 0)
  //    {
  //      *read_data = 0;
  //      
  //      return I2C_BUS_FAULT;
  //    }
  //  }
  //  
  //  I2C_DMACmd(ACCEL_I2C, ENABLE);
  //  
  //  DMA_Cmd(ACCEL_I2C_DMA_RX_CHANNEL, ENABLE);
  //  
  //  while(DMA_GetFlagStatus(ACCEL_I2C_DMA_TC_FLAG) == RESET);
  //  DMA_ClearITPendingBit(ACCEL_I2C_DMA_TC_FLAG);
  //  DMA_Cmd(ACCEL_I2C_DMA_RX_CHANNEL, DISABLE);
  //  I2C_DMACmd(ACCEL_I2C, DISABLE);
  //  I2C_GenerateSTOP(ACCEL_I2C, ENABLE);
  //  
  return I2C_BUS_OK;
}

uint16_t Accel_write_byte(uint8_t data, uint8_t reg)
{
  uint16_t timeout = I2C_BUS_TMIEOUT_TIME;
  
  while( (ACCEL_I2C->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY )
  {
    timeout--;
    if(timeout == 0)
    {
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  
  // Initial Write
  ACCEL_I2C->CR2 &= ~I2C_CR2_SADD;
  ACCEL_I2C->CR2 |= ACCEL_I2C_ADDR;
  ACCEL_I2C->CR2 = (ACCEL_I2C->CR2 & ~I2C_CR2_NBYTES) | (I2C_CR2_NBYTES & (2 << I2C_CR2_NBYTES_Pos));
  ACCEL_I2C->CR2 |= I2C_CR2_AUTOEND;                                            // Software End mode selected
  ACCEL_I2C->CR2 &= ~I2C_CR2_RD_WRN;                                            // Write transfer
  
  // Enable I2C
  ACCEL_I2C->CR1 |= I2C_CR1_PE;
  
  // Generate Start
  ACCEL_I2C->CR2 |= I2C_CR2_START;
  
  // Wait for sending the slave address
  while( (ACCEL_I2C->CR2 & I2C_CR2_START) == I2C_CR2_START );
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS )
  {
    timeout--;
    if(timeout == 0)
    {
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Send Register address
  *(__IO uint8_t *)&ACCEL_I2C->TXDR = reg;
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS )
  {
    timeout--;
    if(timeout == 0)
    {
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Send Data
  *(__IO uint8_t *)&ACCEL_I2C->TXDR = data;
  
  timeout = I2C_BUS_TMIEOUT_TIME;
  while( (ACCEL_I2C->ISR & I2C_ISR_STOPF) != I2C_ISR_STOPF )
  {
    timeout--;
    if(timeout == 0)
    {
      ACCEL_I2C->CR2 |= I2C_CR2_STOP;
      return I2C_BUS_FAULT;
    }
  }
  
  // Disable I2C
  ACCEL_I2C->CR1 &= ~I2C_CR1_PE;
  asm("nop");
  
  return I2C_BUS_OK;
}