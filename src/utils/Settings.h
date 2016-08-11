/*
 * Settings.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <Singleton.h>
#include <glib.h>

class CSettings: public CSingleton<CSettings>
{
	friend class CSingleton<CSettings> ;
protected:
	CSettings();
	~CSettings();

public:
	bool getBoolean (char * group,char * key, bool defaultValue);
	int getInt (char * group,char * key, int defaultValue);
	float getFloat (char * group,char * key, float defaultValue);

	void setBoolean (char * group,char * key, bool val);
	void setInt (char * group,char * key, int val);
	void setFloat (char * group,char * key, float val);

private:
	GKeyFile * m_KeyFile;

};

#endif /* SETTINGS_H_ */
