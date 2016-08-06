/*
 * Settings.cpp
 *
 *  Created on: 5 août 2016
 *      Author: lythaniel
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

bool CSettings::getBoolean (char * group,char * key, bool defaultValue)
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

int CSettings::getInt (char * group,char * key, int defaultValue)
{
	GError *error = NULL;
	int ret;
	ret = g_key_file_get_integer (m_KeyFile, group, key, &error);
	if (error != NULL)
	{
		ret = defaultValue;
		setInt (group,key,ret);
	}
	return ret;

}

float CSettings::getFloat (char * group,char * key, float defaultValue)
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

void CSettings::setBoolean (char * group,char * key, bool val)
{
	GError *error = NULL;
	g_key_file_set_boolean (m_KeyFile, group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}

}
void CSettings::setInt (char * group,char * key, int val)
{
	GError *error = NULL;
	g_key_file_set_integer (m_KeyFile, group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}
}
void CSettings::setFloat (char * group,char * key, float val)
{
	GError *error = NULL;
	g_key_file_set_double(m_KeyFile, group, key, val);

	if (!g_key_file_save_to_file(m_KeyFile,"YapiBot.ini", &error))
	{
		g_error (error->message);
	}

}


