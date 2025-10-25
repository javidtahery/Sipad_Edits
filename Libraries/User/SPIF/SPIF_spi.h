#ifndef __SPIF_SPI_H
#define __SPIF_SPI_H

/* ---------------- Include ---------------- */
#include "main.h"


/* ---------------- Define ---------------- */
#define SPIF_SPIx                               SPI2
#define SPIF_DMA_RX_CHANNEL                     DMA1_Channel2
#define SPIF_DMA_TX_CHANNEL                     DMA1_Channel3

#define SPIF_RST_GPIO                           GPIOA
#define SPIF_RST_PIN                            GPIO_BSRR_BS8

#define SPIF_WP_GPIO                            GPIOC
#define SPIF_WP_PIN                             GPIO_BSRR_BS6

#define SPIF_CS_GPIO                            GPIOA
#define SPIF_CS_PIN                             GPIO_BSRR_BS9

#define SPIF_SCK_GPIO                           GPIOB
#define SPIF_SCK_PIN                            GPIO_Pin_13

#define SPIF_MISO_GPIO                          GPIOB
#define SPIF_MISO_PIN                           GPIO_Pin_14

#define SPIF_MOSI_GPIO                          GPIOB
#define SPIF_MOSI_PIN                           GPIO_Pin_15

#define SPIF_CS_LOW                             SPIF_CS_GPIO->BRR = SPIF_CS_PIN
#define SPIF_CS_HIGH                            SPIF_CS_GPIO->BSRR = SPIF_CS_PIN


#define DMA_RX                                  1
#define DMA_TX                                  2


/* ---------------- Structure ---------------- */


/* ---------------- Enum ---------------- */


/* ---------------- Extern ---------------- */
extern __IO uint16_t SPIF_spi_counter;

/* ---------------- Prototype ---------------- */
void SPIF_cfg_spi(void);
uint8_t SPIx_ReadWriteByte(uint8_t TxData);
uint8_t SPIF_read_SPIx_DMA(uint8_t* data, uint16_t data_count);
uint8_t SPIF_write_SPIx_DMA(uint8_t* data, uint16_t data_count);


#endif  /* __SPIF_SPI_H */