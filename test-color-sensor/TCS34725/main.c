#include <stdio.h>      //printf()
#include <stdlib.h>     //exit()
#include <signal.h>

#include "I2CDriver.h"
#include "TCS34725.h"

uint16_t r,g,b,c;
uint16_t cpl,lux,k;

void  Handler(int signo)
{
    //System Exit
    printf("\r\nHandler:Program stop\r\n"); 
    I2C_CloseAllDevices();    
    exit(0);
}

int main(int argc, char **argv)
{
    RGB rgb;
    uint32_t RGB888=0;
    uint16_t   RGB565=0;
	
    
    // Exception handling:ctrl + c
    signal(SIGINT, Handler);
    
    if(TCS34725_Init() != 0){
        printf("TCS34725 initialization error!!\r\n");
        exit(0);
    } 
    printf("TCS34725 initialization success!!\r\n");
    
    // TCS34725_SetLight(60);
    
    while(1){    
        rgb=TCS34725_Get_RGBData();
        RGB888=TCS34725_GetRGB888(rgb);
        RGB565=TCS34725_GetRGB565(rgb);
        printf(" RGB888 :R=%d   G=%d  B=%d   RGB888=0X%X  RGB565=0X%X  \n", (RGB888>>16), \
                (RGB888>>8) & 0xff, (RGB888) & 0xff, RGB888, RGB565);
	}

    return 0; 
}