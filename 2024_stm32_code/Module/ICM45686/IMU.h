#ifndef __IMU_H
#define __IMU_H

#include "headfile.h"

typedef struct
{
    float x;
    float y;
    float z;
} xyz_f_t;
extern volatile uint32_t nowtime;
extern xyz_f_t north,west;
extern volatile float ypr[3];   //处理航向的增值
extern float motion6[7];
//Mini IMU AHRS 解算的API
void IMU_init(void); //初始化
void IMU_getYawPitchRoll(volatile float * ypr); //更新姿态
void IMU_TT_getgyro(float * zsjganda);
//uint32_t micros(void);	//读取系统上电后的时间  单位 us 
void MPU6050_InitAng_Offset(void);
#endif

//------------------End of File----------------------------