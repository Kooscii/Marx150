#include "stm32f10x.h"

const char * str_DEVICE[] = {"层 - 0", "层 - 1", "层 - 2","层 - 3", "层 - 4","层 - 5", "层 - 6","层 - 7", "层 - 8","层 - 9"};
const char * str_PARAM_NO[] = {"参数1","参数2","参数3","参数4","参数5"};
const char * str_TRG_STATUS[] = {"参数设定","等待触发","暂停中..","触发中..","充电中.."};
const char * str_CHG_STATUS[] = {"掉电","充电"};
const char * str_CTRL_MODE[] = {"手动","自动"};
const char * str_DEV_STATUS[] = {"关机","开机"};
const char * str_PARAM_NAME[] =	
{/* param 1*/
"输出电压      kV",
"宽度设置      us",
"延时设置      us",
/* param 2*/
"重复频率      Hz",
"重复次数      次",
"输出极性        ",
/* param 3*/
"加电步进      kV",
"步进范围      kV",
"      至      kV"};

const char * str_SEP = "电源0 1 2 3 4 5 6 7 8 9 ";
const char * str_BLANK = "                        ";
const char * str_ON[] = {"  ","开"};
const char * str_CHG[] = {"  ","充"};
const char * str_DZ = "死区    ns";
const char * str_PR = "PWM";
const char * str_CF = "截止    us";
