/*
 * Positioning.h
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

#ifndef POSITIONING_H_
#define POSITIONING_H_

#include <Singleton.h>
#include <Compass.h>
#include <Accelerometer.h>
#include <Gyroscope.h>
#include "YapiBotCmd.h"

typedef struct {
	float32_t roll;
	float32_t pitch;
	float32_t yaw;
} sOrientation;

typedef struct {
	float32_t x;
	float32_t y;
	float32_t z;
} Vector3;

typedef struct {
	float32_t q0;
	float32_t q1;
	float32_t q2;
	float32_t q3;
} Quaternion;

typedef Vector3 sPosition;


class CPositioning: public CSingleton<CPositioning>
{
	friend class CSingleton<CPositioning> ;
protected:
	CPositioning();
	~CPositioning();

public:

	void update (sAngularRate angRate, sAccel accel, sMagField field);
	void update (sAngularRate angRate, sAccel accel);
	void reset (void);
	sOrientation getOrientation (void);
	sPosition getPosition (void);
	void setFilterGain (float32_t fltGain) {beta = fltGain;}
	float32_t getFilterGain (void) {return beta;}

	void setParameter (YapiBotParam_t, int8_t * buffer, uint32_t size);
	void getParameter (YapiBotParam_t param);


private:

	float32_t invSqrt (float32_t x);
	Vector3 rotateVector (Vector3 value, Quaternion rotation);
	float32_t filter (float32_t x, float32_t * coeff, float32_t * delay);

	Quaternion m_Orient;

	Vector3 m_Acceleration;
	Vector3 m_Speed;
	Vector3 m_AccBiasComp;
	sPosition m_Pos;

	float32_t sampleFreq;
	float32_t beta;
	float32_t AccBiasFact;

	float32_t m_FltDelay[3][3];
	float32_t m_FltCoeff[5];
};

#endif /* POSITIONING_H_ */
