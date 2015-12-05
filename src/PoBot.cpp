#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "Motors.h"
#include "Network.h"
#include "Controller.h"
#include "ImageProcessing.h"
#include "Semaphore.h"
#include "Motors.h"
#include "I2Cbus.h"
#include "Compass.h"
#include "RangeFinder.h"
#include "EventObserver.h"
#include "ScriptEngine.h"

#define MAIN_TEXTURE_WIDTH 640
#define MAIN_TEXTURE_HEIGHT 480
#define FRAMERATE 30



//entry point
int main(int argc, const char **argv)
{
	//Create Event Observer
	CEventObserver::getInstance();

	//Create the network.
	CNetwork::getInstance()->start();

	//Create the video processing.
	CImageProcessing::getInstance()->init(MAIN_TEXTURE_WIDTH,MAIN_TEXTURE_HEIGHT,FRAMERATE);

	//Create I2C bus:
	CI2Cbus i2c(1);

	//Create compass
	CCompass::getInstance()->setI2Cbus(&i2c);

	//Create range finder
	CRangeFinder::getInstance()->setI2Cbus(&i2c);

	//Create the motors
	CMotors::getInstance();


	//Finally create our main controler.
	CController::getInstance();

	//Script engine:
	CScriptEngine::getInstance();

	CSemaphore deadlock;
	deadlock.wait (); //Semaphore are created with value 0, so it will wait forever.

}





