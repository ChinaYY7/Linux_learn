#ifndef _ICP209_CMD_H
#define _ICP209_CMD_H
#include "apue.h"
int StrtoHex(unsigned char *str_o, unsigned char *str_d);
void HextoStr(unsigned char *str_o, int len, unsigned char *str_d);
void Signal_Deal_Ctc(void);
int Send_Cmd(unsigned char *str);
int Recv_Value(unsigned char *str, int len);
void Error_Detail(void);

int Get_Radom(unsigned char *str);
int Verfiy_PIN(unsigned char *pin);
int Generate_Key_Pair(unsigned char *Public_key, unsigned char *Private_key);
int Generate_Certificate(unsigned char *Private_key);
int Get_Certificate(unsigned char *Certificate);
int Asymmetric_Verification(unsigned char *Radom, unsigned char *Signature_value);
int Set_Sym_Verification_Key(unsigned char *Key);
int Symmetric_Verification(unsigned char *Radom, unsigned char *Ciphertext);
int Set_RW_Key(unsigned char *Key, unsigned char RW);
int Set_Ciphertext_Space(unsigned char *Page_Num);
int Encryption_Write(unsigned char *address, unsigned char *date);
int Encryption_Read(unsigned char *address, int len, unsigned char *date);
int Definitely_Read(unsigned char *address, int len, unsigned char *date);
int Definitely_Write(unsigned char *address, unsigned char *date);
#endif
