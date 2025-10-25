#include "GSM_BT.h"

void BT_Enable(void)
{
  GSM_Send_Usart("AT+QBTPWR=1\r", strlen("AT+QBTPWR=1\r"));
}

void BT_Set_BT_Name(void)
{
  GSM_Send_Usart("AT+QBTNAME=\"SRC_Sipaad_BT\"\r", strlen("AT+QBTNAME=\"SRC_Sipaad_BT\"\r"));
}