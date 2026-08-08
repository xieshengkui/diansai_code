#include "headfile.h"

uint8_t OLED_GRAM[144][8];

//反色显示设置
void OLED_ColorTurn(uint8_t i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xA6,OLED_CMD);//正常显示
		}
	if(i==1)
		{
			OLED_WR_Byte(0xA7,OLED_CMD);//反色显示
		}
}

//屏幕旋转180度
void OLED_DisplayTurn(uint8_t i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xC8,OLED_CMD);//正常显示
			OLED_WR_Byte(0xA1,OLED_CMD);
		}
	if(i==1)
		{
			OLED_WR_Byte(0xC0,OLED_CMD);//翻转显示
			OLED_WR_Byte(0xA0,OLED_CMD);
		}
}

void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    if(cmd)
      OLED_DC_Set();
    else
      OLED_DC_Clr();
    OLED_CS_Clr();

      //发送数据
      DL_SPI_transmitData8(SPI_OLED_INST, dat);
      //等待SPI总线空闲
      while(DL_SPI_isBusy(SPI_OLED_INST));

    OLED_CS_Set();
    OLED_DC_Set();
}

//开启OLED显示 
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵设置
	OLED_WR_Byte(0x14,OLED_CMD);//开启电荷泵
	OLED_WR_Byte(0xAF,OLED_CMD);//打开屏幕
}

//关闭OLED显示 
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵设置
	OLED_WR_Byte(0x10,OLED_CMD);//关闭电荷泵
	OLED_WR_Byte(0xAE,OLED_CMD);//关闭屏幕
}

//刷新显存到OLED	
void OLED_Refresh(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   OLED_WR_Byte(0xb0+i,OLED_CMD); //设置页起始地址
	   OLED_WR_Byte(0x00,OLED_CMD);   //设置列低地址
	   OLED_WR_Byte(0x10,OLED_CMD);   //设置列高地址
	   for(n=0;n<128;n++)
		 OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
  }
}
//清屏
void OLED_Clear(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   for(n=0;n<128;n++)
			{
			 OLED_GRAM[n][i]=0;//清空显存
			}
  }
	OLED_Refresh();//刷新显示
}

//画点 
//x:0~127
//y:0~63
//t:1 点亮 0,熄灭	
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	if(t){OLED_GRAM[x][i]|=n;}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t mode)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算x轴差值 
	delta_y=y2-y1;
	uRow=x1;//起点X
	uCol=y1;//起点Y
	if(delta_x>0)incx=1; //正向步进 
	else if (delta_x==0)incx=0;//竖线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//横线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //取长轴作为步进总长度 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		OLED_DrawPoint(uRow,uCol,mode);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}
//x,y:圆心坐标
//r:圆半径
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
	int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b,1);
        OLED_DrawPoint(x - a, y - b,1);
        OLED_DrawPoint(x - a, y + b,1);
        OLED_DrawPoint(x + a, y + b,1);
 
        OLED_DrawPoint(x + b, y + a,1);
        OLED_DrawPoint(x + b, y - a,1);
        OLED_DrawPoint(x - b, y - a,1);
        OLED_DrawPoint(x - b, y + a,1);
        
        a++;
        num = (a * a + b * b) - r*r;//判断是否超出圆边界
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}

//在指定位置显示单个字符
//x:0~127
//y:0~63
//size1:字体大小 6x8/6x12/8x16/12x24
//mode:0,白底黑字;1,黑底白字
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode)
{
	uint8_t i,m,temp,size2,chr1;
	uint8_t x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //计算单个字符占用字节数
	chr1=chr-' ';  //偏移至字库起始索引
	for(i=0;i<size2;i++)
	{
		if(size1==8)
			  {temp=asc2_0806[chr1][i];} //调用0806字库
		else if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字库
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字库
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字库
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
}

//显示字符串
//x,y:起始坐标  
//size1:字体大小 
//*chr:字符串首地址 
//mode:0,白底黑字;1,黑底白字
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode)
{
	while((*chr>=' ')&&(*chr<='~'))//判断是否为可打印字符
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8)x+=6;
		else x+=size1/2;
		chr++;
  }
}

//m^n
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

//显示数字
//x,y :起始坐标
//num :要显示的数字
//len :数字位数
//size:字体大小
//mode:0,白底黑字;1,黑底白字
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode)
{
	uint8_t t,temp,m=0;
	if(size1==8)m=2;
	for(t=0;t<len;t++)
	{
		temp=(num/OLED_Pow(10,len-t-1))%10;
			if(temp==0)
			{
				OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
      }
			else
			{
			  OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
			}
  }
}

/**
  * @brief  显示带符号整数（支持负数）
  * @param  x,y  : 起始坐标
  * @param  num  : 要显示的带符号整数
  * @param  len  : 总显示位数（包含负号）
  * @param  size1: 字体大小
  * @param  mode : 0=白底黑字, 1=黑底白字
  */
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size1, uint8_t mode)
{
    uint8_t t, temp, m = 0;
    uint32_t abs_num;
    uint8_t cur_x = x;
    uint8_t digit_len;

    // 字体间距补偿，和原函数逻辑完全一致
    if (size1 == 8)
        m = 2;

    // 1. 处理符号：负数先显示负号，正数直接显示数字
    if (num < 0)
    {
        // 显示负号
        OLED_ShowChar(cur_x, y, '-', size1, mode);
        // 光标右移一个字符宽度
        cur_x += (size1 / 2 + m);
        // 取绝对值用于后续数字显示
        abs_num = (uint32_t)(-num);
        // 剩余位数用于显示数字（总位数减去负号占的1位）
        digit_len = len - 1;
    }
    else
    {
        // 非负数：逻辑和原ShowNum完全一致
        abs_num = (uint32_t)num;
        digit_len = len;
    }

    // 2. 显示数字部分，高位补0，和原函数逻辑完全一致
    for (t = 0; t < digit_len; t++)
    {
        temp = (abs_num / OLED_Pow(10, digit_len - t - 1)) % 10;
        OLED_ShowChar(cur_x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
    }
}

//显示浮点数（保留两位小数，消隐前导零，支持正负）
//x,y     :起始坐标
//num     :要显示的浮点数（支持正负）
//intLen  :整数部分最大位数（包含负号占位，用于右对齐）
//size1   :字体大小
//mode    :0白底黑字;1黑底白字
void OLED_ShowFloat(uint8_t x,uint8_t y,float num,uint8_t intLen,uint8_t size1,uint8_t mode)
{
    uint8_t m = 0;
    if (size1 == 8) m = 2;
    uint8_t step = size1 / 2 + m;  // 单个字符像素宽度

    bool isNeg = false;
    float absNum = num;
    // 区分正负
    if(num < 0.0f)
    {
        isNeg = true;
        absNum = -num;
    }

    // 分离整数、小数，四舍五入保留两位
    uint32_t intPart = (uint32_t)absNum;
    uint32_t decPart = (uint32_t)((absNum - (float)intPart) * 100.0f + 0.5f);

    // 小数进位溢出处理：9.996 → 10.00
    if (decPart >= 100)
    {
        intPart += 1;
        decPart -= 100;
    }

    // 计算整数数字实际位数（不含负号）
    uint8_t actualDigits = 1;
    uint32_t temp = intPart;
    while (temp >= 10)
    {
        actualDigits++;
        temp /= 10;
    }

    // 总占用字符数 = 符号(0/1) + 整数数字位
    uint8_t totalDigitLen = actualDigits + (isNeg ? 1 : 0);
    // 前导空格填充，实现右对齐
    uint8_t padCount = (totalDigitLen < intLen) ? (intLen - totalDigitLen) : 0;

    // 1. 打印前导空白
    uint8_t curX = x;
    for(uint8_t i = 0; i < padCount; i++)
    {
        OLED_ShowChar(curX, y, ' ', size1, mode);
        curX += step;
    }

    // 2. 打印负号（负数才输出）
    if(isNeg)
    {
        OLED_ShowChar(curX, y, '-', size1, mode);
        curX += step;
    }

    // 3. 打印整数部分（自动消隐前导零）
    for(uint8_t i = 0; i < actualDigits; i++)
    {
        uint8_t digit = (intPart / OLED_Pow(10, actualDigits - 1 - i)) % 10;
        OLED_ShowChar(curX, y, '0' + digit, size1, mode);
        curX += step;
    }

    // 4. 小数点位置：整数区总宽度位置
    uint8_t dotX = x + step * intLen;
    OLED_ShowChar(dotX, y, '.', size1, mode);

    // 5. 两位小数，不足自动补0
    uint8_t decX = dotX + (size1 / 2);
    OLED_ShowNum(decX, y, decPart, 2, size1, mode);
}

//显示汉字
//x,y:起始坐标
//num:对应汉字字库索引
//mode:0,白底黑字;1,黑底白字
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode)
{
	uint8_t m,temp;
	uint8_t x0=x,y0=y;
	uint16_t i,size3=(size1/8+((size1%8)?1:0))*size1;  //单个汉字占用字节数
	for(i=0;i<size3;i++)
	{
		if(size1==16)
				{temp=Hzk1[num][i];}//调用16*16字库
		else if(size1==24)
				{temp=Hzk2[num][i];}//调用24*24字库
		else if(size1==32)       
				{temp=Hzk3[num][i];}//调用32*32字库
		else if(size1==64)
				{temp=Hzk4[num][i];}//调用64*64字库
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((x-x0)==size1)
		{x=x0;y0=y0+8;}
		y=y0;
	}
}

//num 滚动汉字数量
//space 每组文字间隔空格数
//mode:0,白底黑字;1,黑底白字
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t mode)
{
	uint8_t i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,24,t,16,mode); //右侧写入新汉字到显存
			t++;
		}
		if(t==num)
			{
				for(r=0;r<16*space;r++)      //空白停留
				 {
					for(i=1;i<144;i++)
						{
							for(n=0;n<8;n++)
							{
								OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
							}
						}
           OLED_Refresh();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=1;i<144;i++)   //实现左移滚动
		{
			for(n=0;n<8;n++)
			{
				OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
			}
		}
		OLED_Refresh();
	}
}

//x,y图片左上角坐标
//sizex,sizey,图片宽高
//BMP[]要显示的图片数组
//mode:0,白底黑字;1,黑底白字
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,uint8_t BMP[],uint8_t mode)
{
	uint16_t j=0;
	uint8_t i,n,temp,m;
	uint8_t x0=x,y0=y;
	sizey=sizey/8+((sizey%8)?1:0);
	for(n=0;n<sizey;n++)
	{
		 for(i=0;i<sizex;i++)
		 {
				temp=BMP[j];
				j++;
				for(m=0;m<8;m++)
				{
					if(temp&0x01)OLED_DrawPoint(x,y,mode);
					else OLED_DrawPoint(x,y,!mode);
					temp>>=1;
					y++;
				}
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y0=y0+8;
				}
				y=y0;
     }
	 }
}
//OLED的初始化
void OLED_Init(void)
{
    OLED_RES_Clr();
    delay_ms(200);
    OLED_RES_Set();

    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset        Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
    OLED_Clear();
    OLED_WR_Byte(0xAF,OLED_CMD);

	OLED_ColorTurn(0);//0正常显示，1 反色显示
    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
}