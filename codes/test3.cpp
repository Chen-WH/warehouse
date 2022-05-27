#include <stdio.h>
#include <wiringPi.h>
#include <iostream>
using namespace std;

// BCM编码
const int pinPUL1 = 25;  // 驱动器PUL+/转盘1
const int pinDIR1 = 24;  // 驱动器DIR+/转盘1
const int pinPUL2 = 23;  // 驱动器PUL+/转盘2
const int pinDIR2 = 22;  // 驱动器DIR+/转盘2
const int pinPUL3 = 29;  // 驱动器PUL+/叉车
const int pinDIR3 = 28;  // 驱动器DIR+/叉车
// BCM编码
// 如果使用其他编码方式，上面的pinPUL和pinDIR也需要相应修改
int setup(){
        wiringPiSetup();	
	/*转盘1*/
	pinMode(pinPUL1, OUTPUT);
	digitalWrite(pinPUL1,HIGH);
	pinMode(pinDIR1, OUTPUT);
	/*转盘2*/
	pinMode(pinPUL2,OUTPUT);
	digitalWrite(pinPUL2,HIGH);
	pinMode(pinDIR2,OUTPUT);
	/*叉车*/
	pinMode(pinPUL3,OUTPUT);
	digitalWrite(pinPUL2,HIGH);
	pinMode(pinDIR3,OUTPUT);
	return 1;
}
void CW1() {
	digitalWrite(pinDIR1, LOW);
}
void CCW1() {
	digitalWrite(pinDIR1, HIGH);
}
void CW2(){
	digitalWrite(pinDIR2,LOW);
}
void CCW2(){
	digitalWrite(pinDIR2,HIGH);
}
void CW3(){
	digitalWrite(pinDIR3,LOW);
}
void CCW3(){
	digitalWrite(pinDIR3,HIGH);
}
// 发射一次脉冲
//  @param: delayMicroS (微秒)
// //    可以控制脉冲频率，进而控制电机转速
// //    参数值越大，每两次脉冲之间的延时越长，脉冲频率越低，转速越慢
// //    参数值越小，转速越快
// // 但是！尽量不要小于60
void pulseOnce(int delayMicroS,int pin) {
	digitalWrite(pin, LOW);
	delayMicroseconds(delayMicroS);
        digitalWrite(pin, HIGH);
        delayMicroseconds(delayMicroS);
}
// 发射count次脉冲
void pulse(int count, int delayMicroS,int pin) {	
	for (int i = 0; i < count; ++i) {
     	   pulseOnce(delayMicroS,pin);
     	}
}
//15000circle1
//
int circle = 15000 ;
//1 move 1066.67 pulse 
void move(int move,int pin){
	int ang = move*30;
	/*12°加速*/
	for(int i = 0;i<8; i++)
		pulse (circle/4/360,400*(8-1*i),pin);
	/*匀速*/
	pulse (circle*(ang-4)/360+15,400,pin);
	/*12°减速*/
	for(int i =0;i<8; i++)
		pulse(circle/4/360,400*(i+1),pin);
}
//CW3 向下
int main() {
	if (!setup()) {
		return 1;
	}
	// 设置为顺时针转动
  	CCW3();
	move(12,pinPUL3);

}
