#include "PS2X_lib.h"  //for v1.6
#include <Wire.h>
#include <Servo.h>
/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        6  
#define PS2_CMD        5  
#define PS2_SEL        4 
#define PS2_CLK        3 

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
#define pressures   true 
#define rumble      true

//todo Надо все китайские комментарии переписать на русский или английский.
PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

int lx = 0;
int ly=  0;
int L_Speed = 0;
int R_Speed = 0;

Servo myservo1,myservo2,myservo3,myservo4;  // create servo object to control a servo
const int SERVOS = 4;  //舵机数4个

const int ACC = 10; // the accurancy of the potentiometer value before idle starts counting
int  value[SERVOS], idle[SERVOS], currentAngle[SERVOS], MIN[SERVOS], MAX[SERVOS], INITANGLE[SERVOS], previousAngle[SERVOS],ANA[SERVOS];

//todo Надо все управление мотор-колесами переписать на данные переменные (сделать константами), а то везьде используются "магические" числа, но не эти переменные.
//todo Надо все методы управления мотор-колесами и их инициализацию переписать на C++ класс, а здесь использовать.
int Left_motor=8;     //Левый мотор (IN3) выход 0 - вперед, выход 1 - назад
int Left_motor_pwm=9;     //ШИМ-регулировка скорости левого мотора

int Right_motor_pwm=10;    // ШИМ-регулировка скорости правого мотора
int Right_motor=11;    // Правый мотор назад (IN1) выход 0 - вперед, выход 1 - назад


void setup(){
  Serial.begin(57600);
  
   myservo1.attach(2);  // Когтевой двигатель
   myservo2.attach(7);  // Мотор верхнего плеча (правый Серво)
   myservo3.attach(12); // Двигатель нижнего рычага (левый Серво)
   myservo4.attach(13); // Базовый мотор (Серво внизу, вправо-влево) 
   
  //-----电机IO口定-
  pinMode( 8, OUTPUT);
  pinMode( 9, OUTPUT);
  pinMode( 10, OUTPUT);
  pinMode( 11, OUTPUT);
  stop();   
  //手爪 Servo
  MIN[0] = 10;
  MAX[0] = 50;
 INITANGLE[0] = 30;

  
  //上臂电机
  MIN[1] = 10; // This should bring the lever to just below 90deg to ground
  MAX[1] = 140;
  INITANGLE[1] = 90; // This should bring the lever parallel with the ground

  
  //下臂电机
  MIN[2] = 40;
  MAX[2] = 170;
  INITANGLE[2] = 90;

  
  //底座电机
  MIN[3] = 0;
  MAX[3] = 170;
  INITANGLE[3] = 90;
 
  //初始化电机
  myservo1.write(INITANGLE[0]);  
  myservo2.write(INITANGLE[1]);  
  myservo3.write(INITANGLE[2]);
  myservo4.write(INITANGLE[3]);  

   currentAngle[0]=INITANGLE[0];
   currentAngle[1]=INITANGLE[1];
   currentAngle[2]=INITANGLE[2];
   currentAngle[3]=INITANGLE[3];
   
  delay(2000);  //added delay to give wireless ps2 module some time to startup, before configuring it
  Serial.print("Search Controller..");
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  do{
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    if(error == 0){
      Serial.println("\nConfigured successful ");
      break;
    }else{
      Serial.print(".");
      delay(100);
    }
  }while(1);  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.println("Unknown Controller type found ");
      break;
    case 1:
      Serial.println("DualShock Controller found ");
      break;
    case 2:
      Serial.println("GuitarHero Controller found ");
      break;
    case 3:
      Serial.println("Wireless Sony DualShock Controller found ");
      break;
   }
  //震动一，说明连接
   ps2x.read_gamepad(true, 200);  //开启震动
   delay(500);
   ps2x.read_gamepad(false, 200); //开启震动
   delay(300);
   ps2x.read_gamepad(true, 200);  //开启震动
   delay(500);
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
    ps2x.read_gamepad(false, vibrate);  //开启震动
    
    if(ps2x.Button(PSB_START)){         //开始按键
      Serial.println("Start is being held");
      ps2x.read_gamepad(true, 200);
    }
    if(ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");      
    if(ps2x.Button(PSB_PAD_UP))  //方向按键向上按下
    {                 
      Serial.print("Up held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_UP), DEC);
      forward();
       
    }else if(ps2x.ButtonReleased(PSB_PAD_UP))  {   //方向按键向上释放
        stop();      
    }
    if(ps2x.Button(PSB_PAD_DOWN))   //方向按键向下按下
    {            
      Serial.print("DOWN held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
      back();
    }
    else if(ps2x.ButtonReleased(PSB_PAD_DOWN))//方向按键向下释放
    {  
        stop();  
    }
    if(ps2x.Button(PSB_R1)){                          // Боковой переключатель правый R1
      R_Speed = 0;                 
    }
    if(ps2x.Button(PSB_L1)){                          // Боковой переключатель левый L1
      L_Speed = 0;               
    } 
    if(ps2x.Button(PSB_PAD_LEFT)) //方向按键左侧按下  
    {                          
      left();
    }else if(ps2x.ButtonReleased(PSB_PAD_LEFT))  //方向按键左侧释放
    {    
       stop();
    }
    if(ps2x.Button(PSB_PAD_RIGHT))   //方向按键右侧按下
    {                   
      Serial.println(ps2x.Analog(PSB_PAD_RIGHT));
        right();
    }
    else if(ps2x.ButtonReleased(PSB_PAD_RIGHT))  //方向按键右侧释放
    {   
//     Serial.println("*****PSB_PAD_RIGHT***** ");
      stop();     
    }   
    vibrate = ps2x.Analog(PSAB_CROSS);  
    //this will set the large motor vibrate speed based on how hard you press the blue (X) button
    {
      
    }
  
    if (ps2x.NewButtonState()) {        //will be TRUE if any button changes state (on to off, or off to on)
      if(ps2x.Button(PSB_L3))         //左侧L3
        Serial.println("L3 pressed");
      if(ps2x.Button(PSB_R3))         //右侧L3
        Serial.println("R3 pressed");
      if(ps2x.Button(PSB_L2))         //左侧L2
        Serial.println("L2 pressed");
      if(ps2x.Button(PSB_R2))         //右侧R2
        Serial.println("R2 pressed");
      if(ps2x.Button(PSB_TRIANGLE))
        Serial.println("Triangle pressed");   //三角形按键按下     
    }
    if(ps2x.ButtonPressed(PSB_CIRCLE))
    {               
      //will be TRUE if button was JUST pressed
      Serial.println("Circle just pressed");
      openGripper(); 
    }
    if(ps2x.NewButtonState(PSB_CROSS)) {      // ×键新键改变
      Serial.println("X just changed");
      ps2x.read_gamepad(true, vibrate);       //获取震动值
     }
    if(ps2x.ButtonPressed(PSB_SQUARE)) 
    {             
      //will be TRUE if button was JUST released
      Serial.println("Square just released");
      closeGripper() ;
    }
    if(ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) { //print stick values if either is TRUE
      Serial.print("Stick Values:");
      Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_LX), DEC); 
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_RY), DEC); 
      Serial.print(",");
      Serial.println(ps2x.Analog(PSS_RX), DEC);      
    }     
 
    value[0] = ps2x.Analog(PSS_LX);
    value[1] = ps2x.Analog(PSS_RY);
    value[2] = ps2x.Analog(PSS_LY);
    value[3] = ps2x.Analog(PSS_RX);
  
   for (int i = 0; i < SERVOS; i++)
   {
      // для базового Серво (вправо-влево) меняем направление на зеркальное
      if (i == 3) {
        if (value[i] > 130) {
          if (currentAngle[i] > MIN[i]) 
            currentAngle[i] -= 1;
        } else {
          if (currentAngle[i] < MAX[i]) 
            currentAngle[i] += 1;
        }
        switch(i)
        {    
          case 0:  myservo1.write(currentAngle[i]);break;
          case 1:  myservo2.write(currentAngle[i]);break;
          case 2:  myservo3.write(currentAngle[i]);break;
          case 3:  myservo4.write(currentAngle[i]);break;
        }   
      } else {
        // для остальных Серво приводов делаем как и раньше было
        if (value[i] > 130) 
        {
          if (currentAngle[i] < MAX[i]) 
          currentAngle[i]+=1;
  //          Serial.print("value"); 
  //          Serial.print(i); 
  //          Serial.print(":"); 
  //          Serial.println(value[i]);
            switch(i)
            {
                
              case 0:  myservo1.write(currentAngle[i]);break;
              case 1:  myservo2.write(currentAngle[i]);break;
              case 2:  myservo3.write(currentAngle[i]);break;
              case 3:  myservo4.write(currentAngle[i]);break;
            }   
        } 
      else if (value[i] < 120) 
      {
        if (currentAngle[i] > MIN[i]) currentAngle[i]-=1;
  //        Serial.print("value"); 
  //        Serial.print(i); 
  //        Serial.print(":"); 
  //        Serial.println(value[i]);
         switch(i)
        {
         case 0:  myservo1.write(currentAngle[i]);break;
         case 1:  myservo2.write(currentAngle[i]);break;
         case 2:  myservo3.write(currentAngle[i]);break;
         case 3:  myservo4.write(currentAngle[i]);break;
        }   
      }
    }  
  }  
  delay(10);
}

//Grab something
void openGripper() {
  myservo1.write(MIN[0]);
  delay(300);
}

//Let go of something
void closeGripper() {
   myservo1.write(MAX[0]);
  delay(300);
}

void forward()
{
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
   
  digitalWrite(11, LOW); 
  digitalWrite(10, HIGH);
}

void right()
{
   digitalWrite(8, LOW);
  digitalWrite(9, HIGH);
  
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
}

void back()
{
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  
  digitalWrite(11, HIGH);
  digitalWrite(10, HIGH);
}

void left()
{
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  
  digitalWrite(11, LOW);
  digitalWrite(10, HIGH);
}

void stop()
{
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  
  digitalWrite(11, LOW);
  digitalWrite(10, LOW); 
}
