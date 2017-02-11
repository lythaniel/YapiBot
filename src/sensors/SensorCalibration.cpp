/*
 * SensorCalibration.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <SensorCalibration.h>
#include <math.h>
#include <SensorFactory.h>

#define MAX_ITERATION 200
#define LIMIT_CONV_FUNCT 0.0
#define LIMIT_CONV_ROOT 0.0

#define NUM_SAMPLE_AVERAGE 500.0
#define NUM_MEASURE 500


using namespace Eigen;



void CSensorCalibration::runCalibration(void)
{
	avgAcc_t avgMagField = {0.0, 0.0, 0.0};
	avgAcc_t avgAccel = {0.0, 0.0, 0.0};
	avgAcc_t avgAngRate = {0.0, 0.0, 0.0};
	char str [2];

	MatrixXd MeasAccel(6,3);
	MatrixXd MeasCompass(6,3);

	printf ("Starting sensor calibration ...");

	CCompass * compass = CSensorFactory::getInstance()->getCompass();
	CAccelerometer * accel = CSensorFactory::getInstance()->getAccelerometer();
	CGyroscope	* gyro = CSensorFactory::getInstance()->getGyroscope();

	if (compass != NULL)
	{
		compass->startCompassCalibration();
	}
	if (accel != NULL)
	{
		accel->startAccelCalibration();
	}
	if (gyro != NULL)
	{
		gyro->startGyroCalibration();
	}


	/****************************************/
	/*			First measurement 			*/
	/****************************************/


	printf ("Set YapiBot flat on the floor then press enter ...\n");

	scanf ("%1s",str);

	printf ("Measuring, please wait ...\n");
	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(0,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(0,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(0,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(0,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(0,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(0,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	avgMagField.x = 0;
	avgMagField.y = 0;
	avgMagField.z = 0;

	avgAccel.x = 0;
	avgAccel.y = 0;
	avgAccel.z = 0;

	/****************************************/
	/*			2nd measurement 			*/
	/****************************************/

	printf ("Set YapiBot reverse on the floor then press a key ...\n");
	scanf ("%1s",str);
	printf ("Measuring, please wait ...\n");

	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(1,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(1,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(1,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(1,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(1,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(1,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	avgMagField.x = 0;
	avgMagField.y = 0;
	avgMagField.z = 0;

	avgAccel.x = 0;
	avgAccel.y = 0;
	avgAccel.z = 0;

	/****************************************/
	/*			3rd measurement 			*/
	/****************************************/

	printf ("Set YapiBot with the front pointing upward and then press a key ...\n");
	scanf ("%1s",str);
	printf ("Measuring, please wait ...\n");

	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(2,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(2,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(2,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(2,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(2,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(2,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	avgMagField.x = 0;
	avgMagField.y = 0;
	avgMagField.z = 0;

	avgAccel.x = 0;
	avgAccel.y = 0;
	avgAccel.z = 0;

	/****************************************/
	/*			4th measurement 			*/
	/****************************************/

	printf ("Set YapiBot with the front pointing downward and then press a key ...\n");
	scanf ("%1s",str);
	printf ("Measuring, please wait ...\n");

	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(3,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(3,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(3,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(3,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(3,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(3,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	avgMagField.x = 0;
	avgMagField.y = 0;
	avgMagField.z = 0;

	avgAccel.x = 0;
	avgAccel.y = 0;
	avgAccel.z = 0;

	/****************************************/
	/*			5th measurement 			*/
	/****************************************/

	printf ("Set YapiBot on side with the front pointing left and then press a key ...\n");
	scanf ("%1s",str);
	printf ("Measuring, please wait ...\n");

	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(4,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(4,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(4,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(4,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(4,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(4,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	avgMagField.x = 0;
	avgMagField.y = 0;
	avgMagField.z = 0;

	avgAccel.x = 0;
	avgAccel.y = 0;
	avgAccel.z = 0;

	/****************************************/
	/*			6th measurement 			*/
	/****************************************/

	printf ("Set YapiBot on side with the front pointing right and then press a key ...\n");
	scanf ("%1s",str);
	printf ("Measuring, please wait ...\n");

	//Flush sensors.
	flushSensors();
	//Measure.
	measureAndIntegrate(&avgMagField, &avgAngRate, &avgAccel);

	MeasCompass(5,0) = avgMagField.x;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(5,1) = avgMagField.y;// / NUM_SAMPLE_AVERAGE;
	MeasCompass(2,2) = avgMagField.z;// / NUM_SAMPLE_AVERAGE;

	MeasAccel(5,0) = avgAccel.x;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(5,1) = avgAccel.y;// / NUM_SAMPLE_AVERAGE;
	MeasAccel(5,2) = avgAccel.z;// / NUM_SAMPLE_AVERAGE;

	//avgAngRate.x /= (6*NUM_SAMPLE_AVERAGE);
	//avgAngRate.y /= (6*NUM_SAMPLE_AVERAGE);
	//avgAngRate.z /= (6*NUM_SAMPLE_AVERAGE);

	printf ("Done ! (avg mag field = %f/%f/%f avg Accel = %f,%f,%f)\n", avgMagField.x, avgMagField.y, avgMagField.z, avgAccel.x, avgAccel.y, avgAccel.z);

	MatrixXd compassAlign = solve (MeasCompass);
	MatrixXd accelAlign = solve (MeasAccel);
	printf ("Computing alignment ....\n");

	if (compass != NULL)
	{
		sMagField offset;
		sMagField scale;
		offset.x = -compassAlign(0,0);
		offset.y = -compassAlign(1,0);
		offset.z = -compassAlign(2,0);
		scale.x = compassAlign(3,0);
		scale.y = compassAlign(4,0);
		scale.z = compassAlign(5,0);
		compass->stopCompassCalibration(offset, scale);
	}
	if (accel != NULL)
	{
		sAccel offset;
		sAccel scale;
		offset.x = -accelAlign(0,0);
		offset.y = -accelAlign(1,0);
		offset.z = -accelAlign(2,0);
		scale.x = accelAlign(3,0);
		scale.y = accelAlign(4,0);
		scale.z = accelAlign(5,0);
		accel->stopAccelCalibration(offset, scale);
	}
	if (gyro != NULL)
	{
		sAngularRate offset;
		sAngularRate scale;
		offset.x = -avgAngRate.x;
		offset.y = -avgAngRate.y;
		offset.z = -avgAngRate.z;
		scale.x = 1.0f;
		scale.y = 1.0f;
		scale.z = 1.0f;
		gyro->stopGyroCalibration(offset, scale);
	}

	printf ("Calibration complete !\n");

}

void CSensorCalibration::flushSensors (void)
{
	CCompass * compass = CSensorFactory::getInstance()->getCompass();
	CAccelerometer * accel = CSensorFactory::getInstance()->getAccelerometer();
	CGyroscope	* gyro = CSensorFactory::getInstance()->getGyroscope();

	if (compass != NULL)
	{
		while (compass->magFieldAvailable())
		{
			compass->getMagField();
		}
	}
	if (gyro != NULL)
	{
		while (gyro->angRateSamplesAvailable())
		{
			gyro->getAngularRate();
		}
	}
	if (accel != NULL)
	{
		while (accel->accelSamplesAvailable())
		{
			accel->getAccel();
		}
	}

}

void CSensorCalibration::measureAndIntegrate (avgAcc_t * avgMagField, avgAcc_t * avgAngRate, avgAcc_t * avgAccel)
{
	sMagField magField;
	sAccel acc;
	sAngularRate angRate;

	double magFieldAccX = 0, magFieldAccY = 0, magFieldAccZ = 0;
	double accelAccX = 0, accelAccY = 0, accelAccZ = 0;
	//double angRateAccX = 0, angRateAccY = 0, angRateAccZ = 0,

	CCompass * compass = CSensorFactory::getInstance()->getCompass();
	CAccelerometer * accel = CSensorFactory::getInstance()->getAccelerometer();
	CGyroscope	* gyro = CSensorFactory::getInstance()->getGyroscope();

	//Start measuring and integrating.
	for (int i = 0; i < NUM_MEASURE; i++)
	{
		if (compass != NULL)
		{
			//Wait for measurement to be available.
			while (!compass->magFieldAvailable());
			magField = compass->getMagField();
			magFieldAccX += (double)magField.x / NUM_SAMPLE_AVERAGE;
			magFieldAccY += (double)magField.y / NUM_SAMPLE_AVERAGE;
			magFieldAccZ += (double)magField.z / NUM_SAMPLE_AVERAGE;
		}
		if (gyro != NULL)
		{
			//Wait for measurement to be available.
			while (!gyro->angRateSamplesAvailable());
			angRate = gyro->getAngularRate();
			avgAngRate->x += (double)angRate.x / (NUM_SAMPLE_AVERAGE*6);
			avgAngRate->y += (double)angRate.y / (NUM_SAMPLE_AVERAGE*6);
			avgAngRate->z += (double)angRate.z / (NUM_SAMPLE_AVERAGE*6);
		}
		if (accel != NULL)
		{
			//Wait for measurement to be available.
			while (!accel->accelSamplesAvailable());
			acc = accel->getAccel();
			accelAccX += (double)acc.x / NUM_SAMPLE_AVERAGE;
			accelAccY += (double)acc.y / NUM_SAMPLE_AVERAGE;
			accelAccZ += (double)acc.z / NUM_SAMPLE_AVERAGE;
		}
	}
	avgMagField->x = magFieldAccX;
	avgMagField->y = magFieldAccY;
	avgMagField->z = magFieldAccZ;
	avgAccel->x = accelAccX;
	avgAccel->y = accelAccY;
	avgAccel->z = accelAccZ;


}

double CSensorCalibration::partialDerivateOffset (double val, double offset, double scale)
{
	double ret =  2.0 * (val * scale + offset);
	return ret;
}

double CSensorCalibration::partialDerivateScale (double val, double offset, double scale)
{
	double ret =  2.0 * (val * val * scale + val * offset);
	return ret;
}

MatrixXd CSensorCalibration::solve (MatrixXd & Meas)
{
	MatrixXd jacobian(6,6);
	MatrixXd X(6,1);
	MatrixXd F(6,1);
	int32_t iter;

	//Init solution
	X << 0.0, 0.0, 0.0, 1.0, 1.0, 1.0;

	for (iter = 0; iter < MAX_ITERATION; iter++)
	{
		for (int i = 0; i < 6; i++)
		{
			for (int k = 0; k < 3; k++)
			{
				jacobian(i,k) = partialDerivateOffset(Meas(i,k),X(k,0),X(3+k,0));
				jacobian(i,3+k) = partialDerivateScale(Meas(i,k),X(k,0),X(3+k,0));
			}
		}
		//std::cout << "Jacobian:" << std::endl << jacobian << std::endl;
		for (int i = 0; i < 6; i++)
		{
			F(i,0) = pow(Meas(i,0)*X(3,0) + X(0,0), 2) + pow(Meas(i,1)*X(4,0) + X(1,0), 2) + pow(Meas(i,2)*X(5,0) + X(2,0), 2) - 1;
		}

		/*if (abs(F.sum()) <= LIMIT_CONV_FUNCT)
		{
			break;
		}*/
		//std::cout << "F:" << std::endl << F << std::endl;

		MatrixXd Xd = jacobian.lu().solve(-F);

		/*if (abs(Xd.sum()) <= LIMIT_CONV_ROOT)
		{
			break;
		}*/
		//std::cout << "Xd:" << std::endl << Xd << std::endl;

		X = X + Xd;

		//std::cout << "X:" << std::endl << X << std::endl;

	}
	return X;

}
