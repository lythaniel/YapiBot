/*
 * Settings.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <Settings.h>



CSettings::CSettings() {
	m_KeyFile = g_key_file_new ();
	GKeyFileFlags flags = (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);
	GError *error = NULL;

	/* Load the GKeyFile from YapiBot.ini  */
	if (!g_key_file_load_from_file (m_KeyFile, "YapiBot.ini", flags, &error))
	{
		//g_error (error->message);
	}


}

CSettings::~CSettings() {
	GError *error = NULL;
	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		//g_error (error->message);
	}
}

bool CSettings::getBoolean (const char * group, const char * key, bool defaultValue)
{
	GError *error = NULL;
	bool ret;
	ret = g_key_file_get_boolean (m_KeyFile, group, key, &error);
	if (error != NULL)
	{
		ret = defaultValue;
		setBoolean (group,key,ret);
	}
	return ret;
}

int32_t CSettings::getInt (const char * group, const char * key, int32_t defaultValue)
{
	GError *error = NULL;
	int32_t ret;
	ret = g_key_file_get_integer (m_KeyFile, group, key, &error);
	if (error != NULL)
	{
		ret = defaultValue;
		setInt (group,key,ret);
	}
	return ret;

}

float32_t CSettings::getFloat (const char * group, const char * key, float32_t defaultValue)
{
	GError *error = NULL;
	double ret;
	ret = g_key_file_get_double (m_KeyFile, group, key, &error);
	if (error != NULL)
	{
		ret = defaultValue;
		setFloat (group,key,ret);
	}
	return ret;

}

void CSettings::setBoolean (const char * group, const char * key, bool val)
{
	GError *error = NULL;
	g_key_file_set_boolean (m_KeyFile, group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}

}
void CSettings::setInt (const char * group, const char * key, int32_t val)
{
	GError *error = NULL;
	g_key_file_set_integer (m_KeyFile, group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}
}
void CSettings::setFloat (const char * group, const char * key, float32_t val)
{
	GError *error = NULL;
	g_key_file_set_double(m_KeyFile,group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}

}


