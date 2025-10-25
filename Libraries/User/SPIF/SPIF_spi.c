#include "SPIF_spi.h"

__IO uint16_t SPIF_spi_counter = 0;

void SPIF_cfg_spi(void)
{
  /* ------------- RST ------------- */
  SPIF_RST_GPIO->MODER = (SPIF_RST_GPIO->MODER & (~GPIO_MODER_MODE8)) | GPIO_MODER_MODE8_0;     // Set as Output
  SPIF_RST_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;
  SPIF_RST_GPIO->PUPDR = (SPIF_RST_GPIO->PUPDR & (~GPIO_PUPDR_PUPD8)) | GPIO_PUPDR_PUPD8_0;     // Pulled Up

  SPIF_RST_GPIO->BRR = SPIF_RST_PIN;
  Delay(5);
  SPIF_RST_GPIO->BSRR = SPIF_RST_PIN;
  
  /* ------------- WP ------------- */
  SPIF_WP_GPIO->MODER = (SPIF_WP_GPIO->MODER & (~GPIO_MODER_MODE6)) | GPIO_MODER_MODE6_0;       // Set as Output
  SPIF_WP_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
  SPIF_WP_GPIO->PUPDR = (SPIF_WP_GPIO->PUPDR & (~GPIO_PUPDR_PUPD6)) | GPIO_PUPDR_PUPD6_0;       // Pulled Up
  
  SPIF_WP_GPIO->BSRR = SPIF_WP_PIN;
  
  /* ------------- CS ------------- */
  SPIF_CS_GPIO->MODER = (SPIF_CS_GPIO->MODER & (~GPIO_MODER_MODE9)) | GPIO_MODER_MODE9_0;       // Set as Output
  SPIF_CS_GPIO->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;
  SPIF_CS_GPIO->PUPDR = (SPIF_CS_GPIO->PUPDR & (~GPIO_PUPDR_PUPD9)) | GPIO_PUPDR_PUPD9_0;       // Pulled Up
  
  SPIF_CS_HIGH;
  
  /* ------------- SPI GPIO ------------- */
  SPIF_SCK_GPIO->MODER &= (~(GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15));
  SPIF_SCK_GPIO->MODER |= GPIO_MODER_MODE13_1 | GPIO_MODER_MODE14_1 | GPIO_MODER_MODE15_1;      // Set as Alternative
  SPIF_SCK_GPIO->OSPEEDR |= (GPIO_OSPEEDR_OSPEED13 | GPIO_OSPEEDR_OSPEED14 | GPIO_OSPEEDR_OSPEED15);
  SPIF_SCK_GPIO->PUPDR &= ~(GPIO_PUPDR_PUPD13 | GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD15);
  SPIF_SCK_GPIO->PUPDR |= GPIO_PUPDR_PUPD13_1 | GPIO_PUPDR_PUPD14_0 | GPIO_PUPDR_PUPD15_0;      // Pulled Down
  
  SPIF_SCK_GPIO->AFR[1] = (SPIF_SCK_GPIO->AFR[1] & 0x000FFFFF);                                 // AF0
  
  /* ------------- SPI ------------- */
  // SPI Mode 0, MSB First,fPCLK/2, Master Mode, Software NSS Control
  SPIF_SPIx->CR1 &= ~SPI_CR1_SPE;                       // Disable SPI
  SPIF_SPIx->CR1 = (SPIF_SPIx->CR1 & (~SPI_CR1_BR));
  SPIF_SPIx->CR1 |= SPI_CR1_MSTR;
  SPIF_SPIx->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;          // Disable NSS pin
  SPIF_SPIx->CR2 |= SPI_CR2_FRXTH;
  
  SPIF_SPIx->CR1 |= SPI_CR1_SPE;
}

// SPIx reads and writes a byte
// Returned value: the read byte
uint8_t SPIx_ReadWriteByte(uint8_t TxData)
{
//  if ((SPIF_SPIx->SR & SPI_SR_OVR) == SPI_SR_OVR)
//    return (uint8_t)SPIF_SPIx->DR;
//  
  SPIF_spi_counter = 500;
  while ( (SPIF_SPIx->SR & SPI_SR_TXE) != SPI_SR_TXE)             //Wait for the send area to be empty   
  {
    if(SPIF_spi_counter == 0)
    {
      return 0;
    }
  }
  *(__IO uint8_t *)&SPIF_SPIx->DR = TxData;
  
  SPIF_spi_counter = 500;
  while( (SPIF_SPIx->SR & SPI_SR_RXNE) != SPI_SR_RXNE)          //Waiting to receive a byte
  {
    if(SPIF_spi_counter == 0)
    {
      return 0;
    }
  }
  uint8_t tmp_read = *(__IO uint8_t *)&SPIF_SPIx->DR;
  
  return tmp_read;
}


uint8_t SPIF_read_SPIx_DMA(uint8_t* data, uint16_t data_count)
{
  while ( (SPIF_SPIx->SR & SPI_SR_BSY) == SPI_SR_BSY);  // Wait, SPI is busy
  
  SPIF_SPIx->CR1 &= ~SPI_CR1_SPE; 
  if((data_count % 2) == 1)
  {
    SPIF_SPIx->CR2 |= SPI_CR2_LDMATX;
    SPIF_SPIx->CR2 |= SPI_CR2_LDMARX;
  }
  else
  {
    SPIF_SPIx->CR2 &= ~SPI_CR2_LDMATX;
    SPIF_SPIx->CR2 &= ~SPI_CR2_LDMARX;
  }
  SPIF_SPIx->CR1 |= SPI_CR1_SPE;
  
  // Config TX DMA
  DMAMUX1_Channel0->CCR &= ~DMAMUX_CxCR_DMAREQ_ID;      // Clear the Request ID
  DMAMUX1_Channel0->CCR |= 19;                          // SPI2 TX ID
  
  DMA1_Channel1->CCR |= DMA_CCR_DIR;                    // Send from Memory to Peripheral
  DMA1_Channel1->CCR |= DMA_CCR_MINC;                   // Memory Increment
  DMA1_Channel1->CCR &= ~ DMA_CCR_PSIZE;                // Clear Peripheral Size (8 Bits)
  DMA1_Channel1->CCR &= ~ DMA_CCR_MSIZE;                // Clear Memory Size (8 Bits)
  DMA1_Channel1->CCR |= DMA_CCR_PL;                     // Set Priority Level to Very High
  DMA1_Channel1->CNDTR = data_count;                    // Set Channel Number of data to transmit
  DMA1_Channel1->CPAR = (__IO uint32_t )&SPIF_SPIx->DR;
  DMA1_Channel1->CMAR = (__IO uint32_t )data;
  DMA1_Channel1->CCR |= DMA_CCR_EN;                     // Enable DMA Channel 1
  
  // Config RX DMA
  DMAMUX1_Channel1->CCR &= ~DMAMUX_CxCR_DMAREQ_ID;      // Clear the Request ID
  DMAMUX1_Channel1->CCR |= 18;                          // SPI2 RX ID
  
  DMA1_Channel2->CCR &= ~DMA_CCR_DIR;                   // Send from Peripheral to Memory
  DMA1_Channel2->CCR |= DMA_CCR_MINC;                   // Memory Increment
  DMA1_Channel2->CCR &= ~ DMA_CCR_PSIZE;                // Clear Peripheral Size (8 Bits)
  DMA1_Channel2->CCR &= ~ DMA_CCR_MSIZE;                // Clear Memory Size (8 Bits)
  DMA1_Channel2->CCR |= DMA_CCR_PL;                     // Set Priority Level to Very High
  DMA1_Channel2->CNDTR = data_count;                    // Set Channel Number of data to transmit
  DMA1_Channel2->CPAR = (__IO uint32_t )&SPIF_SPIx->DR;
  DMA1_Channel2->CMAR = (__IO uint32_t )data;
  DMA1_Channel2->CCR |= DMA_CCR_EN;                     // Enable DMA Channel 2
  
  // Enable SPI DMA Request
  SPIF_SPIx->CR2 |= SPI_CR2_TXDMAEN;                    // Enable SPI TX DMA
  SPIF_SPIx->CR2 |= SPI_CR2_RXDMAEN;                    // Enable SPI TX DMA
  
  // Wait to send all dummy bytes
  SPIF_spi_counter = 1000;
  while( (DMA1->ISR & DMA_ISR_TCIF1) != DMA_ISR_TCIF1)
  {
    if(SPIF_spi_counter == 0)
      return ERROR;
  }
  // Disable TX DMA request
  SPIF_SPIx->CR2 &= ~SPI_CR2_TXDMAEN;                   // Diable SPI TX DMA
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;                    // Disable DMA Channel 1
  DMA1->IFCR |= DMA_IFCR_CTCIF1;
  
  
  // Wait to recieve all the data
  SPIF_spi_counter = 1000;
  while( (DMA1->ISR & DMA_ISR_TCIF2) != DMA_ISR_TCIF2)
  {
    if(SPIF_spi_counter == 0)
      return ERROR;
  }
  
  // Disable RX DMA request
  SPIF_SPIx->CR2 &= ~SPI_CR2_RXDMAEN;                   // Diable SPI TX DMA
  DMA1_Channel2->CCR &= ~DMA_CCR_EN;                    // Disable DMA Channel 2
  DMA1->IFCR |= DMA_IFCR_CTCIF2;
  
  // Clear the RX FIFO Buffer
  while( (SPIF_SPIx->SR & SPI_SR_FRLVL) != 0 )
     *(__IO uint8_t *)&SPIF_SPIx->DR;
  
  return SUCCESS;
}

uint8_t SPIF_write_SPIx_DMA(uint8_t* data, uint16_t data_count)
{
  while ( (SPIF_SPIx->SR & SPI_SR_BSY) == SPI_SR_BSY);  // Wait, SPI is busy
  
  SPIF_SPIx->CR1 &= ~SPI_CR1_SPE; 
  if((data_count % 2) == 1)
    SPIF_SPIx->CR2 |= SPI_CR2_LDMATX;
  else
    SPIF_SPIx->CR2 &= ~SPI_CR2_LDMATX;
  SPIF_SPIx->CR1 |= SPI_CR1_SPE;
  
  // Config TX DMA
  DMAMUX1_Channel0->CCR &= ~DMAMUX_CxCR_DMAREQ_ID;      // Clear the Request ID
  DMAMUX1_Channel0->CCR |= 19;                          // SPI2 TX ID
  
  DMA1_Channel1->CCR |= DMA_CCR_DIR;                    // Send from Memory to Peripheral
  DMA1_Channel1->CCR |= DMA_CCR_MINC;                   // Memory Increment
  DMA1_Channel1->CCR &= ~ DMA_CCR_PSIZE;                // Clear Peripheral Size (8 Bits)
  DMA1_Channel1->CCR &= ~ DMA_CCR_MSIZE;                // Clear Memory Size (8 Bits)
  DMA1_Channel1->CCR |= DMA_CCR_PL;                     // Set Priority Level to Very High
  DMA1_Channel1->CNDTR = data_count;                    // Set Channel Number of data to transmit
  DMA1_Channel1->CPAR = (__IO uint32_t )&SPIF_SPIx->DR;
  DMA1_Channel1->CMAR = (__IO uint32_t )data;
  DMA1_Channel1->CCR |= DMA_CCR_EN;                     // Enable DMA Channel 1
  
  SPIF_SPIx->CR2 |= SPI_CR2_TXDMAEN;                    // Enable SPI TX DMA
   
  // Wait to send all data
  SPIF_spi_counter = 1000;
  while( (DMA1->ISR & DMA_ISR_TCIF1) != DMA_ISR_TCIF1)
  {
    if(SPIF_spi_counter == 0)
      return ERROR;
  }
  
  // Disable TX DMA request
  SPIF_SPIx->CR2 &= ~SPI_CR2_TXDMAEN;                   // Diable SPI TX DMA
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;                    // Disable DMA Channel 1
  DMA1->IFCR |= DMA_IFCR_CTCIF1;
  
  // Clear the RX FIFO Buffer
  while( (SPIF_SPIx->SR & SPI_SR_FRLVL) != 0 )
     *(__IO uint8_t *)&SPIF_SPIx->DR;
  
  return SUCCESS;
}