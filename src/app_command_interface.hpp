/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

/* This class handles parsing of a simple command syntax for scripting,
   UI components, network commands, etc.
*/

#ifndef _APP_COMMAND_INTERFACE_H_
#define _APP_COMMAND_INTERFACE_H_

#include "utility.hpp"

class Core;
class CoreIO;
class App;
class ScriptMgr;
class Media;

class AppCommandInterface {

public:
	AppCommandInterface(Core * core, CoreIO* _coreIO, App * app, Media* _media);
	~AppCommandInterface();
	AppCommandInterface(AppCommandInterface const &) = delete;
	AppCommandInterface& operator = (AppCommandInterface const &) = delete;

	int executeCommand(const std::string &commandline);
	int executeCommand(const std::string &command, unsigned long int &wait);
	int executeIcommand(const std::string &command, double arg);
	int executeIcommand(const std::string &command, int arg);
	bool setFlag(const std::string &name, const std::string &value, bool &newval);
	std::string getErrorString();

protected:
	//all different command
	int commandAdd();
	int commandAudio();
	int commandBodyTrace();
	int commandBody();
	int commandCamera(unsigned long int &wait);
	int commandClear();
	int commandColor();
	int commandComment();
	int commandConfiguration();
	int commandConstellation();
	int commandDate();
	int commandDefine();
	int commandDeselect();
	int commandDomemasters();
	int commandDso();
	int commandExternalMplayer();
	int commandExternalViewer();
	int commandFlag();
	int commandGet();
	int commandIlluminate();
	int commandImage();
	int commandLandscape();
	int commandLook();
	int commandMedia();
	int commandMeteors();
	int commandMoveto();
	int commandMovetocity();
	int commandMultiplier();
	int commandMultiply();
	int commandPersonal();
	int commandPersoneq();
	int commandPlanetScale();
	int commandPosition();
	int commandPrint();
	int commandRandom();
	int commandScript();
	int commandSearch();
	int commandSelect();
	int commandSet();
	int commandShutdown();
	int commandSkyCulture();
	int commandSkyDraw();
	int commandStarLines();
	int commandStruct();
	int commandSuntrace();
	int commandText();
	int commandTimerate();
	int commandUncomment();
	int commandWait(unsigned long int &wait);
	int commandZoom(unsigned long int &wait);

private:
	Core * stcore = nullptr;
	CoreIO * coreIO = nullptr;
	App * stapp = nullptr;
	Media* media = nullptr;
	std::string commandline;
	std::string command;
	stringHash_t args;

	//liste de toutes les commandes
	enum class LC_COMMAND : char {LC_ADD, LC_AUDIO, LC_BODY_TRACE, LC_BODY, LC_CAMERA, LC_CLEAR, LC_COLOR, LC_CONFIGURATION, LC_CONSTELLATION, LC_DATE, LC_DEFINE, LC_DESELECT,
								  LC_DOMEMASTERS,
	                              LC_DSO, LC_EXERNALC_MPLAYER, LC_EXTERNALC_VIEWER, LC_FLAG, LC_GET, LC_ILLUMINATE, LC_IMAGE, LC_LANDSCAPE, LC_LOOK, LC_MEDIA, LC_METEORS,
	                              LC_MOVETO, LC_MOVETOCITY, LC_MULTIPLIER, LC_MULTIPLY, LC_PERSONAL, LC_PERSONEQ, LC_PLANET_SCALE, LC_POSITION, LC_PRINT, LC_RANDOM,
	                              LC_SCRIPT, LC_SEARCH, LC_SELECT, LC_SET, LC_SHUTDOWN, LC_SKY_CULTURE, LC_SKY_DRAW, LC_STAR_LINES, LC_STRUCT, LC_SUNTRACE, LC_TEXT,
	                              LC_TIMERATE, LC_WAIT, LC_ZOOM, LC_FLYTO
	                             };

	enum class FLAG_VALUES: char { FV_TOGGLE, FV_ON, FV_OFF};

	enum class FLAG_NAMES: char {FN_ANTIALIAS_LINES, FN_CONSTELLATION_DRAWING, FN_CONSTELLATION_NAMES, FN_CONSTELLATION_ART, FN_CONSTELLATION_BOUNDARIES, FN_CONSTELLATION_PICK,
	                             FN_STAR_TWINKLE, FN_NAVIGATION, FN_SHOW_TUI_DATETIME, FN_SHOW_TUI_SHORT_OBJ_INFO, FN_MANUAL_ZOOM, FN_LIGHT_TRAVEL_TIME, FN_DSO_PICTOGRAMS,
	                             FN_FOG, FN_ATMOSPHERE, FN_AZIMUTHAL_GRID, FN_EQUATORIAL_GRID, FN_ECLIPTIC_GRID, FN_GALACTIC_GRID, FN_EQUATOR_LINE, FN_GALACTIC_LINE,
	                             FN_ECLIPTIC_LINE, FN_PRECESSION_CIRCLE, FN_CIRCUMPOLAR_CIRCLE, FN_TROPIC_LINES, FN_MERIDIAN_LINE, FN_ZENITH_LINE, FN_POLAR_CIRCLE,
	                             FN_ECLIPTIC_CENTER, FN_GALACTIC_POLE, FN_GALACTIC_CENTER, FN_VERNAL_POINTS, FN_ANALEMMA_LINE, FN_ANALEMMA, FN_ARIES_LINE,
	                             FN_ZODIAC, FN_PERSONAL, FN_PERSONEQ, FN_GREENWICH_LINE, FN_VERTICAL_LINE, FN_CARDINAL_POINTS, FN_CLOUDS, FN_MOON_SCALED, FN_SUN_SCALED,
	                             FN_LANDSCAPE, FN_STARS, FN_STAR_NAMES, FN_PLANETS, FN_PLANET_NAMES, FN_PLANET_ORBITS, FN_ORBITS, FN_PLANETS_ORBITS, FN_PLANETS_AXIS,
	                             FN_SATELLITES_ORBITS, FN_NEBULAE, FN_NEBULA_NAMES, FN_NEBULA_HINTS, FN_MILKY_WAY, FN_BRIGHT_NEBULAE, FN_OBJECT_TRAILS, FN_TRACK_OBJECT,
	                             FN_SCRIPT_GUI_DEBUG, FN_LOCK_SKY_POSITION, FN_BODY_TRACE, FN_SHOW_LATLON, FN_COLOR_INVERSE, FN_OORT, FN_STARS_TRACE, FN_STAR_LINES,
	                             FN_SKY_DRAW, FN_ZODIAC_LIGHT , FN_TULLY, FN_SATELLITES
	                            };

	FLAG_VALUES convertStrToFlagValues(const std::string &value);

	int recordable;
	bool swapCommand;     // boolean qui indique si l'instruction doit etre exécutée ou pas
	bool swapIfCommand;     // boolean qui indique si dans un if l'instruction doit etre exécutée ou pas

	//! vérifie que la chaine a représente bien un boolean
	bool isBoolean(const std::string &a); //cyp
	//! vérifie que la chaine a représente FAUX
	bool isFalse(const std::string &a);
	//! vérifie que la chaine a représente VRAI
	bool isTrue(const std::string &a);

	std::string debug_message;  //!< for 'executeCommand' error details
	int parseCommand(const std::string &command_line, std::string &command, stringHash_t &arguments);
	int executeCommandStatus();

	double evalDouble(const std::string &var);
	int evalInt (const std::string &var);//cyp
	std::string evalString (const std::string &var);//cyp

	void initialiseCommandsName();
	void initialiseFlagsName();


	void deleteVar();
	void printVar();
	double max_random;
	double min_random;

	//variables utilisées dans le moteur de scripts
	std::map<const std::string, std::string> variables;
	std::map<const std::string, std::string>::iterator var_it;
	//map assurant la transcription entre le texte et la commande associée
	std::map<const std::string, LC_COMMAND> m_commands;
	std::map<const std::string, LC_COMMAND>::iterator m_commands_it;
	//map assurant la transcription entre le texte et le flag associé
	std::map<const std::string, FLAG_NAMES> m_flags;
	std::map<const std::string, FLAG_NAMES>::iterator m_flag_it;
};


#endif // _APP_COMMAND_INTERFACE_H
