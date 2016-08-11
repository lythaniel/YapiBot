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

#include "YapiBotTypes.h"
#include <Singleton.h>
#include <glib.h>

class CSettings: public CSingleton<CSettings>
{
	friend class CSingleton<CSettings> ;
protected:
	CSettings();
	~CSettings();

public:
	bool getBoolean (const char * group, const char * key, bool defaultValue);
	int32_t getInt (const char * group, const char * key, int32_t defaultValue);
	float32_t getFloat (const char * group, const char * key, float32_t defaultValue);

	void setBoolean (const char * group, const char * key, bool val);
	void setInt (const char * group, const char * key, int32_t val);
	void setFloat (const char * group, const char * key, float32_t val);

private:
	GKeyFile * m_KeyFile;

};

#endif /* SETTINGS_H_ */
