#include "usb_hub.h"
#include "Deal_Error.h"
#include "Panel.h"
#include "ICP209_cmd.h"
#define Root_Public_key "df630125b23a3a01c8d6e4a0a0617b7e06c51b673f4970a3300baec510c1b8cc28a85614a90d0bc015caeeb1780d9cad"
#define Root_Private_key "4b3ccfb1b53bb955ee484f22ee40fa9d25e539b479ab5c1f"
#define Rodom_V "bd3e4dbb553342839e4d4f9c1e0e234d"
#define Key "11223344556677881122334455667700"
#define Ciphertext_Space "08"
#define Encryption_Addr "0001"
#define Encryption_Date "0102030405060708090a0b0c0e0f"
#define Definitely_Addr "0001"
#define Definitely_Date "0102030405060708090a0b0c0e0f"

int main(void)
{
    int ret; 	

    //初始化USB
    Init_USB();

    //ctrl+c 信号处理
    Signal_Deal_Ctc();
    
    //开启热插拔
    Register_Hotplug();

    //检测特定设备是否连接
    printf("Wait connecting......\n");
    ret = Find_device();
    if(ret < 0) 
    {			
        if(ret == -1)
            System_Error("*** Find device failed!");
        else
            printf("*** Not Find Device\n");
    }
    else if(ret == 0)
        Display_Device_Info();
    
    uint8_t Display_Sta = 1;
    int Verfiy_PIN_Sta;
    int Get_Radom_Sta;
    int Generate_Key_Sta;
    int Generate_Certificate_Sta;
    int Get_Certificate_Sta;
    int Asymmetric_Verification_Sta;
    int Set_Sym_Verification_Key_Sta;
    int Symmetric_Verification_Sta;
    int Set_RW_Key_Sta;
    int Set_Ciphertext_Space_Sta;
    int Encryption_Write_Sta;
    int Encryption_Read_Sta;
    int Definitely_read_Sta;
    int Definitely_Write_Sta;
    unsigned char Radom[17], Radom_Str[34];
    unsigned char Public_key[48],Private_key[24],Public_key_Str[97],Private_key_Str[49];
    unsigned char Certificate[113], Certificate_Str[226];
    unsigned char Signature_value[49], Signature_value_Str[99];
    unsigned char Ciphertext[17], Ciphertext_Str[34];
    unsigned char Encryption_Read_Date[128], Encryption_Read_Date_Str[256];
    unsigned char Definitely_Read_Date[128], Definitely_Read_Date_Str[256];
    unsigned char *PIN = "e1c9aaa518389fd0";

    int cmd;

    while(1)
    {
        if(!Detect_Device_Connect_sta())
            Display_Sta = 1;
        
        if(Detect_Device_Connect_sta())
        {
            if(Display_Sta)
            {
                Display_Panel();
                Display_Sta = 0;
            }
            printf("Input cmd(cmd:20 clear):");
            scanf("%d", &cmd);
            if(cmd == 0)
                break;
            switch(cmd)
            {
                case 1:
                {
                    Command_Mode();
                    Display_Sta = 1;
                    break;
                }
                case 2:
                    Verfiy_PIN_Sta = Verfiy_PIN(PIN);
                    if(Verfiy_PIN_Sta < 0)
                        printf("PIN(%s):验证失败！\n", PIN);
                    else
                        printf("PIN(%s):验证成功！\n", PIN);
                    break;
                case 3:
                    Get_Radom_Sta = Get_Radom(Radom);
                    if(Get_Radom_Sta < 0)
                        printf("获取随机数失败\n");
                    else
                    {
                        HextoStr(Radom, Get_Radom_Sta, Radom_Str);
                        printf("随机数为(%d bytes): %s\n", Get_Radom_Sta, Radom_Str);
                    }
                    break;
                case 4:
                    Generate_Key_Sta = Generate_Key_Pair(Public_key, Private_key);
                    if(Generate_Key_Sta < 0)
                        printf("生成公私钥对失败\n");
                    else
                    {
                        HextoStr(Public_key, 48, Public_key_Str);
                        HextoStr(Private_key, 24, Private_key_Str);
                        printf("公钥为(%ld bytes): %s\n", strlen(Public_key_Str) / 2, Public_key_Str);
                        printf("私钥为(%ld bytes): %s\n", strlen(Private_key_Str) / 2, Private_key_Str);
                    }
                    break;
                case 5:
                    Generate_Certificate_Sta = Generate_Certificate(Root_Private_key);
                    if(Generate_Certificate_Sta < 0)
                        printf("生成证书，认证公私钥对失败！\n");
                    else
                        printf("生成证书，认证公私钥对成功！\n");
                    break;
                case 6:
                    Get_Certificate_Sta = Get_Certificate(Certificate);
                    if(Get_Certificate_Sta < 0)
                        printf("读取证书失败\n");
                    else
                    {
                        HextoStr(Certificate, Get_Certificate_Sta, Certificate_Str);
                        printf("证书为(%d bytes): %s\n", Get_Certificate_Sta, Certificate_Str);
                    }
                    break;
                case 7:
                    Asymmetric_Verification_Sta = Asymmetric_Verification(Rodom_V,Signature_value);
                    if(Asymmetric_Verification_Sta < 0)
                        printf("读取签名值失败\n");
                    else
                    {
                        HextoStr(Signature_value, Asymmetric_Verification_Sta, Signature_value_Str);
                        printf("非对称认证签名值为(%d bytes): %s\n", Asymmetric_Verification_Sta, Signature_value_Str);
                    }
                    break;
                case 8:
                    Set_Sym_Verification_Key_Sta = Set_Sym_Verification_Key(Key);
                    if(Set_Sym_Verification_Key_Sta < 0)
                        printf("设置对称密钥失败！\n");
                    else
                        printf("设置对称密钥成功！\n");
                    break;
                case 9:
                    Symmetric_Verification_Sta = Symmetric_Verification(Rodom_V,Ciphertext);
                    if(Symmetric_Verification_Sta < 0)
                        printf("读取对称认证密文失败\n");
                    else
                    {
                        HextoStr(Ciphertext, Symmetric_Verification_Sta, Ciphertext_Str);
                        printf("对称认证密文为(%d bytes): %s\n", Symmetric_Verification_Sta, Ciphertext_Str);
                    }
                    break;
                case 10:
                    Set_RW_Key_Sta = Set_RW_Key(Key,0);
                    if(Set_RW_Key_Sta < 0)
                        printf("设置写密钥失败！\n");
                    else
                        printf("设置写密钥成功！\n");
        
                    Set_RW_Key_Sta = Set_RW_Key(Key,1);
                    if(Set_RW_Key_Sta < 0)
                        printf("设置读密钥失败！\n");
                    else
                        printf("设置读密钥成功！\n");
                    break;
                case 11:
                    Set_Ciphertext_Space_Sta = Set_Ciphertext_Space(Ciphertext_Space);
                    if(Set_Ciphertext_Space_Sta < 0)
                        printf("设置密文空间失败！\n");
                    else
                        printf("设置密文空间成功！\n");
                    break;
                case 12:
                    Encryption_Read_Sta = Encryption_Read(Encryption_Addr, 14, Encryption_Read_Date);
                    if(Encryption_Read_Sta < 0)
                    {
                        printf("读取密文空间失败\n");
                        Error_Detail();
                    }
                    else
                    {
                        HextoStr(Encryption_Read_Date, Encryption_Read_Sta, Encryption_Read_Date_Str);
                        printf("读取的密文为(%d bytes): %s\n", Encryption_Read_Sta, Encryption_Read_Date_Str);
                    }
                    break;
                case 13:
                    Encryption_Write_Sta = Encryption_Write(Encryption_Addr,Encryption_Date);
                    if(Encryption_Write_Sta < 0)
                    {
                        printf("写入密文空间失败！\n");
                        Error_Detail();
                    }
                    else
                        printf("写入密文空间成功！\n");
                    break;
                case 14:
                    Definitely_read_Sta = Definitely_Read(Definitely_Addr, 14, Definitely_Read_Date);
                    if(Definitely_read_Sta < 0)
                    {
                        printf("读取明文空间失败\n");
                        Error_Detail();
                    }
                    else
                    {
                        HextoStr(Definitely_Read_Date, Definitely_read_Sta, Definitely_Read_Date_Str);
                        printf("读取的明文为(%d bytes): %s\n", Definitely_read_Sta, Definitely_Read_Date_Str);
                    }
                    break;
                case 15:
                    Definitely_Write_Sta = Definitely_Write(Definitely_Addr,Definitely_Date);
                    if(Definitely_Write_Sta < 0)
                    {
                        printf("写入明文空间失败！\n");
                        Error_Detail();
                    }
                    else
                        printf("写入明文空间成功！\n");
                    break;
                case 20:
                    system("clear");
                    Display_Sta = 1;
                    break;
                default:
                    printf("cmd is wrong\n");
            }
        }
    }
    
    Close_USB();
    printf("Exit !!!\n"); 
}
