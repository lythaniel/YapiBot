/*
 * RangeFinder.h
 *
 *  Created on: 1 janv. 2015
 *      Author: lythaniel
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
	virtual int getRange (void);

private:


};

#endif /* RANGEFINDER_H_ */
