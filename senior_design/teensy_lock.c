#include teensy_lock.h

int usbState;

int serialTimer;
int pinTimer;
const int serialTimeout;
const int pinTimeout;
const char pinResetDigit = ;//*
const int pinLength=6;
char[pinLength] pinNum;

const int nids = 500;
int[nids] idNumbers;//how many to store??//get rid of magic number
const int swipeDataLength = 32;

//define all these
char* lockString;
char* unlockString;
char* sleepString;

const int interuptPin;
const int lockSensorPin;
const int openSensorPin;
const int batterySensorPin;

const int serialStartBit;
const int keyPressType;
const int swipePressType;
const int keyPressReadyBit;
const int swipeReadyBit;

int checkUSB(){return digitalRead(USBpin);}

void checkSwipe(char[swipeDataLength] swipeData){
	for(int i=0;i<nids;i++){
		if(strequals(idNumbers[i],swipeData)){
			if(!digitalRead(lockSensorPin))
				unlock();
			break;
		}
	}
}

char[pinLength] pinEntry; //set size and malloc if variable

void checkPin(char digit){
	static int pinIndex = 0;
	if(checkTimeout(&pinTimer, pinTimeout))
		pinIndex = 0;
	pinEntry[pinIndex] = digit;
	if(digit == pinResetDigit){
		setTimer(&pinTimer);
		if(pinIndex == pinLength-1 && strequals(pinNum, pinEntry) && !digitalRead(lockSensorPin))
			unlock();
		pinIndex = 0;
	}
}
//check if full pin is there then test if it is wrong, flash lights if wrong;
//if passes and locked unlock


void handleSerial(){
	int type;
	setTimeout(&serialTimer);
	while(Serial1.read() != serialStartBit){
		if(checkTimeout(&serialTimer, serialTimeout)) return;}
	while(type = Serial1.read()){//list possible types
		if(checkTimeout(&serialTimer, serialTimeout)) return;}
	if(type == keyPressType){
		while(Serial1.read() != keyPressReadyBit){
			if(checkTimeout(&serialTimer, serialTimeout)) return;}
		if(usbState)
			Keypad.println(formatKeypad(Serial1.read()));
		else
			checkPin(Serial1.read());
	}else if(type == swipePressType){
		int i=0;
		int read;
		char[swipeDataLength] swipeData;//check size
		while(Serial1.read() != swipeReadyBit){
		if(checkTimeout(&serialTimer, serialTimeout)) return;}
		while(read = Serial1.read()){
			swipeData[i] = (char)read; //FIXME
			i++;
			if(checkTimeout(&serialTimer, serialTimeout)) return;}
		if(usbState)
			Swipe.println(formatSwipe(swipeData));
		else
			checkSwipe(swipeData);

	}
}

char* formatKeypad(int readByte){}

char* formatSwipe(int readByte){}

int checkTimeout(int *timer, int timeout);
	if (time() - *timer > timeout)
		return 1;
	return 0;

void setTimer(int *timer){
	*timer = time();//FIXME
}

void sendSleep(){
	Serial1.print(sleepString);
	//check if there is a response
}

int getPinNum();
	//get pin from eeprom. 

void setPinNum();
	//write pin to eeprom.

//define command types here:
const lockType = 0;
const unlockType = 1;
const setPinType = 2;
const lockCheckType = 3;
const openCheckType = 4;

struct Command{
	int type;
	char[32] data;
}

void handleCommand(){
	struct Command cmd = (struct Command)usbCommand.read();
	if(cmd.type == lockType)
		lock();
	else if(cmd.type == unlockType)
		unlock();
	else if(cmd.type == setPinType){
		setPinType(cmd.data);
		pinNum = getPinNum();
	}else if(cmd.type == lockCheckType){
		Sensors.sendLockStatus(digitalRead(lockSensorPin));
	}else if(cmd.type == openCheckType){
		Sensors.sendOpenStatus(digitalRead(openSensorPin));
	}
}

int lock(){//send ack!!!
	Serial1.print(lockString);
	return digitalRead(lockSensorPin);

}//check if locked

int unlock(){//send ack!!!
	Serial1.print(unlockString);
	return !digitalRead(lockSensorPin);
}//check if unlocked


void setup(){
	//usbState = checkUSB();//check this periodically, send lock state with this
	timer = setTimer();
	Serial1.begin(19200);
	//Serial2.begin(19200);//possibly just digital
	pinNum = getPinNum();
}

void loop(){
	usbState = checkUSB();
	if(Serial1.available() > 0) {
        //setTimer(&serialTimer);
        handleSerial();
	}
	if(usbCommand.available() == sizeof(struct Command))
		handleCommand();
}

