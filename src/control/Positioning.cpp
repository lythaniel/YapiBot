/*
 * Positioning.cpp
 *
 *
 * Simple C++ wrapper of Sebastian Madgwick's AHRS algorithm with modification
 * to the invSqrt method to be compatible with 64bits devices.
 * more info at: http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Positioning.h>
#include <math.h>
#include <string.h>
#include "Network.h"
#include "Settings.h"
#include "Utils.h"

#define A1 0
#define A2 1
#define B0 2
#define B1 3
#define B2 4

#define AHR_FLTGAIN_DEFAULT 5.0f

CPositioning::CPositioning() :
sampleFreq(119.0f),
beta(AHR_FLTGAIN_DEFAULT),
AccBiasFact(0.001f)
{
	m_Orient.q0 = 1.0f;
	m_Orient.q1 = 0.0f;
	m_Orient.q2 = 0.0f;
	m_Orient.q3 = 0.0f;
	m_Speed.x = 0;
	m_Speed.y = 0;
	m_Speed.z = 0;
	m_AccBiasComp.x = 0;
	m_AccBiasComp.y = 0;
	m_AccBiasComp.z = 0;
	m_Pos.x = 0;
	m_Pos.y = 0;
	m_Pos.z = 0;
	memset (m_FltDelay, 0x00, sizeof (m_FltDelay));
	m_FltCoeff[A1] = -1.998222847291841740f; //2nd order HP IIR butterworth fc=5.95Hz @ Fs = 119Hz
	m_FltCoeff[A2] = 0.998224425026400519f;
	m_FltCoeff[B0] = 0.999111818079560621f;
	m_FltCoeff[B1] =  -1.998223636159121240f;
	m_FltCoeff[B2] =  0.999111818079560621f;

	beta = CSettings::getInstance()->getFloat("POSITIONNING", "AHR Filter gain", AHR_FLTGAIN_DEFAULT);

}

CPositioning::~CPositioning() {

}

void CPositioning::update (sAngularRate angRate, sAccel accel, sMagField field)
{
	float32_t recipNorm;
	float32_t s0, s1, s2, s3;
	float32_t qDot1, qDot2, qDot3, qDot4;
	float32_t hx, hy;
	float32_t _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

	Vector3 accelVect;
	accelVect.x = accel.x;
	accelVect.y = accel.y;
	accelVect.z = accel.z;

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((field.x == 0.0f) && (field.y == 0.0f) && (field.z == 0.0f)) {
		update(angRate, accel);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-m_Orient.q1 * angRate.x - m_Orient.q2 * angRate.y - m_Orient.q3 * angRate.z);
	qDot2 = 0.5f * (m_Orient.q0 * angRate.x + m_Orient.q2 * angRate.z - m_Orient.q3 * angRate.y);
	qDot3 = 0.5f * (m_Orient.q0 * angRate.y - m_Orient.q1 * angRate.z + m_Orient.q3 * angRate.x);
	qDot4 = 0.5f * (m_Orient.q0 * angRate.z + m_Orient.q1 * angRate.y - m_Orient.q2 * angRate.x);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((accel.x == 0.0f) && (accel.y == 0.0f) && (accel.z == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
		accel.x *= recipNorm;
		accel.y *= recipNorm;
		accel.z *= recipNorm;

		// Normalise magnetometer measurement
		recipNorm = invSqrt(field.x * field.x + field.y * field.y + field.z * field.z);
		field.x *= recipNorm;
		field.y *= recipNorm;
		field.z *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * m_Orient.q0 * field.x;
		_2q0my = 2.0f * m_Orient.q0 * field.y;
		_2q0mz = 2.0f * m_Orient.q0 * field.z;
		_2q1mx = 2.0f * m_Orient.q1 * field.x;
		_2q0 = 2.0f * m_Orient.q0;
		_2q1 = 2.0f * m_Orient.q1;
		_2q2 = 2.0f * m_Orient.q2;
		_2q3 = 2.0f * m_Orient.q3;
		_2q0q2 = 2.0f * m_Orient.q0 * m_Orient.q2;
		_2q2q3 = 2.0f * m_Orient.q2 * m_Orient.q3;
		q0q0 = m_Orient.q0 * m_Orient.q0;
		q0q1 = m_Orient.q0 * m_Orient.q1;
		q0q2 = m_Orient.q0 * m_Orient.q2;
		q0q3 = m_Orient.q0 * m_Orient.q3;
		q1q1 = m_Orient.q1 * m_Orient.q1;
		q1q2 = m_Orient.q1 * m_Orient.q2;
		q1q3 = m_Orient.q1 * m_Orient.q3;
		q2q2 = m_Orient.q2 * m_Orient.q2;
		q2q3 = m_Orient.q2 * m_Orient.q3;
		q3q3 = m_Orient.q3 * m_Orient.q3;

		// Reference direction of Earth's magnetic field
		hx = field.x * q0q0 - _2q0my * m_Orient.q3 + _2q0mz * m_Orient.q2 + field.x * q1q1 + _2q1 * field.y * m_Orient.q2 + _2q1 * field.z * m_Orient.q3 - field.x * q2q2 - field.x * q3q3;
		hy = _2q0mx * m_Orient.q3 + field.y * q0q0 - _2q0mz * m_Orient.q1 + _2q1mx * m_Orient.q2 - field.y * q1q1 + field.y * q2q2 + _2q2 * field.z * m_Orient.q3 - field.y * q3q3;
		_2bx = sqrt(hx * hx + hy * hy);
		_2bz = -_2q0mx * m_Orient.q2 + _2q0my * m_Orient.q1 + field.z * q0q0 + _2q1mx * m_Orient.q3 - field.z * q1q1 + _2q2 * field.y * m_Orient.q3 - field.z * q2q2 + field.z * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - accel.x) + _2q1 * (2.0f * q0q1 + _2q2q3 - accel.y) - _2bz * m_Orient.q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - field.x) + (-_2bx * m_Orient.q3 + _2bz * m_Orient.q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - field.y) + _2bx * m_Orient.q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - field.z);
		s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - accel.x) + _2q0 * (2.0f * q0q1 + _2q2q3 - accel.y) - 4.0f * m_Orient.q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - accel.z) + _2bz * m_Orient.q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - field.x) + (_2bx * m_Orient.q2 + _2bz * m_Orient.q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - field.y) + (_2bx * m_Orient.q3 - _4bz * m_Orient.q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - field.z);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - accel.x) + _2q3 * (2.0f * q0q1 + _2q2q3 - accel.y) - 4.0f * m_Orient.q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - accel.z) + (-_4bx * m_Orient.q2 - _2bz * m_Orient.q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - field.x) + (_2bx * m_Orient.q1 + _2bz * m_Orient.q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - field.y) + (_2bx * m_Orient.q0 - _4bz * m_Orient.q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - field.z);
		s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - accel.x) + _2q2 * (2.0f * q0q1 + _2q2q3 - accel.y) + (-_4bx * m_Orient.q3 + _2bz * m_Orient.q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - field.x) + (-_2bx * m_Orient.q0 + _2bz * m_Orient.q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - field.y) + _2bx * m_Orient.q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - field.z);
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	m_Orient.q0 += qDot1 * (1.0f / sampleFreq);
	m_Orient.q1 += qDot2 * (1.0f / sampleFreq);
	m_Orient.q2 += qDot3 * (1.0f / sampleFreq);
	m_Orient.q3 += qDot4 * (1.0f / sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(m_Orient.q0 * m_Orient.q0 + m_Orient.q1 * m_Orient.q1 + m_Orient.q2 * m_Orient.q2 + m_Orient.q3 * m_Orient.q3);
	m_Orient.q0 *= recipNorm;
	m_Orient.q1 *= recipNorm;
	m_Orient.q2 *= recipNorm;
	m_Orient.q3 *= recipNorm;


	//Rotate gravity vector by our orientation
	Vector3 gravity = {0.0f, 0.0f, 1.0f};
	Quaternion rot;

	rot.q0 = m_Orient.q0;
	rot.q1 = -m_Orient.q1;
	rot.q2 = -m_Orient.q2;
	rot.q3 = -m_Orient.q3;

	Vector3 gravCapt = rotateVector(gravity,rot);

	accelVect.x -= (gravCapt.x + m_AccBiasComp.x);
	accelVect.y -= (gravCapt.y + m_AccBiasComp.y);
	accelVect.z -= (gravCapt.z + m_AccBiasComp.z);


	double accelnorm = sqrt(accelVect.x * accelVect.x + accelVect.y * accelVect.y + accelVect.z * accelVect.z);
	if (accelnorm < 0.05f)
	{
		m_AccBiasComp.x += (accelVect.x * AccBiasFact);
		m_AccBiasComp.y += (accelVect.y * AccBiasFact);
		m_AccBiasComp.z += (accelVect.z * AccBiasFact);
	}

	//float32_t accelnorm = (accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
	Vector3 acc = rotateVector (accelVect, m_Orient);


	/*Vector3 acc,acc_flt;
	acc.x = ((accel_rot.x - gravity.x) );
	acc.y = ((accel_rot.y - gravity.y) );
	acc.z = ((accel_rot.z - gravity.z) );*/




	m_Acceleration.x = acc.x;// * 9810.0f; //mm/s2
	m_Acceleration.y = acc.y;// * 9810.0f;
	m_Acceleration.z = acc.z;// * 9810.0f;


	m_Speed.x += m_Acceleration.x / sampleFreq; // mm/s
	m_Speed.y += m_Acceleration.y / sampleFreq;
	m_Speed.z += m_Acceleration.z / sampleFreq;


	float32_t speednormflt = sqrt(m_Speed.x * m_Speed.x + m_Speed.y * m_Speed.y + m_Speed.z * m_Speed.z);
	m_Pos.x += m_Speed.x / sampleFreq; // mm
	m_Pos.y += m_Speed.y / sampleFreq; // mm
	m_Pos.z += m_Speed.z / sampleFreq; // mm
	//float32_t posnormflt = sqrt(m_Pos.x * m_Pos.x + m_Pos.y * m_Pos.y + m_Pos.z * m_Pos.z);

	//printf ("[POSITIONING] flt acc norm = %f, flt speed norm = %f, flt pos norm = %f\n",accelnormflt, speednormflt, posnormflt);
	//printf ("[POSITIONING] Estimated acceleration: norm = %f,  x: %f y: %f, z: %f\n",accelnorm, m_Acceleration.x, m_Acceleration.y, m_Acceleration.z);
	//printf ("%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f\n",accelVect.x, accelVect.y, accelVect.z,acc.x, acc.y, acc.z,m_Acceleration.x, m_Acceleration.y, m_Acceleration.z, m_Speed.x, m_Speed.y, m_Speed.z);
	//printf ("[POSITIONING] Estimated speed: norm: %f x: %f y: %f, z: %f\n",speednormflt, m_Speed.x, m_Speed.y, m_Speed.z);
	//printf ("[POSITIONING] Estimated pos: x: %f y: %f, z: %f\n",m_Pos.x, m_Pos.y, m_Pos.z);
}

//---------------------------------------------------------------------------------------------------
// IMU algorithm update

void CPositioning::update (sAngularRate angRate, sAccel accel)
{
	//---------------------------------------------------------------------------------------------------
	// IMU algorithm update


	float32_t recipNorm;
	float32_t s0, s1, s2, s3;
	float32_t qDot1, qDot2, qDot3, qDot4;
	float32_t _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-m_Orient.q1 * angRate.x - m_Orient.q2 * angRate.y - m_Orient.q3 * angRate.z);
	qDot2 = 0.5f * (m_Orient.q0 * angRate.x + m_Orient.q2 * angRate.z - m_Orient.q3 * angRate.y);
	qDot3 = 0.5f * (m_Orient.q0 * angRate.y - m_Orient.q1 * angRate.z + m_Orient.q3 * angRate.x);
	qDot4 = 0.5f * (m_Orient.q0 * angRate.z + m_Orient.q1 * angRate.y - m_Orient.q2 * angRate.x);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((accel.x == 0.0f) && (accel.y == 0.0f) && (accel.z == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
		accel.x *= recipNorm;
		accel.y *= recipNorm;
		accel.z *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * m_Orient.q0;
		_2q1 = 2.0f * m_Orient.q1;
		_2q2 = 2.0f * m_Orient.q2;
		_2q3 = 2.0f * m_Orient.q3;
		_4q0 = 4.0f * m_Orient.q0;
		_4q1 = 4.0f * m_Orient.q1;
		_4q2 = 4.0f * m_Orient.q2;
		_8q1 = 8.0f * m_Orient.q1;
		_8q2 = 8.0f * m_Orient.q2;
		q0q0 = m_Orient.q0 * m_Orient.q0;
		q1q1 = m_Orient.q1 * m_Orient.q1;
		q2q2 = m_Orient.q2 * m_Orient.q2;
		q3q3 = m_Orient.q3 * m_Orient.q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * accel.x + _4q0 * q1q1 - _2q1 * accel.y;
		s1 = _4q1 * q3q3 - _2q3 * accel.x + 4.0f * q0q0 * m_Orient.q1 - _2q0 * accel.y - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * accel.z;
		s2 = 4.0f * q0q0 * m_Orient.q2 + _2q0 * accel.x + _4q2 * q3q3 - _2q3 * accel.y - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * accel.z;
		s3 = 4.0f * q1q1 * m_Orient.q3 - _2q1 * accel.x + 4.0f * q2q2 * m_Orient.q3 - _2q2 * accel.y;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	m_Orient.q0 += qDot1 * (1.0f / sampleFreq);
	m_Orient.q1 += qDot2 * (1.0f / sampleFreq);
	m_Orient.q2 += qDot3 * (1.0f / sampleFreq);
	m_Orient.q3 += qDot4 * (1.0f / sampleFreq);

	// Normalise quaternion
	recipNorm = invSqrt(m_Orient.q0 * m_Orient.q0 + m_Orient.q1 * m_Orient.q1 + m_Orient.q2 * m_Orient.q2 + m_Orient.q3 * m_Orient.q3);
	m_Orient.q0 *= recipNorm;
	m_Orient.q1 *= recipNorm;
	m_Orient.q2 *= recipNorm;
	m_Orient.q3 *= recipNorm;

	//Rotate gravity vector by our orientation
	Vector3 gravity = {0.0f, 0.0f, 1.0f};
	Quaternion rot;
	Vector3 accelVect;
	rot.q0 = m_Orient.q0;
	rot.q1 = m_Orient.q1;
	rot.q2 = m_Orient.q2;
	rot.q3 = m_Orient.q3;
	accelVect.x = accel.x;
	accelVect.y = accel.y;
	accelVect.z = accel.z;
	Vector3 accel_rot = rotateVector (accelVect, rot);
	Vector3 acc;
	acc.x = accel_rot.x - gravity.x;
	acc.y = accel_rot.y - gravity.y;
	acc.z = accel_rot.z - gravity.z;
	m_Acceleration = acc;


}

void CPositioning::reset (void)
{
	m_Orient.q0 = 1.0f;
	m_Orient.q1 = 0.0f;
	m_Orient.q2 = 0.0f;
	m_Orient.q3 = 0.0f;
}

sOrientation CPositioning::getOrientation (void)
{
	sOrientation ret = {0.0f, 0.0f, 0.0f};

	ret.roll = -atan2f(2.0f*(m_Orient.q2*m_Orient.q3 - m_Orient.q0*m_Orient.q1), -1.0f + 2.0f*(m_Orient.q0*m_Orient.q0 + m_Orient.q3*m_Orient.q3)) / M_PI * 180.0f;
	ret.pitch = - asinf(2.0f*(m_Orient.q1*m_Orient.q3 + m_Orient.q0*m_Orient.q2)) / M_PI * 180.0f;
	ret.yaw =  atan2f (2.0f*(m_Orient.q1*m_Orient.q2 - m_Orient.q0*m_Orient.q3), -1.0f + 2.0f*(m_Orient.q0*m_Orient.q0 + m_Orient.q1 * m_Orient.q1)) / M_PI * 180.0f;


	//printf ("[POSITIONING] Estimated orientation: roll: %f pitch: %f, yaw: %f\n",ret.roll, ret.pitch, ret.yaw);
	//printf ("[POSITIONING] Estimated orientation: w: %f x: %f, y: %f z: %f\n",m_Orient.q0, m_Orient.q1, m_Orient.q2, m_Orient.q3);
	//printf ("[POSITIONING] Estimated acceleration: x: %f y: %f, z: %f / Comp: x: %f y: %f, z: %f\n",m_Acceleration.x, m_Acceleration.y, m_Acceleration.z,m_AccBiasComp.x, m_AccBiasComp.y, m_AccBiasComp.z);
	//printf ("[POSITIONING] Estimated speed: x: %f y: %f, z: %f\n",m_Speed.x, m_Speed.y, m_Speed.z);
	//printf ("[POSITIONING] Estimated pos: x: %f y: %f, z: %f\n",m_Pos.x, m_Pos.y, m_Pos.z);
	return ret;
}

sPosition CPositioning::getPosition (void)
{
	sPosition ret = {0.0f, 0.0f, 0.0f};
	return ret;

}

float32_t CPositioning::invSqrt (float32_t x)
{
    uint32_t i = 0x5F1F1412 - (*(uint32_t*)&x >> 1);
    float32_t tmp = *(float32_t*)&i;
    float32_t y = tmp * (1.69000231f - 0.714158168f * x * tmp * tmp);
	return y;

}

Vector3 CPositioning::rotateVector (Vector3 value, Quaternion rotation)
{

	 Vector3 vector;
	float32_t num12 = rotation.q1 + rotation.q1;
	float32_t num2 = rotation.q2 + rotation.q2;
	float32_t num = rotation.q3 + rotation.q3;
	float32_t num11 = rotation.q0 * num12;
	float32_t num10 = rotation.q0 * num2;
	float32_t num9 = rotation.q0 * num;
	float32_t num8 = rotation.q1 * num12;
	float32_t num7 = rotation.q1 * num2;
	float32_t num6 = rotation.q1 * num;
	float32_t num5 = rotation.q2 * num2;
	float32_t num4 = rotation.q2 * num;
	float32_t num3 = rotation.q3 * num;
	float32_t num15 = ((value.x * ((1.0f - num5) - num3)) + (value.y * (num7 - num9))) + (value.z * (num6 + num10));
	float32_t num14 = ((value.x * (num7 + num9)) + (value.y * ((1.0f - num8) - num3))) + (value.z * (num4 - num11));
	float32_t num13 = ((value.x * (num6 - num10)) + (value.y * (num4 + num11))) + (value.z * ((1.0f - num8) - num5));
	vector.x = num15;
	vector.y = num14;
	vector.z = num13;
	return vector;
}


float32_t CPositioning::filter (float32_t x, float32_t * coeff, float32_t * delay)
{
	float32_t y;
	delay[0] = x - coeff[A1] * delay[1] - coeff[A2]  * delay[2];
	y = coeff[B0] * delay[0] + coeff[B1] * delay[1] + coeff[B2] * delay[2];
	delay[2] = delay[1];
	delay[1] = delay[0];
	return y;

}

void CPositioning::setParameter (YapiBotParam_t param, int8_t * buffer, uint32_t size)
{
	uint32_t val;
	if (size < 4)
	{
		fprintf (stderr, "Cannot set position parameter (not enough arguments)");
	}



	switch (param)
	{
		case PosFltGain:
			beta = Utils::toFloat (&buffer[0]);
			CSettings::getInstance()->setFloat("POSITIONNING", "AHR Filter gain", beta);
			break;
		default:
			fprintf (stderr, "Unknown Positioning parameter !");
			break;
	}
}

void CPositioning::getParameter (YapiBotParam_t param)
{
	YapiBotParamAnswer_t answer;

	Utils::fromInt((int32_t)param, &answer.param);

	switch (param)
	{
		case PosFltGain:
			Utils::fromFloat (beta, &answer.val);
			break;

		default:
			fprintf (stderr, "Unknown Positioning parameter !");
			return;
	}
	CNetwork::getInstance()->sendCmdPck (CmdInfoParam, (uint8_t *)&answer, sizeof(YapiBotParamAnswer_t));
}


