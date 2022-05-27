#include <stdio.h>
#include <wiringPi.h>
#include <iostream>
using namespace std;

// BCM编码
const int pinPUL = 25;  // 驱动器PUL+

int setup() {
// BCM编码
// 如果使用其他编码方式，上面的pinPUL和pinDIR也需要相应修改
	if (wiringPiSetupGpio() != 0) { 
		printf("Wiringpi setup failed\n");
		return 0;
	}

	pinMode(26,OUTPUT);
	digitalWrite(26,HIGH);
	
	return 1;
}

int main() {
	if (!setup()) {
		return 1;
	}
	while(1){
	}
}
