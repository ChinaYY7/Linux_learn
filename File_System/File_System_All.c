//"c": "cd $dir && gcc -g $fileName File_System.c -o ../../bin/$fileNameWithoutExt.out && cd $dir/../../bin && ./$fileNameWithoutExt.out",
#include "apue.h"
#include "File_System.h"
#define Path "/mnt/f/Linux_code/File_System/File_System.Fsys"

void Split_Cmd(char *Cmd, char Control[][NAME_MAX_LENGTH])
{
    int i = 0,j = 0,m = 0;
    
    Control[0][0] = '\0';
    Control[1][0] = '\0';
    Control[2][0] = '\0';
    while(1)
    {
        Control[i][j] = Cmd[m];
        if(((strcmp(Control[0],"write") != 0) || (i != 2)))
        {
            if(Control[i][j] == ' ')
            {
                Control[i][j] = '\0';
                if((strcmp(Control[0],"read") == 0) || (strcmp(Control[0],"write") == 0))
                    i++;
                else
                {
                    if((i == 0) &&(Cmd[m+1] != '-'))
                        i+=2;
                    else
                        i++;
                }
                j = -1;
            }
        }
        j++;
        m++;
        if(Cmd[m] == '\0')
        {
            Control[i][j] = Cmd[m];
            break;
        }
            
    }
}
int Chars2Num(char *str)
{
    int str_len,i,level = 1;
    int num = 0;
    str_len = strlen(str);
    for(i = str_len - 1; i >= 0; i--)
    {
        num = num + (str[i] - '0') * level;
        level *= 10;
    }
    return num;
}
int main(void)
{
    char Cmd[NAME_MAX_LENGTH] = {"read 2.c 100"};
    char Current_Dirent_Name[NAME_MAX_LENGTH];
    char CONTROL[3][NAME_MAX_LENGTH];
    int ret,Write_Num, Read_Num, Read_Num_S;
    Mem_Inode_P Mem_Inode_Info;
    char Read_Buf[4096];
    
    Open_File_System(Path);
    /*
    Get_Current_Name(Current_Dirent_Name);
    ls('u',NULL);
    Cd_S("Document1/Document2");
    Get_Current_Name(Current_Dirent_Name);
    ls('u',NULL);
    */
    while(1)
    {
        Get_Current_Name(Current_Dirent_Name);
        printf("\nYY_File_System@%s:",Current_Dirent_Name);
        gets(Cmd);
        Split_Cmd(Cmd,CONTROL);
#if DEBUG == 1
        printf("Cmd1: %s cmd2: %s cmd3: %s\n", CONTROL[0],CONTROL[1],CONTROL[2]);
#endif
        if(strcmp(CONTROL[0],"ls") == 0)
        {
            if(strcmp(CONTROL[1],"-l") == 0)
            {
                if(CONTROL[2][0] == '\0')
                    ls('u',NULL);
                else
                    ls('u',CONTROL[2]);
            }
            else if(CONTROL[1][0] == '\0')
            {
                if(CONTROL[2][0] == '\0')
                    ls('d',NULL);
                else
                    ls('d',CONTROL[2]);
            }
            else
                printf("Input Correct param\n");
        }

        else if(strcmp(CONTROL[0],"cd") == 0)
        {
            if(CONTROL[1][0] == '\0')
            {
                if(CONTROL[2][0] == '\0')
                    Cd_S("...");
                else
                    Cd_S(CONTROL[2]);
            }
            else
                printf("Input Correct param\n"); 
        }

        else if(strcmp(CONTROL[0],"create") == 0)
        {
            if((CONTROL[1][0] == '\0') && (CONTROL[2][0] != '\0'))
            {
                ret = Creat_S(CONTROL[2]);
                if(ret == 1)
                    printf("Input appropriate path name!\n");
                else if(ret == 2)
                    printf("File already exit\n");
            }
            else
                printf("Input Correct param\n"); 
        }

        else if(strcmp(CONTROL[0],"mkdir") == 0)
        {
            if((CONTROL[1][0] == '\0') && (CONTROL[2][0] != '\0'))
            {
                ret = Mkdir_S(CONTROL[2]);
                if(ret == 1)
                    printf("Input appropriate path name!\n");
                else if(ret == 2)
                    printf("File already exit\n");
            }
            else
                printf("Input Correct param\n"); 
        }

        else if(strcmp(CONTROL[0],"unlink") == 0)
        {
            if((CONTROL[1][0] == '\0') && (CONTROL[2][0] != '\0'))
            {
                ret = Unlink_S(CONTROL[2]);
                if(ret != 0)
                    printf("Unlink Failed!\n");
            }
            else
                printf("Input Correct param\n"); 
        }

        else if(strcmp(CONTROL[0],"write") == 0)
        {
            if(CONTROL[1][0] != '\0')
            {
                if(Open_S(CONTROL[1],&Mem_Inode_Info) != 0)
                    continue;
                if(CONTROL[2][0] != '\0')
                {
                    Write_Num = Write_S(Mem_Inode_Info,CONTROL[2],strlen(CONTROL[2]));
                    printf("Write %d Bytes\n",Write_Num);
                }
                else
                    printf("Input Correct param\n");
            }
            else
                printf("Input Correct param\n"); 
            Close_s(Mem_Inode_Info);
        }

        else if(strcmp(CONTROL[0],"read") == 0)
        {
            if(CONTROL[1][0] != '\0')
            {
                if(Open_S(CONTROL[1],&Mem_Inode_Info) != 0)
                    continue;
                if(CONTROL[2][0] != '\0')
                {
                    Read_Num_S = Chars2Num(CONTROL[2]);
                    Read_Num = Read_S(Mem_Inode_Info,Read_Buf,Read_Num_S);
                    Read_Buf[Read_Num_S] = '\0';
                    printf("Read %d Bytes\n",Read_Num);
                    printf("Read Buf: %s\n",Read_Buf);
                }
                else
                    printf("Input Correct param\n"); 
            }
            else
                printf("Input Corret param\n");
            Close_s(Mem_Inode_Info); 
        }

        else if(strcmp(CONTROL[0],"end") == 0)
        {
            Close_File_System();
            break;
        }
        else
        {
            printf("Wrong cmd");
        }
        
    }
    Close_File_System();
}