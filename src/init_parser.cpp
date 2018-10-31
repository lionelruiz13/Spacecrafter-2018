/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which parse an init file
// C++ warper for the iniparser free library from N.Devillard

#include <sstream>
#include <iomanip>

#include "init_parser.hpp"

#include "log.hpp"

using namespace std;

#define DEBUG 0

InitParser::InitParser(void) : dico(NULL)
{
	dico = dictionary_new(0);
}

InitParser::~InitParser()
{
	if (dico) freeDico();
	dico = NULL;
}

void InitParser::load(const string& file)
{
	// Check file presence
	FILE * fp = NULL;
	fp = fopen(file.c_str(),"rt");
	if (fp) {
		fclose(fp);
	} else {
		Log.write("Init_parser : can't find config file "+ file, cLog::LOG_TYPE::L_ERROR);
		exit(-1);
	}

	if (dico) freeDico();
	dico = NULL;
	dico = iniparser_load(file.c_str());
	if (!dico) {
		Log.write("Init_parser : can't read config file "+ file, cLog::LOG_TYPE::L_ERROR);
		exit(-1);
	}
}

void InitParser::save(const string& file_name) const
{
	// Check file presence
	FILE * fp = NULL;
	fp = fopen(file_name.c_str(),"wt");
	if (fp) {
		iniparser_dump_ini(dico, fp);
	} else {
		Log.write("Init_parser : can't open config file "+ file_name, cLog::LOG_TYPE::L_ERROR);
	}
	if (fp) fclose(fp);
}

string InitParser::getStr(const string& key) const
{
	if (DEBUG)
		printf("string key %s\n", key.c_str() );

	if (iniparser_getstring(dico, key.c_str(), NULL)) return string(iniparser_getstring(dico, key.c_str(), NULL));
	else
		Log.write("Init_parser can't find the configuration key \"" + key + "\", default empty string returned", cLog::LOG_TYPE::L_WARNING);
	return string();
}

string InitParser::getStr(const string& section, const string& key) const
{
	return getStr((section+":"+key).c_str());
}

string InitParser::getStr(const string& section, const string& key, const string& def) const
{
	Log.write("Init_parser def_str " + section + ":" + key, cLog::LOG_TYPE::L_WARNING);
	return iniparser_getstring(dico, (section+":"+key).c_str(), def.c_str());
}

int InitParser::getInt(const string& key) const
{
	if (DEBUG)
		printf("int key %s\n", key.c_str() );

	int i = iniparser_getint(dico, key.c_str(), -11111);
	// To be sure :) (bugfree)
	if (i==-11111) {
		i = iniparser_getint(dico, key.c_str(), 0);
		if (i==0) {
			Log.write("Init_parser : can't find the configuration key \"" + key + "\", default 0 value returned", cLog::LOG_TYPE::L_WARNING);
		}
	}
	return i;
}

int InitParser::getInt(const string& section, const string& key) const
{
	return getInt((section+":"+key).c_str());
}

int InitParser::getInt(const string& section, const string& key, int def) const
{
	Log.write("Init_parser def_int " + section + ":" + key, cLog::LOG_TYPE::L_WARNING);
	return iniparser_getint(dico, (section+":"+key).c_str(), def);
}

double InitParser::getDouble(const string& key) const
{
	if (DEBUG)
		printf("double key %s\n", key.c_str() );

	double d = iniparser_getdouble(dico, key.c_str(), -11111.111111356);
	// To be sure :) (bugfree)
	if (d==-11111.111111356) {
		d = iniparser_getdouble(dico, key.c_str(), 0.);
		if (d==0.) {
			Log.write("Init_parser : can't find the configuration key \"" + key + "\", default 0 value returned", cLog::LOG_TYPE::L_WARNING);
		}
	}
	return d;
}

double InitParser::getDouble(const string& section, const string& key) const
{
	return getDouble((section+":"+key).c_str());
}

double InitParser::getDouble(const string& section, const string& key, double def) const
{
	Log.write("Init_parser def_double " + section + ":" + key, cLog::LOG_TYPE::L_WARNING);
	return iniparser_getdouble(dico, (section+":"+key).c_str(), def);
}

bool InitParser::getBoolean(const string& key) const
{
	if (DEBUG)
		printf("boolean key %s\n", key.c_str() );

	int b = iniparser_getboolean(dico, key.c_str(), -10);
	// To be sure :) (bugfree)
	if (b==-10) {
		b = iniparser_getboolean(dico, key.c_str(), 0);
		if (b==0) {
			Log.write("Init_parser : can't find the configuration key \"" + key + "\", default 0 value returned", cLog::LOG_TYPE::L_WARNING);
		}
	}
	return b;
}

bool InitParser::getBoolean(const string& section, const string& key) const
{
	return getBoolean((section+":"+key).c_str());
}

bool InitParser::getBoolean(const string& section, const string& key, bool def) const
{
	Log.write("Init_parser def_bool " + section + ":" + key, cLog::LOG_TYPE::L_WARNING);
	return iniparser_getboolean(dico, (section+":"+key).c_str(), def);
}

// Set the given entry with the provided value. If the entry cannot be found
// -1 is returned and the entry is created. Else 0 is returned.
int InitParser::setStr(const string& key, const string& val)
{
	makeSectionFromKey(key);
	int return_val;
	if (findEntry(key)) return_val = 0;
	else return_val = -1;

	dictionary_set(dico, key.c_str(), val.c_str());
	return return_val;
}

int InitParser::setInt(const string& key, int val)
{
	makeSectionFromKey(key);
	int return_val;
	if (findEntry(key)) return_val = 0;
	else return_val = -1;

	stringstream ss;
	ss << val;
	dictionary_set(dico, key.c_str(), ss.str().c_str());

	return return_val;
}

int InitParser::setDouble(const string& key, double val)
{
	makeSectionFromKey(key);
	int return_val;
	if (findEntry(key)) return_val = 0;
	else return_val = -1;

	ostringstream os;
	os << setprecision(16) << val;
	dictionary_set(dico, key.c_str(), os.str().c_str());

	return return_val;
}

int InitParser::setBoolean(const string& key, bool val)
{
	if (val) return setStr(key, "true");
	else return setStr(key, "false");
}

// Check if the key is in the form section:key and if yes create the section in the dictionnary
// if it doesn't exist.
void InitParser::makeSectionFromKey(const string& key)
{
	int pos = key.find(':');
	if (!pos) return;				// No ':' were found
	string sec = key.substr(0,pos);
	if (findEntry(sec)) return;	// The section is already present into the dictionnary
	dictionary_set(dico, sec.c_str(), NULL);	// Add the section key
}

// Get number of sections
int InitParser::getNsec(void) const
{
	return iniparser_getnsec(dico);
}

// Get name for section n.
string InitParser::getSecname(int n) const
{
	if (iniparser_getsecname(dico, n)) return string(iniparser_getsecname(dico, n));
	return string();
}

// Return 1 if the entry exists, 0 otherwise
int InitParser::findEntry(const string& entry) const
{
	return iniparser_find_entry(dico, entry.c_str());
}

void InitParser::freeDico(void)
{
	iniparser_freedict(dico);
}
