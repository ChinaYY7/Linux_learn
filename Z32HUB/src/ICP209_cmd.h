#ifndef _ICP209_CMD_H
#define _ICP209_CMD_H
#include "apue.h"
void StrToByte_stream(unsigned char *str_o, unsigned char *str_d);
void Byte_streamToStr(unsigned char *str_o, unsigned char *str_d);
void Signal_Deal_Ctc(void);
int Send_Cmd(unsigned char *str, int len);
int Recv_Value(unsigned char *str, int len);
int Get_Cmd_Sta(unsigned char *Recv_str);

int Get_Radom(unsigned char *str);
int Verfiy_PIN(unsigned char *pin);
#endif
