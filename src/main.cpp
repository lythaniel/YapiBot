/*
 * main.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

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
#include "SensorFactory.h"
#include "EventObserver.h"
#include "ScriptEngine.h"
#include "Mapper.h"
#include "Settings.h"

#define MAIN_TEXTURE_WIDTH 640
#define MAIN_TEXTURE_HEIGHT 480
#define FRAMERATE 30



//entry point
int main(int argc, const char **argv)
{
	//Create Settings manager.
	CSettings::getInstance();

	//Create Event Observer
	CEventObserver::getInstance();

	//Create the network.
	CNetwork::getInstance()->start();

	//Create the video processing.
	CImageProcessing::getInstance()->init(MAIN_TEXTURE_WIDTH,MAIN_TEXTURE_HEIGHT,FRAMERATE);

	//Create I2C bus:
	CI2Cbus i2c(1);

	//Create compass
	CCompass * compass = CSensorFactory::getInstance()->createCompass(COMPASS_LSM9DS1);
	if (compass != NULL)
	{
		compass->setBus(&i2c);
	}

	//Create range finder
	CRangeFinder * rangeFinder = CSensorFactory::getInstance()->createRangeFinder(RANGEFINDER_2Y0A21);
	if (rangeFinder != NULL)
	{
		rangeFinder->setBus(&i2c);
	}

	//Create linear accelerometer
	CLinAccel * linAccel = CSensorFactory::getInstance()->createLinAccel(LINACCEL_LSM9DS1);
	if (linAccel != NULL)
	{
		linAccel->setBus(&i2c);
	}

	//Create the mapper.
	CMapper::getInstance();

	//Create the motors
	CMotors::getInstance();


	//Finally create our main controler.
	CController::getInstance();

	//Script engine:
	CScriptEngine::getInstance();

	CSemaphore deadlock;
	deadlock.wait (); //Semaphore are created with value 0, so it will wait forever.

	return 0;

}





