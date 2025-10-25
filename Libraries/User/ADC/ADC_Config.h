#ifndef __ADC_CONFIG_H
#define __ADC_CONFIG_H

/* ---------------- Include ---------------- */
#include "main.h"

/* ---------------- Define ---------------- */


#define MAX_ADC_ITERATION_COUNT                 25
#define MAIN_VOLTAGE_BIAS_VOLTAGE               900     //mA
#define MAIN_POWER_R_LOW                        1.2
#define MAIN_POWER_R_HIGH                       12

#define BATTERY_R_LOW                           56
#define BATTERY_R_HIGH                          18

#define DEADZONE_THRESHOLD                      3700    // mVolt
#define VREFINT                                 3.00

#define VREFINT_CAL                             *((uint16_t*)(0x1FFF75AAUL))
#define TS_CAL1                                 *((uint16_t*)(0x1FFF75A8UL))    // Temprature Calibration 1
#define TS_CAL2                                 *((uint16_t*)(0x1FFF75CAUL))    // Temprature Calibration 2
#define AVG_SLOPE                               (float)((TS_CAL2 - TS_CAL1)) 

#define TS_CAL1_TEMP                            30
#define TS_CAL2_TEMP                            110

/* ---------------- Enum ---------------- */


/* ---------------- Structure ---------------- */
typedef struct{
  uint16_t Main_power;
  uint16_t Temperture;
  uint16_t vref;
  uint16_t Internal_Battery;
  
 float Average_Internal_BAT;
 float Average_Main_Power_Voltage;
 float Vref_voltage;
 uint16_t Average_Temperture;
}analog_typedef;


/* ---------------- Extern ---------------- */
extern analog_typedef   ADC_Values;
/* ---------------- Prototype ---------------- */
void ADC_Config(void);
void DMA_Config_for_ADC(void);
void ADC_calculate_average(void);


#endif /* __ADC_CONFIG_H */