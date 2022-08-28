/********************************
 # Editor : miemie
 # Version: 2.0
 # Product: SSD1306 ADS1115 HC-05/06
 # Date:24.06.2022
**********************************/
#include <Arduino.h>
#include <U8x8lib.h>
#include <U8g2lib.h>
#include<ADS1115_WE.h> 
#include<Wire.h>

#define ADS1115_I2C_ADDRESS 0x48
#define LED 13
#define EN1 9  //TMUX1108
#define A0 10
#define A1 11
#define A2 12


String inputString = "";         // a String to hold incoming data
bool stringComplete = false; 
int showMode = 3;               //ssd1306显示模式
float k1 = 0, b1 = 0;           //kk斜率，bb截距
float xConcentration[10] = {0}; //x轴浓度值
float yPotential[10] = {0};     //y轴电位值
int xnum = 0;           //x轴数据个数

ADS1115_WE adc = ADS1115_WE(ADS1115_I2C_ADDRESS);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);//用我就对了，丑是丑了点
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);//全功能
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);//do-while画图


void setup() 
  {
  pinMode(LED, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);

  //////////////////////Serial
  Serial.begin(115200);
  inputString.reserve(200);  //string length
  Serial.println("Serial begin");

  //////////////////////SSD1306
  u8g2.begin();
  u8x8.begin();
  u8x8.setPowerSave(0);
  
  //////////////////////ADS1115
  Wire.begin();
  if(!adc.init())
    {
      Serial.println("ADS1115 not connected!");
    }

  /* Set the voltage range of the ADC to adjust the gain
   * Please note that you must not apply more than VDD + 0.3V to the input pins!
   * 
   * ADS1115_RANGE_6144  ->  +/- 6144 mV
   * ADS1115_RANGE_4096  ->  +/- 4096 mV
   * ADS1115_RANGE_2048  ->  +/- 2048 mV (default)
   * ADS1115_RANGE_1024  ->  +/- 1024 mV
   * ADS1115_RANGE_0512  ->  +/- 512 mV
   * ADS1115_RANGE_0256  ->  +/- 256 mV
   */
  adc.setVoltageRange_mV(ADS1115_RANGE_6144); //comment line/change parameter to change range

  /* Set the inputs to be compared
   *  
   *  ADS1115_COMP_0_1    ->  compares 0 with 1 (default)
   *  ADS1115_COMP_0_3    ->  compares 0 with 3
   *  ADS1115_COMP_1_3    ->  compares 1 with 3
   *  ADS1115_COMP_2_3    ->  compares 2 with 3
   *  ADS1115_COMP_0_GND  ->  compares 0 with GND
   *  ADS1115_COMP_1_GND  ->  compares 1 with GND
   *  ADS1115_COMP_2_GND  ->  compares 2 with GND
   *  ADS1115_COMP_3_GND  ->  compares 3 with GND
   */
  //adc.setCompareChannels(ADS1115_COMP_0_GND); //comment line/change parameter to change channel 后面写了，这里不需要

  /* Set the conversion rate in SPS (samples per second)
   * Options should be self-explaining: 
   * 
   *  ADS1115_8_SPS 
   *  ADS1115_16_SPS  
   *  ADS1115_32_SPS 
   *  ADS1115_64_SPS  
   *  ADS1115_128_SPS (default)
   *  ADS1115_250_SPS 
   *  ADS1115_475_SPS 
   *  ADS1115_860_SPS 
   */
   //adc.setConvRate(ADS1115_64_SPS); //uncomment if you want to change the default

  /* Set continuous or single shot mode:
   * 
   *  ADS1115_CONTINUOUS  ->  continuous mode
   *  ADS1115_SINGLE     ->  single shot mode (default)
   */
  //adc.setMeasureMode(ADS1115_CONTINUOUS); //comment line/change parameter to change mode

  /* Enable or disable permanent automatic range selection mode. If enabled, the range will
   * change if the measured values are outside of 30-80% of the maximum value of the current 
   * range.  
   * !!! Use EITHER this function once OR setAutoRange() whenever needed (see below) !!!
   */
  adc.setPermanentAutoRangeMode(true);
  

  /*初始化设置*/
  digitalWrite(LED, HIGH);  //LED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
 // u8x8.drawString(0,0,"----------------");
 // u8x8.drawString(0,1,"Receive Command!");
 // u8x8.drawString(0,2,"----------------");
  }


void loop()
  { 
    SSD1306_DisplayMode(showMode); //0: A0-A4 Voltage display 
    
      if (stringComplete)  // print the string when a newline arrives:
        { 
          Serial.println("Received");

          //输入命令
          if (inputString.startsWith("LED"))
            {
              if (inputString.startsWith("LED OFF")||inputString.startsWith("LED off")||inputString.startsWith("LED 0"))
                {
                  digitalWrite(LED, LOW);
                  Serial.println("LED OFF");
                }
              else
                {
                  digitalWrite(LED, HIGH);
                  Serial.println("LED ON");
                }
            }

          else if (inputString.startsWith("ADC"))
            {
              int num = inputString.substring(4).toInt();
              showMode = 0;
              u8x8.clearDisplay();
              float value = 0;
              switch(num)
              {
                case 0:
                value = readChannel(ADS1115_COMP_0_GND);
                break;
                case 1:
                value = readChannel(ADS1115_COMP_1_GND);
                break;
                case 2:
                value = readChannel(ADS1115_COMP_2_GND);
                break;
                case 3:
                value = readChannel(ADS1115_COMP_3_GND);
                break;
            
              }
              Serial.println(value,5);
              u8x8.clearDisplay();
              u8x8.drawString(0,7,"ADC:");
              u8x8.setCursor(5,7);
              u8x8.print(value,5);
              delay(1000);
            }


          else if (inputString.startsWith("DATA")) 
            {
              showMode = 0; //设置空白模式
              u8x8.clearDisplay();
              if(inputString[5]=='x'||inputString[5]=='X') //e.g. 发送格式DATA3x 1 2 3
              {
                int num=inputString.substring(4).toInt();//数据计数器
                xnum=inputString.substring(4).toInt();//数据计数器
                for(int i=0;i<num;i++)
                {
                  xConcentration[i]=inputString.substring(6+i*2).toFloat();
                }
                //for(int i=0;i<num;i++){Serial.println(xConcentration[i]);}
                u8x8.drawString(0,0,"X:"); //显示X轴数据
                for(int i=0;i<num;i++)
                {
                 u8x8.setCursor(2,i);
                 u8x8.print(xConcentration[i]);
                }

              }
              else if(inputString[5]=='y'||inputString[5]=='Y')//e.g. 发送格式DAT4y 1.1234 2.1234 3.1456 0.1651 都是4位小数
              {
                int num=inputString.substring(4).toInt();//数据计数器
                for(int i=0;i<num;i++)
                {
                  yPotential[i]=inputString.substring(6+i*7).toFloat();
                }
                //for(int i=0;i<num;i++){Serial.println(yPotential[i],5);}
                u8x8.drawString(7,0,"Y:");
                u8x8.drawString(0,0,"X:");
                for(int i=0;i<num;i++)
                {
                 u8x8.setCursor(9,i);
                 u8x8.print(yPotential[i],4);
                 u8x8.setCursor(2,i);
                 u8x8.print(xConcentration[i]);
                }
              }

              else
              {
                for(int i=0;i<10;i++)//clear the array
                {
                  xConcentration[i]=0;
                  yPotential[i]=0;
                }

                Serial.println("no data");
              }
             
            }            


          else if(inputString.startsWith("MODE"))
            {
              if (inputString.startsWith("MODE 0")||inputString.startsWith("STOP")||inputString.startsWith("stop"))
                {
                  showMode = 0;
                  u8x8.clearDisplay();
                  Serial.println("MODE 0");
                }
              else if (inputString.startsWith("MODE 1"))
                {
                  showMode = 1;
                  u8x8.clearDisplay();
                  Serial.println("MODE 1");
                }
              else if (inputString.startsWith("MODE 2"))
                {
                  showMode = 2;
                  u8x8.clearDisplay();
                  Serial.println("MODE 2");
                }
              else if (inputString.startsWith("MODE 3")||inputString.startsWith("START")||inputString.startsWith("start"))
                {
                  showMode = 3;
                  u8x8.clearDisplay();
                  Serial.println("MODE 3");
                }
              else if (inputString.startsWith("MODE 4"))
                {
                  showMode = 4;
                  u8x8.clearDisplay();
                  Serial.println("MODE 4");
                }
              else if (inputString.startsWith("MODE 5"))
                {
                  showMode = 5;
                  u8x8.clearDisplay();
                  Serial.println("MODE 5");
                }
            }
  
           
          else if(inputString=="a\n")
            {
              u8x8.clearDisplay();
              u8x8.drawString(0,3,"Command:");
              u8x8.setCursor(0,4);
              u8x8.print(inputString);

            }

          inputString = ""; // clear the string:
          stringComplete = false;
          //digitalWrite(LED,digitalRead(LED)^1); 
                          
              
        }

  }

void SSD1306_DisplayMode(int modeNum)
  {
    float voltage = 0.0;

    switch(modeNum)
    {
      case 0:

        break;


      case 1: //不处理版本，直接显示S1-S8的放大后电压
             
        //u8x8.setFont(u8x8_font_chroma48medium8_r);
        //u8x8.drawString(0,0,"Potential [V]:");

        for(int num=1;num<9;num++) //8个channal
        {
          TMUX1108_Mode(num);
          delay(10);
          voltage = readChannel(ADS1115_COMP_0_GND);
          //unsigned int voltageRange = adc.getVoltageRange_mV(); //查看分辨率
          //Serial.print("Channel0 Resolution >>>");
          //Serial.println(voltageRange);
          char buffer[20];
          sprintf(buffer, "Channel S%d [V]: ", num);
          Serial.print(buffer);
          Serial.println(voltage,4);
          sprintf(buffer, "S%d: ", num);
          u8x8.drawString(0,num-1, buffer);
          u8x8.setCursor(4,num-1);
          u8x8.print(voltage,4);
          delay(10);
        }
        break; 
      
      case 2: //均值滤波版本，直接显示S2-S8的放大后电压
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        //u8x8.clearDisplay();
        u8x8.drawString(0,0,"Potential [V]:");

        for(int num=2;num<9;num++) //7个channal
        {
          TMUX1108_Mode(num);
          delay(10);
          voltage = 0;
          for(int i=0;i<20;i++)//求平均电压值，30个一组效果最好
          {
            voltage += readChannel(ADS1115_COMP_0_GND);
            delay(2);
          }
          voltage = voltage/20;
          //unsigned int voltageRange = adc.getVoltageRange_mV(); //查看分辨率
          //Serial.print("Channel0 Resolution >>>");
          //Serial.println(voltageRange);
          char buffer[20];
          sprintf(buffer, "Channel S%d [V]: ", num); 
          Serial.print(buffer);
          Serial.println(voltage,4);
          sprintf(buffer, "S%d: ", num);
          u8x8.drawString(0,num-1, buffer);
          u8x8.setCursor(4,num-1);
          u8x8.print(voltage,4);
          delay(10);
        }
        break;   

      case 3: //均值滤波版本，直接显示S2-S8的原电压，串口每次发1个点,对应串口调试助手波形显示
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        //u8x8.clearDisplay();
        u8x8.drawString(0,0,"Potential [V]:");

        for(int num=2;num<9;num++) //7个channal
        {
          TMUX1108_Mode(num);
          delay(10);
          voltage = 0;
          for(int i=0;i<30;i++)//求平均电压值
          {
            voltage += readChannel(ADS1115_COMP_0_GND);
            delay(2);
          }
          voltage = voltage/30;
          //unsigned int voltageRange = adc.getVoltageRange_mV(); //查看分辨率
          //Serial.print("Channel0 Resolution >>>");
          //Serial.println(voltageRange);
          char buffer[20];
          //sprintf(buffer, "Channel S%d [V]: ", num); 
          sprintf(buffer, "S%d=", num); 
          Serial.print(buffer);
          Serial.println(voltage/2,4);
          sprintf(buffer, "S%d: ", num);
          u8x8.drawString(0,num-1, buffer);
          u8x8.setCursor(4,num-1);
          u8x8.print(voltage/2,4);
          delay(10);
        }
        break;  


      case 4: //均值滤波版本，直接显示S4的原电压，串口每次发1个点,对应串口调试助手波形显示，保留4位小数
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        //u8x8.clearDisplay();
        u8x8.drawString(0,0,"Potential [V]:");

        //for(int num=1;num<7;num++) //6个channal
        //{
          int num=4;
          TMUX1108_Mode(num);
          delay(10);
          voltage = 0;
          for(int i=0;i<20;i++)//求平均电压值
          {
            voltage += readChannel(ADS1115_COMP_0_GND);
          }
          voltage = voltage/20;
          //unsigned int voltageRange = adc.getVoltageRange_mV(); //查看分辨率
          //Serial.print("Channel0 Resolution >>>");
          //Serial.println(voltageRange);
          char buffer[20];
          //sprintf(buffer, "Channel S%d [V]: ", num); 
          sprintf(buffer, "S%d=", num); 
          Serial.print(buffer);
          Serial.println(voltage/2,4);
          sprintf(buffer, "S%d: ", num);
          u8x8.drawString(0,num, buffer);
          u8x8.setCursor(4,num);
          u8x8.print(voltage/2,4);
          delay(10);
        //}
        break; 

      case 5:         //通过输入的点来拟合曲线
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        LScurveFitting(xConcentration, yPotential, xnum, &k1, &b1);
        Serial.print("rSquare: ");
        Serial.println(LScurveFitting(xConcentration, yPotential, xnum, &k1, &b1),4);
        Serial.print("k1=");
        Serial.println(k1,4);
        Serial.print("b1=");
        Serial.println(b1,4);
        u8x8.drawString(0,6, "k=");
        u8x8.setCursor(4,6);
        u8x8.print(k1,4);
        u8x8.drawString(0,7, "b=");
        u8x8.setCursor(4,7);
        u8x8.print(b1,4);
        showMode =0;  //only show the result once
        delay(10);

        break;

    }

  }

//least square curve fitting y=kx+b
float LScurveFitting(float* x, float* y, int n, float* a, float* b)
{
	float sumx = 0.0;
	float sumy = 0.0;
	float sumx2 = 0.0;
  float sumy2 = 0.0;
	float sumxy = 0.0;
  float errorSquare = 0.0;
  float rSquare = 0.0;
	for (int i = 0; i < n; i++)
	{
		sumx += x[i];
		sumy += y[i];
		sumx2 += x[i] * x[i];
		sumxy += x[i] * y[i];
    sumy2 += y[i] * y[i];
	}
	*a = (n * sumxy - sumx * sumy) / (n * sumx2 - sumx * sumx);
	*b = (sumy - *a * sumx) / n;
  for(int i=0;i<n;i++)
  {
    errorSquare += (y[i]-(*a*x[i]+*b))*(y[i]-(*a*x[i]+*b));  //误差Q方,越小误差越小
  }
  rSquare = (n * sumxy - sumx * sumy) * (n * sumxy - sumx * sumy) / (n * sumx2 - sumx * sumx) / (n * sumy2 - sumy * sumy); //R方,决定系数越接近1越准确
  //Serial.print("errorSquare: ");
  //Serial.println(errorSquare,5);
  //Serial.print("rSquare: ");
  //Serial.println(rSquare,5);
  return rSquare;
}



void TMUX1108_Mode(int modeNum)
  {
    digitalWrite(EN1, HIGH);
    switch(modeNum)
    {
      case 1:  //选择S1
        digitalWrite(A0, LOW);
        digitalWrite(A1, LOW);
        digitalWrite(A2, LOW);
        break;
      case 2:  //选择S2
        digitalWrite(A0, HIGH);
        digitalWrite(A1, LOW);
        digitalWrite(A2, LOW);
        break;
      case 3:  //选择S3
        digitalWrite(A0, LOW);
        digitalWrite(A1, HIGH);
        digitalWrite(A2, LOW);
        break;
      case 4:  //选择S4
        digitalWrite(A0, HIGH);
        digitalWrite(A1, HIGH);
        digitalWrite(A2, LOW);
        break;
      case 5:  //选择S5
        digitalWrite(A0, LOW);
        digitalWrite(A1, LOW);
        digitalWrite(A2, HIGH);
        break;
      case 6:  //选择S6
        digitalWrite(A0, HIGH);
        digitalWrite(A1, LOW);
        digitalWrite(A2, HIGH);
        break;
      case 7:  //选择S7
        digitalWrite(A0, LOW);
        digitalWrite(A1, HIGH);
        digitalWrite(A2, HIGH);
        break;
      case 8:  //选择S8
        digitalWrite(A0, HIGH);
        digitalWrite(A1, HIGH);
        digitalWrite(A2, HIGH);
        break;
    }
  }

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
  return voltage;
}

void serialEvent() 
{
  while (Serial.available()) 
  {   
    char inChar = (char)Serial.read();// get the new byte:    
    inputString += inChar;// add it to the inputString:
    if (inChar == '\n') 
    {
      stringComplete = true;// if the incoming character is a newline, set a flag so the main loop can
    }
  }
}
