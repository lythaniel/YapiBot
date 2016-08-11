/*
 * RangeFinder_2Y0A21.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef RANGEFINDER_2Y0A21_H_
#define RANGEFINDER_2Y0A21_H_

#include "RangeFinder.h"


class CRangeFinder_2Y0A21: public CRangeFinder {

	friend class CSensorFactory;

protected:
	CRangeFinder_2Y0A21();
	virtual ~CRangeFinder_2Y0A21();

public:
	virtual void setBus (CI2Cbus * bus);
	virtual int32_t getRange (void);

private:


};

#endif /* RANGEFINDER_2Y0A21_H_ */
