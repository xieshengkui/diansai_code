/******************************************************************************
* @file    IMU.c
* @brief   IMU（惯性测量单元）驱动与姿态解算模块
* @details 本文件包含IMU初始化、传感器数据读取、陀螺仪去偏、
*          四元数更新及姿态角解算等功能，采用Mahony互补滤波算法
* @author  
* @date    
******************************************************************************/

#include "inv_imu_driver.h"
#include "headfile.h"
//#include "eeprom.h"

/* XYZ结构体类型定义 */

/******************************************************************************
* 全局变量定义
******************************************************************************/

/* 坐标系变换向量 */
xyz_f_t north;   // 北向向量（由南向北方向加速度在加速度计的分量）
xyz_f_t west;    // 西向向量（由东向西方向加速度在加速度计的分量）

/* 姿态解算相关变量 */
volatile float exInt, eyInt, ezInt;  // 误差积分项（用于PI修正）
volatile float q0, q1, q2, q3;       // 全局四元数，表示当前姿态
volatile float integralFBhand, handdiff;  // 手部运动检测相关
volatile uint32_t lastUpdate, now;   // 采样周期计时（单位：us）
volatile float yaw[5] = {0, 0, 0, 0, 0};  // 航向角历史数据（用于处理航向增值）

/* 传感器校准相关 */
int16_t Ax_offset = 0, Ay_offset = 0;  // 加速度计X/Y轴偏移量
float TTangles_gyro[7];                // 传感器原始数据缓存（0-2:加速度, 3-5:陀螺仪, 6:温度）
float Angle_Final[3];                  // 最终倾斜角度（X/Y/Z轴）


void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

/******************************************************************************
* 外部函数声明
******************************************************************************/
extern int bsp_IcmGetRawData(float accel_mg[3], float gyro_dps[3], float *temp_degc);
extern int setup_imu(int use_ln, int accel_en, int gyro_en);

/**************************实现函数********************************************
*函数原型:	   float invSqrt1(float x)
*功　　能:	   快速计算 1/Sqrt(x)，采用经典的Quake III算法
*原　　理:	   通过牛顿迭代法结合整数位操作实现高速开方倒数运算
*输入参数：	x  --- 要计算的值（必须大于0）
*输出参数：	结果 1/Sqrt(x)
*******************************************************************************/
float invSqrt1(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;           // 将float按long型读取（位操作技巧）
	i = 0x5f3759df - (i >> 1);     // 魔数近似初始值
	y = *(float*)&i;               // 转回float
	y = y * (1.5f - (halfx * y * y)); // 牛顿迭代一次提高精度
	return y;
}

/**************************实现函数********************************************
*函数原型:	   void IMU_init(void)
*功　　能:	   初始化IMU模块
*执行流程:	   1. 初始化ICM-45686传感器硬件
			   2. 启动定时器7中断（用于姿态解算定时）
			   3. 初始化四元数为单位四元数
			   4. 清零误差积分项
			   5. 初始化采样时间戳
*输入参数：	   无
*输出参数：	   无（失败时打印错误信息）
*******************************************************************************/
void IMU_init(void)
{	 
	// 初始化IMU传感器（use_ln=1使用磁力计, accel_en=1使能加速度计, gyro_en=1使能陀螺仪）
	if (0x00 == setup_imu(1, 1, 1))
	{
		// 启动定时器7中断，用于定时执行姿态解算
		HAL_TIM_Base_Start_IT(&htim7);
		
		// 初始化四元数为单位四元数（表示初始姿态为水平静止）
		q0 = 1.0f;  // 四元数实部
		q1 = 0.0f;  // 四元数虚部i
		q2 = 0.0f;  // 四元数虚部j
		q3 = 0.0f;  // 四元数虚部k
		
		// 清零误差积分项（用于PI控制器）
		exInt = 0.0;
		eyInt = 0.0;
		ezInt = 0.0;
		
		// 初始化采样时间戳
		lastUpdate = nowtime;
		now = nowtime;
		return;
	}
	
	// 初始化失败，输出错误信息
	printf("IMU ERROR!!\r\n");
}

/******************************************************************************
* 陀螺仪方差计算相关变量（静态变量，仅本文件可见）
******************************************************************************/
static double Gyro_fill[3][300];     // 陀螺仪数据滑动窗口缓冲区（3轴，每轴300个样本）
static double Gyro_total[3];         // 陀螺仪数据累加和
static double sqrGyro_total[3];      // 陀螺仪数据平方和
static int GyroinitFlag = 0;         // 初始化标志（0:未完成, 1:已完成）
static int GyroCount = 0;            // 当前数据索引

/**************************实现函数********************************************
*函数原型:	   void calGyroVariance(float data[], int length, float sqrResult[], float avgResult[])
*功　　能:	   计算陀螺仪数据的方差和平均值，用于静止检测
*原　　理:	   方差变形公式: S^2 = (X1^2+X2^2+...+Xn^2)/n - (X平均)^2
*执行流程:	   采用滑动窗口算法，维护固定长度的数据缓冲区
*输入参数：	   data[]       --- 当前陀螺仪数据（3轴）
			   length       --- 滑动窗口长度
*输出参数：	   sqrResult[]  --- 方差结果（3轴）
			   avgResult[]  --- 平均值（3轴）
*******************************************************************************/
void calGyroVariance(float data[], int length, float sqrResult[], float avgResult[])
{
	int i;
	double tmplen;
	
	// 第一阶段：填充缓冲区，尚未形成完整窗口
	if (GyroinitFlag == 0)
	{
		for (i = 0; i < 3; i++)
		{
			Gyro_fill[i][GyroCount] = data[i];          // 存入缓冲区
			Gyro_total[i] += data[i];                  // 累加和
			sqrGyro_total[i] += data[i] * data[i];     // 平方和
			sqrResult[i] = 100;                        // 初始化为大值
			avgResult[i] = 0;
		}
	}
	// 第二阶段：滑动窗口更新
	else
	{
		for (i = 0; i < 3; i++)
		{
			Gyro_total[i] -= Gyro_fill[i][GyroCount];       // 减去即将被替换的旧数据
			sqrGyro_total[i] -= Gyro_fill[i][GyroCount] * Gyro_fill[i][GyroCount];
			Gyro_fill[i][GyroCount] = data[i];              // 存入新数据
			Gyro_total[i] += Gyro_fill[i][GyroCount];       // 累加新数据
			sqrGyro_total[i] += Gyro_fill[i][GyroCount] * Gyro_fill[i][GyroCount];
		}
	}
	
	GyroCount++;
	if (GyroCount >= length)
	{
		GyroCount = 0;
		GyroinitFlag = 1;  // 窗口已满，开始输出有效结果
	}
	
	// 窗口未填满时不计算
	if (GyroinitFlag == 0)
	{
		return;
	}
	
	// 计算平均值和方差
	tmplen = length;
	for (i = 0; i < 3; i++)
	{
		avgResult[i] = (float)(Gyro_total[i] / tmplen);
		sqrResult[i] = (float)((sqrGyro_total[i] - Gyro_total[i] * Gyro_total[i] / tmplen) / tmplen);
	}
}

/******************************************************************************
* 陀螺仪自动校准相关变量
******************************************************************************/
float gyro_offset[3] = {0};  // 陀螺仪偏移量（自动校准后更新）
int CalCount = 0;            // 校准计数器
/**************************实现函数********************************************
*函数原型:	   void IMU_getValues(float * values)
*功　　能:	   读取加速度计、陀螺仪的当前值，并进行陀螺仪自动校准
*执行流程:	   1. 从ICM-45686读取原始ADC数据
		   2. 更新全局数据缓存TTangles_gyro
		   3. 计算陀螺仪方差，检测是否静止
		   4. 静止时自动更新陀螺仪偏移量
		   5. 输出去偏后的陀螺仪数据
*输入参数：	   values[] --- 结果输出数组
*输出参数：	   values[0-2] = 加速度原始值
		   values[3-5] = 陀螺仪去偏后值
*传感器量程:	   陀螺仪量程已配置为1000度/秒
*******************************************************************************/
void IMU_getValues(float * values) {  
	float accgyroval[7];      // 临时数据缓冲区
	float sqrResult_gyro[3];  // 陀螺仪方差计算结果
	float avgResult_gyro[3];  // 陀螺仪平均值
	
	// 从ICM-45686读取原始数据
	// accgyroval[0-2] = 加速度计原始值
	// accgyroval[3-5] = 陀螺仪原始值  
	// accgyroval[6]   = 温度值
	bsp_IcmGetRawData(accgyroval, &accgyroval[3], &accgyroval[6]);
	
	// 更新全局数据缓存
	TTangles_gyro[0] = accgyroval[0];
	TTangles_gyro[1] = accgyroval[1];
	TTangles_gyro[2] = accgyroval[2];
	TTangles_gyro[3] = accgyroval[3];
	TTangles_gyro[4] = accgyroval[4];
	TTangles_gyro[5] = accgyroval[5];
	TTangles_gyro[6] = accgyroval[6];
	
	// 计算陀螺仪方差（用于静止检测）
	calGyroVariance(&TTangles_gyro[3], 100, sqrResult_gyro, avgResult_gyro);
	
	// 静止检测条件：
	if (sqrResult_gyro[0] < 2.0f && sqrResult_gyro[1] < 2.0f && 
	    sqrResult_gyro[2] < 2.0f && CalCount >= 99)
	{
		// 更新陀螺仪偏移量（当前平均值即为零漂）
		gyro_offset[0] = avgResult_gyro[0];
		gyro_offset[1] = avgResult_gyro[1];
		gyro_offset[2] = avgResult_gyro[2];
		
		// ========== 调试输出：打印更新后的零漂值 ==========
		printf("[IMU] Zero Offset Updated: X=%.4f, Y=%.4f, Z=%.4f\r\n",
		       gyro_offset[0], gyro_offset[1], gyro_offset[2]);
		// ================================================
		
		// 重置误差积分项
		exInt = 0;
		eyInt = 0;
		ezInt = 0;
		CalCount = 0;
	}
	else if (CalCount < 100)
	{
		CalCount++;  // 计数尚未达到100，继续累积
	}
	
	// 输出结果
	values[0] = accgyroval[0];                    // 加速度X（原始值）
	values[1] = accgyroval[1];                    // 加速度Y（原始值）
	values[2] = accgyroval[2];                    // 加速度Z（原始值）
	values[3] = accgyroval[3] - gyro_offset[0];   // 陀螺仪X（去偏后）
	values[4] = accgyroval[4] - gyro_offset[1];   // 陀螺仪Y（去偏后）
	values[5] = accgyroval[5] - gyro_offset[2];   // 陀螺仪Z（去偏后）
	
	// 注意：陀螺仪量程已配置为1000度/秒
}


/**************************实现函数********************************************
*函数原型:	   void IMU_AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
*功　　能:	   姿态解算核心函数，采用Mahony互补滤波算法更新四元数
*原　　理:	   通过陀螺仪积分预测姿态，加速度计/磁力计修正漂移
*控制参数:	   Kp=0.5f  - 比例增益（控制收敛速度）
			   Ki=0.001f - 积分增益（控制零偏补偿）
*输入参数：	   gx,gy,gz - 陀螺仪角速度（弧度/秒）
			   ax,ay,az - 加速度计原始值
			   mx,my,mz - 磁力计原始值
*输出参数：	   更新全局四元数 q0,q1,q2,q3
*******************************************************************************/
#define Kp 0.5f    // 比例增益（控制加速度计/磁力计的修正强度）
#define Ki 0.001f  // 积分增益（控制陀螺仪零偏补偿速度）

void IMU_AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
  float norm;                 // 向量模长
  float vx, vy, vz;           // 由四元数推算的重力向量
  float ex, ey, ez, halfT;    // 误差向量和半周期
  float tempq0, tempq1, tempq2, tempq3;  // 临时四元数

  // 预计算四元数乘积项，减少重复计算
  float q0q0 = q0 * q0;
  float q0q1 = q0 * q1;
  float q0q2 = q0 * q2;
  float q1q1 = q1 * q1;
  float q1q3 = q1 * q3;
  float q2q2 = q2 * q2;   
  float q2q3 = q2 * q3;
  float q3q3 = q3 * q3;   

  // 计算采样周期（单位：秒）
  now = nowtime;
  if (now < lastUpdate) {
    // 定时器溢出处理
    halfT = ((float)(now + (0xffff - lastUpdate)) / 20000.0f);
  } else {
    halfT = ((float)(now - lastUpdate) / 20000.0f);
  }
  lastUpdate = now;  // 更新时间戳
  
    // 加速度计数据归一化（转成单位向量）
  norm = invSqrt1(ax * ax + ay * ay + az * az);       
  ax = ax * norm;
  ay = ay * norm;
  az = az * norm;

  // 磁力计数据归一化
  norm = invSqrt1(mx * mx + my * my + mz * mz);
  mx = mx * norm;
  my = my * norm;
  mz = mz * norm;

  // 根据当前四元数推算重力向量在机体坐标系的投影
  // 这是方向余弦矩阵第三列的三个元素
  vx = 2 * (q1q3 - q0q2);
  vy = 2 * (q0q1 + q2q3);
  vz = q0q0 - q1q1 - q2q2 + q3q3;
  
    // 计算北向和西向向量（用于后续导航计算）
  north.x = 1 - 2 * (q3 * q3 + q2 * q2);
  north.y = 2 * (-q0 * q3 + q1 * q2);
  north.z = 2 * (q0 * q2 - q1 * q3);
  west.x = 2 * (q0 * q3 + q1 * q2);
  west.y = 1 - 2 * (q3 * q3 + q1 * q1);
  west.z = 2 * (-q0 * q1 + q2 * q3);
//  wx = 2*bx*(0.5 - q2q2 - q3q3) + 2*bz*(q1q3 - q0q2);
//  wy = 2*bx*(q1q2 - q0q3) + 2*bz*(q0q1 + q2q3);
//  wz = 2*bx*(q0q2 + q1q3) + 2*bz*(0.5 - q1q1 - q2q2);  
  
    // 计算误差向量（加速度计测量值与推算值的叉积）
  // exyz表示陀螺积分姿态与加速度计测量姿态之间的偏差
  ex = (ay * vz - az * vy);
  ey = (az * vx - ax * vz);
  ez = (ax * vy - ay * vx);

  // 误差积分与PI修正（仅当误差非零时进行）
  if (ex != 0.0f && ey != 0.0f && ez != 0.0f) {
    // 积分误差累积
    exInt = exInt + ex * Ki * halfT;
    eyInt = eyInt + ey * Ki * halfT;	
    ezInt = ezInt + ez * Ki * halfT;

    // PI修正：用误差向量修正陀螺仪读数
    gx = gx + Kp * ex + exInt;
    gy = gy + Kp * ey + eyInt;
    gz = gz + Kp * ez + ezInt;
  }

  // 四元数微分方程（龙格-库塔一阶近似）
  tempq0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halfT;
  tempq1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * halfT;
  tempq2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * halfT;
  tempq3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * halfT;  
  
  // 四元数规范化（确保模长为1）
  norm = invSqrt1(tempq0 * tempq0 + tempq1 * tempq1 + tempq2 * tempq2 + tempq3 * tempq3);
  q0 = tempq0 * norm;
  q1 = tempq1 * norm;
  q2 = tempq2 * norm;
  q3 = tempq3 * norm;
}


/**************************实现函数********************************************
*函数原型:	   void IMU_getQ(float * q)
*功　　能:	   更新四元数并返回当前四元数值
*执行流程:	   1. 读取传感器原始数据
			   2. 调用AHRS更新四元数
			   3. 返回当前四元数
*输入参数：	   q[] --- 四元数输出数组
*输出参数：	   q[0-3] = 当前四元数 (q0=实部, q1-3=虚部)
*******************************************************************************/
float mygetqval[9];  // 传感器数据临时缓冲区

void IMU_getQ(float * q) {
  // 读取传感器数据
  IMU_getValues(mygetqval);	 
  
  // 更新AHRS四元数
  // 陀螺仪：度/秒 -> 弧度/秒
  // 加速度计和磁力计保持原始ADC值
  IMU_AHRSupdate(mygetqval[3] * M_PI / 180, 
                 mygetqval[4] * M_PI / 180, 
                 mygetqval[5] * M_PI / 180,
                 mygetqval[0], mygetqval[1], mygetqval[2], 
                 mygetqval[6], mygetqval[7], mygetqval[8]);

  // 返回当前四元数值
  q[0] = q0;
  q[1] = q1;
  q[2] = q2;
  q[3] = q3;
}


/**************************实现函数********************************************
*函数原型:	   void IMU_getYawPitchRoll(volatile float * angles)
*功　　能:	   更新四元数并返回解算后的姿态角（偏航/俯仰/横滚）
*执行流程:	   1. 更新四元数
			   2. 四元数转欧拉角
*输入参数：	   angles[] --- 姿态角输出数组
*输出参数：	   angles[0] = yaw(偏航角, -180~180度)
			   angles[1] = pitch(俯仰角, -90~90度)
			   angles[2] = roll(横滚角, -180~180度)
*******************************************************************************/
void IMU_getYawPitchRoll(volatile float * angles) {
  float q[4];  // 四元数临时变量
  
  IMU_getQ(q);  // 更新全局四元数
  
  // 四元数转欧拉角（Z-Y-X顺序，即偏航-俯仰-横滚）
  angles[0] = -atan2(2 * q[1] * q[2] + 2 * q[0] * q[3], 
                     -2 * q[2] * q[2] - 2 * q[3] * q[3] + 1) * 180 / M_PI;  // yaw
  angles[1] = -asin(-2 * q[1] * q[3] + 2 * q[0] * q[2]) * 180 / M_PI;  // pitch
  angles[2] = atan2(2 * q[2] * q[3] + 2 * q[0] * q[1], 
                     -2 * q[1] * q[1] - 2 * q[2] * q[2] + 1) * 180 / M_PI;  // roll
  // 注：如需将yaw转换为0-360度，取消下行注释
  // if(angles[0] < 0) angles[0] += 360.0f;
}

/**************************实现函数********************************************
*函数原型:	   void IMU_TT_getgyro(float * zsjganda)
*功　　能:	   快速读取传感器原始数据缓存（不访问硬件）
*特　　点:	   低延迟，仅读取上次IMU_getValues更新的数据
*输入参数：	   zsjganda[] --- 数据输出数组
*输出参数：	   zsjganda[0-2] = 加速度计原始值
			   zsjganda[3-5] = 陀螺仪原始值
			   zsjganda[6]   = 温度值
*******************************************************************************/
void IMU_TT_getgyro(float * zsjganda)
{
  zsjganda[0] = TTangles_gyro[0];  // 加速度X
  zsjganda[1] = TTangles_gyro[1];  // 加速度Y
  zsjganda[2] = TTangles_gyro[2];  // 加速度Z
  zsjganda[3] = TTangles_gyro[3];  // 陀螺仪X
  zsjganda[4] = TTangles_gyro[4];  // 陀螺仪Y
  zsjganda[5] = TTangles_gyro[5];  // 陀螺仪Z
  zsjganda[6] = TTangles_gyro[6];  // 温度
}

/**************************实现函数********************************************
*函数原型:	   void MPU6050_InitAng_Offset(void)
*功　　能:	   MPU6050角度偏移初始化（预留接口）
*备　　注:	   当前硬件使用ICM-45686，此函数保留用于兼容性
*******************************************************************************/
void MPU6050_InitAng_Offset(void)
{
  // 预留函数体
}

//------------------End of File----------------------------