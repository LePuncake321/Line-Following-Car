#include "I2CDriver.h"
#include "TCS34725.h"
#include <time.h>

TCS34725IntegrationTime_t IntegrationTime_t = TCS34725_INTEGRATIONTIME_700MS;
TCS34725Gain_t Gain_t = TCS34725_GAIN_60X;

static void TCS34725_WriteByte(uint8_t add, uint8_t data) {
    add = add | TCS34725_CMD_BIT;
    I2C_WriteByte(2, add, data);
}

static uint8_t TCS34725_ReadByte(uint8_t add) {
    add = add | TCS34725_CMD_BIT;
    return I2C_ReadByte(2, add);
}

static uint16_t TCS34725_ReadWord(uint8_t add) {
    add = add | TCS34725_CMD_BIT;
    return I2C_ReadWord(2, add);
}

static void TCS34725_Enable(void) {
    TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON);
    usleep(3000);
    TCS34725_WriteByte(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    usleep(3000);
}

void TCS34725_Disable(void) {
    uint8_t reg = 0;
    reg = TCS34725_ReadByte(TCS34725_ENABLE);
    TCS34725_WriteByte(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}

void TCS34725_Set_Integration_Time(TCS34725IntegrationTime_t time) {
    TCS34725_WriteByte(TCS34725_ATIME, time);
    IntegrationTime_t = time;
}

void TCS34725_Set_Gain(TCS34725Gain_t gain) {
    TCS34725_WriteByte(TCS34725_CONTROL, gain);
    Gain_t = gain;
}

static void TCS34725_Interrupt_Enable() {
    uint8_t data = 0;
    data = TCS34725_ReadByte(TCS34725_ENABLE);
    TCS34725_WriteByte(TCS34725_ENABLE, data | TCS34725_ENABLE_AIEN);
}

void TCS34725_Interrupt_Disable() {
    uint8_t data = 0;
    data = TCS34725_ReadByte(TCS34725_ENABLE);
    TCS34725_WriteByte(TCS34725_ENABLE, data & (~TCS34725_ENABLE_AIEN));
}

static void TCS34725_Set_Interrupt_Persistence_Reg(uint8_t TCS34725_PER) {
    if (TCS34725_PER < 0x10)
        TCS34725_WriteByte(TCS34725_PERS, TCS34725_PER);
    else 
        TCS34725_WriteByte(TCS34725_PERS, TCS34725_PERS_60_CYCLE);
}

static void TCS34725_Set_Interrupt_Threshold(uint16_t Threshold_H, uint16_t Threshold_L) {
    TCS34725_WriteByte(TCS34725_AILTL, Threshold_L & 0xff);
    TCS34725_WriteByte(TCS34725_AILTH, Threshold_L >> 8);
    TCS34725_WriteByte(TCS34725_AIHTL, Threshold_H & 0xff);
    TCS34725_WriteByte(TCS34725_AIHTH, Threshold_H >> 8);
}

// static void TCS34725_Clear_Interrupt_Flag() {
//     TCS34725_WriteByte(TCS34725_CMD_Clear_INT, 0x00);
// }

uint8_t TCS34725_Init(void) {
    uint8_t ID = 0;
    I2C_OpenDevice(2, TCS34725_ADDRESS);
    ID = TCS34725_ReadByte(TCS34725_ID);
    if (ID != 0x44 && ID != 0x4D) {
        return 1;
    }

    TCS34725_Set_Integration_Time(TCS34725_INTEGRATIONTIME_154MS);
    TCS34725_Set_Gain(TCS34725_GAIN_60X);

    IntegrationTime_t = TCS34725_INTEGRATIONTIME_154MS;
    Gain_t = TCS34725_GAIN_60X;

    TCS34725_Set_Interrupt_Threshold(0xff00, 0x00ff);  // Set interrupt thresholds
    TCS34725_Set_Interrupt_Persistence_Reg(TCS34725_PERS_2_CYCLE);
    TCS34725_Enable();
    TCS34725_Interrupt_Enable();
    
    // TCS34725_SetLight(40);
    
    return 0;
}

RGB TCS34725_Get_RGBData() {
    RGB temp;
    temp.C = TCS34725_ReadWord(TCS34725_CDATAL | TCS34725_CMD_Read_Word);
    temp.R = TCS34725_ReadWord(TCS34725_RDATAL | TCS34725_CMD_Read_Word);
    temp.G = TCS34725_ReadWord(TCS34725_GDATAL | TCS34725_CMD_Read_Word);
    temp.B = TCS34725_ReadWord(TCS34725_BDATAL | TCS34725_CMD_Read_Word);

    switch (IntegrationTime_t) {
        case TCS34725_INTEGRATIONTIME_2_4MS:
            usleep(3000);  // Use usleep for delay (3 ms)
            break;
        case TCS34725_INTEGRATIONTIME_24MS:
            usleep(24000);  // Use usleep for delay (24 ms)
            break;
        case TCS34725_INTEGRATIONTIME_50MS:
            usleep(50000);  // Use usleep for delay (50 ms)
            break;
        case TCS34725_INTEGRATIONTIME_101MS:
            usleep(101000);  // Use usleep for delay (101 ms)
            break;
        case TCS34725_INTEGRATIONTIME_154MS:
            usleep(154000);  // Use usleep for delay (154 ms)
            break;
        case TCS34725_INTEGRATIONTIME_700MS:
            usleep(700000);  // Use usleep for delay (700 ms)
            break;
    }

    return temp;
}

// uint8_t TCS34725_GetLux_Interrupt(uint16_t Threshold_H, uint16_t Threshold_L) {
//     TCS34725_Set_Interrupt_Threshold(Threshold_H, Threshold_L);
//     if (DEV_Digital_Read(INT_PIN) == 0) {
//         TCS34725_Clear_Interrupt_Flag();
//         TCS34725_Set_Interrupt_Persistence_Reg(TCS34725_PERS_2_CYCLE);
//         return 1;
//     }
//     return 0;
// }

uint16_t TCS34725_Get_ColorTemp(RGB rgb) {
    float cct;
    uint16_t r_comp, b_comp, ir;
    ir = (rgb.R + rgb.G + rgb.B > rgb.C) ? (rgb.R + rgb.G + rgb.B - rgb.C) / 2 : 0;
    r_comp = rgb.R - ir;
    b_comp = rgb.B - ir;
    cct = TCS34725_CT_Coef * (float)(b_comp) / (float)(r_comp) + TCS34725_CT_Offset;

    return (uint16_t)cct;
}

uint16_t TCS34725_Get_Lux(RGB rgb) {
    float lux, cpl, atime_ms, Gain_temp = 1;
    uint16_t ir = 1;
    uint16_t r_comp, g_comp, b_comp;

    atime_ms = ((256 - IntegrationTime_t) * 2.4);
    ir = (rgb.R + rgb.G + rgb.B > rgb.C) ? (rgb.R + rgb.G + rgb.B - rgb.C) / 2 : 0;
    r_comp = rgb.R - ir;
    g_comp = rgb.G - ir;
    b_comp = rgb.B - ir;

    switch (Gain_t) {
        case TCS34725_GAIN_1X:
            Gain_temp = 1;
            break;
        case TCS34725_GAIN_4X:
            Gain_temp = 4;
            break;
        case TCS34725_GAIN_16X:
            Gain_temp = 16;
            break;
        case TCS34725_GAIN_60X:
            Gain_temp = 60;
            break;
    }

    cpl = (atime_ms * Gain_temp) / (TCS34725_GA * TCS34725_DF);

    lux = (TCS34725_R_Coef * (float)(r_comp) + TCS34725_G_Coef * (float)(g_comp) + TCS34725_B_Coef * (float)(b_comp)) / cpl;
    return (uint16_t)lux;
}

uint32_t TCS34725_GetRGB888(RGB rgb) {
    float i = 1;
    rgb.R = rgb.R * TCS34725_R_Coef;
    if (rgb.R >= rgb.G && rgb.R >= rgb.B) { 
        i = rgb.R / 255 + 1;
    } else if (rgb.G >= rgb.R && rgb.G >= rgb.B) { 
        i = rgb.G / 255 + 1;
    } else if (rgb.B >= rgb.G && rgb.B >= rgb.R) { 
        i = rgb.B / 255 + 1;  
    }
    if (i != 0) {
        rgb.R = (rgb.R) / i;
        rgb.G = (rgb.G) / i;
        rgb.B = (rgb.B) / i;
    }
    if (rgb.R > 30)
        rgb.R = rgb.R - 30;
    if (rgb.G > 30)
        rgb.G = rgb.G - 30;
    if (rgb.B > 30)
        rgb.B = rgb.B - 30;
    rgb.R = rgb.R * 255 / 225;
    rgb.G = rgb.G * 255 / 225;
    rgb.B = rgb.B * 255 / 225;

    if (rgb.R > 255)
        rgb.R = 255;
    if (rgb.G > 255)
        rgb.G = 255;
    if (rgb.B > 255)
        rgb.B = 255;
    return (rgb.R << 16) | (rgb.G << 8) | (rgb.B);
}

uint16_t TCS34725_GetRGB565(RGB rgb) {
    float i = 1;
    if (rgb.R >= rgb.G && rgb.R >= rgb.B) { 
        i = rgb.R / 255 + 1;
    } else if (rgb.G >= rgb.R && rgb.G >= rgb.B) { 
        i = rgb.G / 255 + 1;
    } else if (rgb.B >= rgb.G && rgb.B >= rgb.R) { 
        i = rgb.B / 255 + 1;  
    }
    if (i != 0) {
        rgb.R = (rgb.R) / i;
        rgb.G = (rgb.G) / i;
        rgb.B = (rgb.B) / i;
    }
    if (rgb.R > 30)
        rgb.R = rgb.R - 30;
    if (rgb.G > 30)
        rgb.G = rgb.G - 30;
    if (rgb.B > 30)
        rgb.B = rgb.B - 30;
    rgb.R = rgb.R * 255 / 225;
    rgb.G = rgb.G * 255 / 225;
    rgb.B = rgb.B * 255 / 225;

    if (rgb.R > 255)
        rgb.R = 255;
    if (rgb.G > 255)
        rgb.G = 255;
    if (rgb.B > 255)
        rgb.B = 255;
    return ((rgb.R >> 3) << 11) | ((rgb.G >> 2) << 5) | ((rgb.B >> 3));
}

// void TCS34725_SetLight(uint16_t value) {
//     if (value <= 100) {
//         value = value * DEV_PWM_value / 100;
//         DEV_Set_PWM(value);
//     }
// }
