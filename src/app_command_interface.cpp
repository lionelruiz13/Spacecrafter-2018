/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005-2006 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2017 of the LSS Team & Association Sirius
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

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>


#include "app_command_interface.hpp"
#include "app_command_color.hpp"
#include "app.hpp"
#include "core_io.hpp"
// #include "spacecrafter.hpp"
#include "log.hpp"
#include "app_settings.hpp"
#include "utility.hpp"
#include "script_mgr.hpp"
#include "file_path.hpp"
#include "screenFader.hpp"
#include "utility.hpp"
#include "call_system.hpp"
#include "media.hpp"
#include "ui.hpp"

using namespace std;


AppCommandInterface::AppCommandInterface(Core * core, CoreIO* _coreIO, App * app, Media* _media)
{
	stcore = core;
	coreIO = _coreIO;
	stapp = app;
	media = _media;
	swapCommand = false;
	swapIfCommand = false;
	max_random = 1.0;
	min_random = 0.0;
	initialiseCommandsName();
	initialiseFlagsName();
}


void AppCommandInterface::initialiseFlagsName()
{
	m_flags["antialias_lines"]= FLAG_NAMES::FN_ANTIALIAS_LINES;
	m_flags["constellation_drawing"]= FLAG_NAMES::FN_CONSTELLATION_DRAWING;
	m_flags["constellation_names"]= FLAG_NAMES::FN_CONSTELLATION_NAMES;
	m_flags["constellation_art"]= FLAG_NAMES::FN_CONSTELLATION_ART;
	m_flags["constellation_boundaries"]= FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES;
	m_flags["constellation_pick"]= FLAG_NAMES::FN_CONSTELLATION_PICK;

	m_flags["star_twinkle"]= FLAG_NAMES::FN_STAR_TWINKLE;
	m_flags["navigation"]= FLAG_NAMES::FN_NAVIGATION;
	m_flags["show_tui_datetime"]= FLAG_NAMES::FN_SHOW_TUI_DATETIME;
	m_flags["show_tui_short_obj_info"]= FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO;
	m_flags["manual_zoom"]= FLAG_NAMES::FN_MANUAL_ZOOM;

	m_flags["light_travel_time"]= FLAG_NAMES::FN_LIGHT_TRAVEL_TIME;
	m_flags["fog"]= FLAG_NAMES::FN_FOG;
	m_flags["atmosphere"]= FLAG_NAMES::FN_ATMOSPHERE;
	m_flags["azimuthal_grid"]= FLAG_NAMES::FN_AZIMUTHAL_GRID;
	m_flags["equatorial_grid"]= FLAG_NAMES::FN_EQUATORIAL_GRID;

	m_flags["ecliptic_grid"]= FLAG_NAMES::FN_ECLIPTIC_GRID;
	m_flags["galactic_grid"]= FLAG_NAMES::FN_GALACTIC_GRID;
	m_flags["equator_line"]= FLAG_NAMES::FN_EQUATOR_LINE;
	m_flags["galactic_line"]= FLAG_NAMES::FN_GALACTIC_LINE;
	m_flags["ecliptic_line"]= FLAG_NAMES::FN_ECLIPTIC_LINE;

	m_flags["precession_circle"]= FLAG_NAMES::FN_PRECESSION_CIRCLE;
	m_flags["circumpolar_circle"]= FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE;
	m_flags["tropic_lines"]= FLAG_NAMES::FN_TROPIC_LINES;
	m_flags["meridian_line"]= FLAG_NAMES::FN_MERIDIAN_LINE;
	m_flags["zenith_line"]= FLAG_NAMES::FN_ZENITH_LINE;

	m_flags["polar_circle"]= FLAG_NAMES::FN_POLAR_CIRCLE;
	m_flags["ecliptic_center"]= FLAG_NAMES::FN_ECLIPTIC_CENTER;
	m_flags["galactic_pole"]= FLAG_NAMES::FN_GALACTIC_POLE;
	m_flags["galactic_center"]= FLAG_NAMES::FN_GALACTIC_CENTER;
	m_flags["vernal_points"]= FLAG_NAMES::FN_VERNAL_POINTS;

	m_flags["analemma_line"]= FLAG_NAMES::FN_ANALEMMA_LINE;
	m_flags["analemma"]= FLAG_NAMES::FN_ANALEMMA;
	m_flags["aries_line"]= FLAG_NAMES::FN_ARIES_LINE;
	m_flags["zodiac"]= FLAG_NAMES::FN_ZODIAC;
	m_flags["personal"]= FLAG_NAMES::FN_PERSONAL;

	m_flags["personeq"]= FLAG_NAMES::FN_PERSONEQ;
	m_flags["greenwich_line"]= FLAG_NAMES::FN_GREENWICH_LINE;
	m_flags["vertical_line"]= FLAG_NAMES::FN_VERTICAL_LINE;
	m_flags["cardinal_points"]= FLAG_NAMES::FN_CARDINAL_POINTS;
	m_flags["clouds"]= FLAG_NAMES::FN_CLOUDS;

	m_flags["moon_scaled"]= FLAG_NAMES::FN_MOON_SCALED;
	m_flags["sun_scaled"]= FLAG_NAMES::FN_SUN_SCALED;
	m_flags["landscape"]= FLAG_NAMES::FN_LANDSCAPE;
	m_flags["stars"]= FLAG_NAMES::FN_STARS;
	m_flags["star_names"]= FLAG_NAMES::FN_STAR_NAMES;

	m_flags["planets"]= FLAG_NAMES::FN_PLANETS;
	m_flags["planet_names"]= FLAG_NAMES::FN_PLANET_NAMES;
	m_flags["planet_orbits"]= FLAG_NAMES::FN_PLANET_ORBITS;
	m_flags["orbits"]= FLAG_NAMES::FN_ORBITS;
	m_flags["planets_orbits"]= FLAG_NAMES::FN_PLANETS_ORBITS;

	m_flags["planets_axis"]= FLAG_NAMES::FN_PLANETS_AXIS;
	m_flags["satellites_orbits"]= FLAG_NAMES::FN_SATELLITES_ORBITS;
	m_flags["nebulae"]= FLAG_NAMES::FN_NEBULAE;
	m_flags["nebula_names"]= FLAG_NAMES::FN_NEBULA_NAMES;
	m_flags["nebula_hints"]= FLAG_NAMES::FN_NEBULA_HINTS;

	m_flags["milky_way"]= FLAG_NAMES::FN_MILKY_WAY;
	m_flags["bright_nebulae"]= FLAG_NAMES::FN_BRIGHT_NEBULAE;
	m_flags["object_trails"]= FLAG_NAMES::FN_OBJECT_TRAILS;
	m_flags["track_object"]= FLAG_NAMES::FN_TRACK_OBJECT;
	m_flags["script_gui_debug"]= FLAG_NAMES::FN_SCRIPT_GUI_DEBUG;

	m_flags["lock_sky_position"]= FLAG_NAMES::FN_LOCK_SKY_POSITION;
	m_flags["body_trace"]= FLAG_NAMES::FN_BODY_TRACE;
	m_flags["show_latlon"]= FLAG_NAMES::FN_SHOW_LATLON;
	m_flags["color_inverse"]= FLAG_NAMES::FN_COLOR_INVERSE;
	m_flags["oort"]= FLAG_NAMES::FN_OORT;

	m_flags["stars_trace"]= FLAG_NAMES::FN_STARS_TRACE;
	m_flags["star_lines"]= FLAG_NAMES::FN_STAR_LINES;
	m_flags["sky_draw"]= FLAG_NAMES::FN_SKY_DRAW;
	m_flags["dso_pictograms"]= FLAG_NAMES::FN_DSO_PICTOGRAMS;
	m_flags["zodiacal_light"]= FLAG_NAMES::FN_ZODIAC_LIGHT;

	m_flags["tully"]= FLAG_NAMES::FN_TULLY;
	m_flags["satellites"] = FLAG_NAMES::FN_SATELLITES;
}


void AppCommandInterface::initialiseCommandsName()
{
	m_commands["add"] = LC_COMMAND::LC_ADD;
	m_commands["audio"] = LC_COMMAND::LC_AUDIO;
	m_commands["body_trace"] = LC_COMMAND::LC_BODY_TRACE;
	m_commands["audio"] = LC_COMMAND::LC_AUDIO;
	m_commands["body"] = LC_COMMAND::LC_BODY;

	m_commands["camera"] = LC_COMMAND::LC_CAMERA;
	m_commands["flyto"] = LC_COMMAND::LC_CAMERA; //alias de camera

	m_commands["clear"] = LC_COMMAND::LC_CLEAR;
	m_commands["color"] = LC_COMMAND::LC_COLOR;
	m_commands["configuration"] = LC_COMMAND::LC_CONFIGURATION;
	m_commands["constellation"] = LC_COMMAND::LC_CONSTELLATION;
	m_commands["date"] = LC_COMMAND::LC_DATE;
	m_commands["define"] = LC_COMMAND::LC_DEFINE;

	m_commands["deselect"] = LC_COMMAND::LC_DESELECT;
	m_commands["domemasters"] = LC_COMMAND::LC_DOMEMASTERS;
	m_commands["dso"] = LC_COMMAND::LC_DSO;
	m_commands["external_mplayer"] = LC_COMMAND::LC_EXERNALC_MPLAYER;
	m_commands["external_viewer"] = LC_COMMAND::LC_EXTERNALC_VIEWER;

	m_commands["flag"] = LC_COMMAND::LC_FLAG;
	m_commands["get"] = LC_COMMAND::LC_GET;
	m_commands["illuminate"] = LC_COMMAND::LC_ILLUMINATE;
	m_commands["image"] = LC_COMMAND::LC_IMAGE;
	m_commands["landscape"] = LC_COMMAND::LC_LANDSCAPE;

	m_commands["look_at"] = LC_COMMAND::LC_LOOK;
	m_commands["media"] = LC_COMMAND::LC_MEDIA;
	m_commands["meteors"] = LC_COMMAND::LC_METEORS;
	m_commands["moveto"] = LC_COMMAND::LC_MOVETO;
	m_commands["movetocity"] = LC_COMMAND::LC_MOVETOCITY;

	m_commands["multiplier"] = LC_COMMAND::LC_MULTIPLIER;
	m_commands["multiply"] = LC_COMMAND::LC_MULTIPLY;
	m_commands["personal"] = LC_COMMAND::LC_PERSONAL;
	m_commands["personeq"] = LC_COMMAND::LC_PERSONEQ;
	m_commands["planet_scale"] = LC_COMMAND::LC_PLANET_SCALE;

	m_commands["position"] = LC_COMMAND::LC_POSITION;
	m_commands["print"] = LC_COMMAND::LC_PRINT;
	m_commands["random"] = LC_COMMAND::LC_RANDOM;
	m_commands["script"] = LC_COMMAND::LC_SCRIPT;
	m_commands["search"] = LC_COMMAND::LC_SEARCH;

	m_commands["select"] = LC_COMMAND::LC_SELECT;
	m_commands["set"] = LC_COMMAND::LC_SET;
	m_commands["shutdown"] = LC_COMMAND::LC_SHUTDOWN;
	m_commands["sky_culture"] = LC_COMMAND::LC_SKY_CULTURE;
	m_commands["sky_draw"] = LC_COMMAND::LC_SKY_DRAW;

	m_commands["star_lines"] = LC_COMMAND::LC_STAR_LINES;
	m_commands["struct"] = LC_COMMAND::LC_STRUCT;
	m_commands["suntrace"] = LC_COMMAND::LC_SUNTRACE;
	m_commands["text"] = LC_COMMAND::LC_TEXT;
	m_commands["timerate"] = LC_COMMAND::LC_TIMERATE;

	m_commands["wait"] = LC_COMMAND::LC_WAIT;
	m_commands["zoom"] = LC_COMMAND::LC_ZOOM;
}

AppCommandInterface::~AppCommandInterface()
{
}

bool AppCommandInterface::isBoolean(const string &a)
{
	if ( isTrue(a) || isFalse(a) )
		return true;
	else
		return false;
}

bool AppCommandInterface::isTrue(const string &a)
{
	if (a=="true" || a =="1" || a=="on" )
		return true;
	else
		return false;
}

bool AppCommandInterface::isFalse(const string &a)
{
	if (a=="false" || a =="0" || a=="off" )
		return true;
	else
		return false;
}

int AppCommandInterface::parseCommand(const std::string &command_line, string &command, stringHash_t &arguments)
{
	istringstream commandstr( command_line );
	string key, value, temp;
	char nextc;

	commandstr >> command;
	transform(command.begin(), command.end(), command.begin(), ::tolower);

	while (commandstr >> key >> value ) {
		if (value[0] == '"') {
			// pull in all text inside quotes
			if (value[value.length()-1] == '"') {
				// one word in quotes
				value = value.substr(1, value.length() -2 );
			} else {
				// multiple words in quotes
				value = value.substr(1, value.length() -1 );

				while (1) {
					nextc = commandstr.get();
					if ( nextc == '"' || !commandstr.good()) break;
					value.push_back( nextc );
				}
			}
		}
		transform(key.begin(), key.end(), key.begin(), ::tolower);
		arguments[key] = value;
	}

	#ifdef PARSE_DEBUG
	Log.write("Command: " + command + "Argument hash:", cLog::LOG_TYPE::L_DEBUG);
	for ( stringHashIter_t iter = arguments.begin(); iter != arguments.end(); ++iter ) {
		Log.write("\t" + iter->first + " : " + iter->second,, cLog::LOG_TYPE::L_DEBUG);
	}
	#endif
	return 1;  // no error checking yet
}


int AppCommandInterface::executeCommand(const string &commandline )
{
	unsigned long int delay;
	return executeCommand(commandline, delay);
	// delay is ignored, as not needed by the ui callers
}

//! for easy calling of simple commands with a double as last argument value
int AppCommandInterface::executeIcommand(const string &command, double arg)
{
	unsigned long int delay;

	std::ostringstream commandline;
	commandline << command << arg;

	return executeCommand(commandline.str(), delay);
}

//! for easy calling of simple commands with an int as last argument value
int AppCommandInterface::executeIcommand(const string &command, int arg)
{
	unsigned long int delay;

	std::ostringstream commandline;
	commandline << command << arg;

	return executeCommand(commandline.str(), delay);
}


//! @brief called by script executors and transform a string to instruction
int AppCommandInterface::executeCommand(const string &_commandline, unsigned long int &wait)
{
	recordable = 1;  // true if command should be recorded (if recording)
	debug_message.clear(); // initialise to empty
	wait = 0;  // default, no wait between commands
	commandline = _commandline;

	command.clear(); // = ""; //vide l'ancienne valeur de args
	args.clear(); //vide les anciennes valeurs de args //TODO A VERIFIER

	// on réécrit toute la ligne proprement sans majuscule minuscule
	parseCommand(commandline, command, args);

	FilePath::fixScriptPath(stapp->scriptMgr->getScriptPath());

	// If command is empty then don't bother checking all these cases
	if( command.length() < 1  && command == "")
		return 0;

	Log.write("Execute_command " + commandline, cLog::LOG_TYPE::L_INFO, cLog::cLog::LOG_FILE::SCRIPT);

	//                                                 //
	// application specific logic to run each command  //
	//                                                 //
	if (command =="comment")
		return commandComment();

	if (command =="uncomment")
		return commandUncomment();

	if (command =="struct")
		return commandStruct();

	if (swapCommand== true || swapIfCommand==true) {	 // on n'execute pas les commandes qui suivent
		Log.write("cette commande n'a pas été exécutée " + commandline, cLog::LOG_TYPE::L_INFO, cLog::cLog::LOG_FILE::SCRIPT);  //A traduire
		return 1;
	}

	m_commands_it = m_commands.find(command);
	if (m_commands_it == m_commands.end()) {
		//~ cout <<"error command "<< command << endl;
		debug_message = _("Unrecognized or malformed command name");
		Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::cLog::LOG_FILE::SCRIPT );
		return 0;
	}

	switch(m_commands_it->second) {
		case LC_COMMAND::LC_ADD : 	return commandAdd(); break;
		case LC_COMMAND::LC_AUDIO : 	return commandAudio(); break;
		case LC_COMMAND::LC_BODY_TRACE :	return commandBodyTrace();  break;
		case LC_COMMAND::LC_BODY :	return commandBody(); break;
		case LC_COMMAND::LC_CAMERA :	return commandCamera(wait); break;
		case LC_COMMAND::LC_CLEAR :	return commandClear(); break;
		case LC_COMMAND::LC_COLOR :	return commandColor(); break;
		case LC_COMMAND::LC_CONFIGURATION :	return commandConfiguration(); break;
		case LC_COMMAND::LC_CONSTELLATION :	return commandConstellation(); break;
		case LC_COMMAND::LC_DATE :	return commandDate(); break;
		case LC_COMMAND::LC_DEFINE :	return commandDefine(); break;
		case LC_COMMAND::LC_DESELECT :	return commandDeselect(); break;
		case LC_COMMAND::LC_DOMEMASTERS :	return commandDomemasters(); break;
		case LC_COMMAND::LC_DSO :	return commandDso(); break;
		case LC_COMMAND::LC_EXERNALC_MPLAYER :	return commandExternalMplayer(); break;
		case LC_COMMAND::LC_EXTERNALC_VIEWER :	return commandExternalViewer(); break;
		case LC_COMMAND::LC_FLAG :	return commandFlag(); break;
		case LC_COMMAND::LC_GET :	return commandGet(); break;
		case LC_COMMAND::LC_ILLUMINATE :	return commandIlluminate(); break;
		case LC_COMMAND::LC_IMAGE :	return commandImage();  break;
		case LC_COMMAND::LC_LANDSCAPE :	return commandLandscape(); break;
		case LC_COMMAND::LC_LOOK :	return commandLook(); break;
		case LC_COMMAND::LC_MEDIA :	return commandMedia(); break;
		case LC_COMMAND::LC_METEORS :	return commandMeteors(); break;
		case LC_COMMAND::LC_MOVETO :	return commandMoveto(); break;
		case LC_COMMAND::LC_MOVETOCITY :	return commandMovetocity(); break;
		case LC_COMMAND::LC_MULTIPLIER :	return commandMultiplier(); break;
		case LC_COMMAND::LC_MULTIPLY :	return commandMultiply(); break;
		case LC_COMMAND::LC_PERSONAL :	return commandPersonal(); break;
		case LC_COMMAND::LC_PERSONEQ :	return commandPersoneq(); break;
		case LC_COMMAND::LC_PLANET_SCALE :	return commandPlanetScale(); break;
		case LC_COMMAND::LC_POSITION :	return commandPosition(); break;
		case LC_COMMAND::LC_PRINT :	return commandPrint(); break;
		case LC_COMMAND::LC_RANDOM :	return commandRandom(); break;
		case LC_COMMAND::LC_SCRIPT :	return commandScript(); break;
		case LC_COMMAND::LC_SEARCH :	return commandSearch(); break;
		case LC_COMMAND::LC_SELECT :	return commandSelect(); break;
		case LC_COMMAND::LC_SET :	return commandSet(); break;
		case LC_COMMAND::LC_SHUTDOWN :	return commandShutdown(); break;
		case LC_COMMAND::LC_SKY_CULTURE :	return commandSkyCulture(); break;
		case LC_COMMAND::LC_SKY_DRAW :	return commandSkyDraw(); break;
		case LC_COMMAND::LC_STAR_LINES :	return commandStarLines(); break;
		//~ case LC_COMMAND::LC_STRUCT :	return commandStruct(); break;
		case LC_COMMAND::LC_SUNTRACE :	return commandSuntrace(); break;
		case LC_COMMAND::LC_TEXT :	return commandText(); break;
		case LC_COMMAND::LC_TIMERATE :	return commandTimerate(); break;
		case LC_COMMAND::LC_WAIT :	return commandWait(wait); break;
		case LC_COMMAND::LC_ZOOM :	return commandZoom(wait); break;
		default:	break;
	}
	return 1;
}

//! set flags
//! @param newval is new value of flag changed
bool AppCommandInterface::setFlag(const std::string &name, const std::string &value, bool &newval)
{
	//test name if exist and get his value
	m_flag_it = m_flags.find(name);
	if (m_flag_it == m_flags.end()) {
		//~ cout <<"error command "<< command << endl;
		debug_message = _("Unrecognized or malformed flag name");
		Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::cLog::LOG_FILE::SCRIPT );
		return false;
	}

	//fix newval dans les cas ou il vaut on ou off, toggle sera fixé après si besoin
	FLAG_VALUES flag_value = convertStrToFlagValues(value);
	if (flag_value==FLAG_VALUES::FV_ON)
		newval = true;
	else if (flag_value==FLAG_VALUES::FV_OFF)
		newval= false;

	switch(m_flag_it->second) {
		case FLAG_NAMES::FN_ANTIALIAS_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagAntialiasLines();

			stcore->setFlagAntialiasLines(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_DRAWING :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->constellationGetFlagLines();

			coreIO->constellationSetFlagLines(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->constellationGetFlagNames();

			coreIO->constellationSetFlagNames(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_ART :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->constellationGetFlagArt();

			coreIO->constellationSetFlagArt(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->constellationGetFlagBoundaries();

			coreIO->constellationSetFlagBoundaries(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_PICK :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->constellationGetFlagIsolateSelected();

			coreIO->constellationSetFlagIsolateSelected(newval);
			break;

		case FLAG_NAMES::FN_STAR_TWINKLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->starGetFlagTwinkle();

			coreIO->starSetFlagTwinkle(newval);
			break;

		case FLAG_NAMES::FN_SHOW_TUI_DATETIME :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = (stapp->ui->FlagShowTuiDateTime = !stapp->ui->FlagShowTuiDateTime);

			stapp->ui->FlagShowTuiDateTime = newval;
			break;

		case FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = (stapp->ui->FlagShowTuiShortObjInfo = !stapp->ui->FlagShowTuiShortObjInfo);

			stapp->ui->FlagShowTuiShortObjInfo = newval;
			break;

		case FLAG_NAMES::FN_MANUAL_ZOOM :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagManualAutoZoom();

			stcore->setFlagManualAutoZoom(newval);
			break;

		case FLAG_NAMES::FN_NAVIGATION :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagNav();

			stcore->setFlagNav(newval);
			break;

		case FLAG_NAMES::FN_LIGHT_TRAVEL_TIME :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagLightTravelTime();

			stcore->setFlagLightTravelTime(newval);
			break;

		case FLAG_NAMES::FN_FOG :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->fogGetFlag();

			coreIO->fogSetFlag(newval);
			break;

		case FLAG_NAMES::FN_ATMOSPHERE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->atmosphereGetFlag();

			if (!newval) coreIO->fogSetFlag(false); // turn off fog with atmosphere
			coreIO->starSetFlagTwinkle(newval); // twinkle stars depending on atmosphere activated
			coreIO->atmosphereSetFlag(newval);
			break;

		case FLAG_NAMES::FN_AZIMUTHAL_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyGridMgrGetFlagShow("GRID_ALTAZIMUTAL");
				coreIO->skyGridMgrFlipFlagShow("GRID_ALTAZIMUTAL");
			} else
				coreIO->skyGridMgrSetFlagShow("GRID_ALTAZIMUTAL", newval);
			break;

		case FLAG_NAMES::FN_EQUATORIAL_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyGridMgrGetFlagShow("GRID_EQUATORIAL");
				coreIO->skyGridMgrFlipFlagShow("GRID_EQUATORIAL");
			} else
				coreIO->skyGridMgrSetFlagShow("GRID_EQUATORIAL", newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyGridMgrGetFlagShow("GRID_ECLIPTIC");
				coreIO->skyGridMgrFlipFlagShow("GRID_ECLIPTIC");
			} else
				coreIO->skyGridMgrSetFlagShow("GRID_ECLIPTIC", newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyGridMgrGetFlagShow("GRID_GALACTIC");
				coreIO->skyGridMgrFlipFlagShow("GRID_GALACTIC");
			} else
				coreIO->skyGridMgrSetFlagShow("GRID_GALACTIC", newval);
			break;

		case FLAG_NAMES::FN_EQUATOR_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_EQUATOR");
				coreIO->skyLineMgrFlipFlagShow("LINE_EQUATOR");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_EQUATOR", newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_GALACTIC_EQUATOR");
				coreIO->skyLineMgrFlipFlagShow("LINE_GALACTIC_EQUATOR");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_GALACTIC_EQUATOR", newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ECLIPTIC");
				coreIO->skyLineMgrFlipFlagShow("LINE_ECLIPTIC");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ECLIPTIC", newval);
			break;

		case FLAG_NAMES::FN_PRECESSION_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_PRECESSION");
				coreIO->skyLineMgrFlipFlagShow("LINE_PRECESSION");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_PRECESSION", newval);
			break;

		case FLAG_NAMES::FN_TROPIC_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_TROPIC");
				coreIO->skyLineMgrFlipFlagShow("LINE_TROPIC");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_TROPIC", newval);
			break;

		case FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_CIRCUMPOLAR");
				coreIO->skyLineMgrFlipFlagShow("LINE_CIRCUMPOLAR");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_CIRCUMPOLAR", newval);
			break;

		case FLAG_NAMES::FN_MERIDIAN_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_MERIDIAN");
				coreIO->skyLineMgrFlipFlagShow("LINE_MERIDIAN");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_MERIDIAN", newval);
			break;

		case FLAG_NAMES::FN_ZENITH_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ZENITH");
				coreIO->skyLineMgrFlipFlagShow("LINE_ZENITH");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ZENITH", newval);
			break;

		case FLAG_NAMES::FN_POLAR_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_CIRCLE_POLAR");
				coreIO->skyLineMgrFlipFlagShow("LINE_CIRCLE_POLAR");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_CIRCLE_POLAR", newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_CENTER :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ECLIPTIC_POLE");
				coreIO->skyLineMgrFlipFlagShow("LINE_ECLIPTIC_POLE");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ECLIPTIC_POLE", newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_POLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_GALACTIC_POLE");
				coreIO->skyLineMgrFlipFlagShow("LINE_GALACTIC_POLE");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_GALACTIC_POLE", newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_CENTER :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_GALACTIC_CENTER");
				coreIO->skyLineMgrFlipFlagShow("LINE_GALACTIC_CENTER");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_GALACTIC_CENTER", newval);
			break;

		case FLAG_NAMES::FN_VERNAL_POINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_VERNAL");
				coreIO->skyLineMgrFlipFlagShow("LINE_VERNAL");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_VERNAL", newval);
			break;

		case FLAG_NAMES::FN_ANALEMMA_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ANALEMMALINE");
				coreIO->skyLineMgrFlipFlagShow("LINE_ANALEMMALINE");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ANALEMMALINE", newval);
			break;

		case FLAG_NAMES::FN_ANALEMMA :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ANALEMMA");
				coreIO->skyLineMgrFlipFlagShow("LINE_ANALEMMA");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ANALEMMA", newval);
			break;

		case FLAG_NAMES::FN_ARIES_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ARIES");
				coreIO->skyLineMgrFlipFlagShow("LINE_ARIES");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ARIES", newval);
			break;

		case FLAG_NAMES::FN_ZODIAC :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_ZODIAC");
				coreIO->skyLineMgrFlipFlagShow("LINE_ZODIAC");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_ZODIAC", newval);
			break;

		case FLAG_NAMES::FN_GREENWICH_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_GREENWICH");
				coreIO->skyLineMgrFlipFlagShow("LINE_GREENWICH");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_GREENWICH", newval);
			break;

		case FLAG_NAMES::FN_VERTICAL_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreIO->skyLineMgrGetFlagShow("LINE_VERTICAL");
				coreIO->skyLineMgrFlipFlagShow("LINE_VERTICAL");
			} else
				coreIO->skyLineMgrSetFlagShow("LINE_VERTICAL", newval);
			break;

		case FLAG_NAMES::FN_PERSONAL :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->personnalGetFlag();

			coreIO->personnalSetFlag(newval);
			break;

		case FLAG_NAMES::FN_PERSONEQ :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->personeqGetFlag();

			coreIO->personeqSetFlag(newval);
			break;

		case FLAG_NAMES::FN_CARDINAL_POINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->cardinalsPointsGetFlag();

			coreIO->cardinalsPointsSetFlag(newval);
			break;

		case FLAG_NAMES::FN_CLOUDS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagClouds();

			stcore->setFlagClouds(newval);
			break;

		case FLAG_NAMES::FN_MOON_SCALED :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagMoonScaled();

			stcore->setFlagMoonScaled(newval);
			break;

		case FLAG_NAMES::FN_SUN_SCALED :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagSunScaled();

			stcore->setFlagSunScaled(newval);
			break;

		case FLAG_NAMES::FN_LANDSCAPE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->landscapeGetFlag();

			coreIO->landscapeSetFlag(newval);
			break;

		case FLAG_NAMES::FN_STARS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->starGetFlag();

			coreIO->starSetFlag(newval);
			break;

		case FLAG_NAMES::FN_STAR_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->starGetFlagName();

			coreIO->starSetFlagName(newval);
			break;

		case FLAG_NAMES::FN_PLANETS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlag();

			coreIO->planetsSetFlag(newval);
			break;

		case FLAG_NAMES::FN_PLANET_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagHints();

			coreIO->planetsSetFlagHints(newval);
			if (coreIO->planetsGetFlagHints()) coreIO->planetsSetFlag(true); // for safety if script turns planets off
			break;

		case FLAG_NAMES::FN_PLANET_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagOrbits() && !coreIO->satellitesGetFlagOrbits();

			coreIO->planetsSetFlagOrbits(newval);
			coreIO->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_PLANETS_AXIS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagAxis();

			coreIO->planetsSetFlagAxis(newval);
			break;

		case FLAG_NAMES::FN_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagOrbits() && !coreIO->satellitesGetFlagOrbits();

			coreIO->planetsSetFlagOrbits(newval);
			coreIO->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_PLANETS_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagOrbits();

			coreIO->planetsSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_SATELLITES_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->satellitesGetFlagOrbits();

			coreIO->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_NEBULAE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->nebulaGetFlag();

			coreIO->nebulaSetFlag(newval);
			break;

		case FLAG_NAMES::FN_NEBULA_HINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->nebulaGetFlagHints();

			coreIO->nebulaSetFlagHints(newval);
			break;

		case FLAG_NAMES::FN_DSO_PICTOGRAMS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getDsoPictograms();

			stcore->setDsoPictograms(newval);
			break;

		case FLAG_NAMES::FN_NEBULA_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->nebulaGetFlagNames();

			if (newval) coreIO->nebulaSetFlagNames(true); // make sure visible
			coreIO->nebulaSetFlagNames(newval);
			break;

		case FLAG_NAMES::FN_MILKY_WAY :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->milkyWayGetFlag();

			coreIO->milkyWaySetFlag(newval);
			break;

		case FLAG_NAMES::FN_ZODIAC_LIGHT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->milkyWayGetFlagZodiacal();

			coreIO->milkyWaySetFlagZodiacal(newval);
			break;

		case FLAG_NAMES::FN_BRIGHT_NEBULAE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->nebulaGetFlagBright();

			coreIO->nebulaSetFlagBright(newval);
			break;

		case FLAG_NAMES::FN_OBJECT_TRAILS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->planetsGetFlagTrails();

			coreIO->planetsSetFlagTrails(newval);
			break;

		case FLAG_NAMES::FN_TRACK_OBJECT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagTracking();

			stcore->setFlagTracking(newval);
			break;

		case FLAG_NAMES::FN_SCRIPT_GUI_DEBUG :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !Log.getDebug();

			Log.setDebug(newval);
			break;

		case FLAG_NAMES::FN_LOCK_SKY_POSITION :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagLockSkyPosition();

			stcore->setFlagLockSkyPosition(newval);
			break;

		case FLAG_NAMES::FN_SHOW_LATLON :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stapp->ui->FlagShowLatLon;

			stapp->ui->FlagShowLatLon =newval;
			break;

		case FLAG_NAMES::FN_OORT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->oortGetFlagShow();

			coreIO->oortSetFlagShow(newval);
			break;

		case FLAG_NAMES::FN_TULLY :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->tullyGetFlagShow();

			coreIO->tullySetFlagShow(newval);
			break;

		case FLAG_NAMES::FN_BODY_TRACE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->bodyTraceGetFlag();

			coreIO->bodyTraceSetFlag(newval);
			break;

		case FLAG_NAMES::FN_COLOR_INVERSE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stapp->flagColorInverse;

			stapp->flagColorInverse = newval;
			break;

		case FLAG_NAMES::FN_STARS_TRACE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->starGetTraceFlag();

			coreIO->starSetTraceFlag(newval);
			break;

		case FLAG_NAMES::FN_STAR_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->starLinesGetFlag();

			coreIO->starLinesSetFlag(newval);
			break;

		case FLAG_NAMES::FN_SKY_DRAW :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->skyDrawGetFlag();

			coreIO->skyDrawSetFlag(newval);
			break;
			
		case FLAG_NAMES::FN_SATELLITES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreIO->hideSatellitesFlag();

			coreIO->setHideSatellites(newval);
			break;
		
		default:
			Log.write("no effect with case " + m_flag_it->first,cLog::LOG_TYPE::L_DEBUG);

			break;
	}
	return true; // flag was found and updated
}

AppCommandInterface::FLAG_VALUES AppCommandInterface::convertStrToFlagValues(const std::string &value)
{
	if (value == "toggle") return FLAG_VALUES::FV_TOGGLE;
	else if (isTrue(value)) return FLAG_VALUES::FV_ON;
	else
		return FLAG_VALUES::FV_OFF;
}

string AppCommandInterface::getErrorString( void )
{
	return( debug_message );
}

int AppCommandInterface::executeCommandStatus()//cyp
{
	if (debug_message.empty()) {
		// if recording commands, do that now
		if (recordable) stapp->scriptMgr->recordCommand(commandline);
		//    cout << commandline << endl;
		return true;
	} else {
		Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT );

		stringstream oss;
		oss << "Could not execute: " << commandline << endl << debug_message;
		Log.write( oss.str(),cLog::LOG_TYPE::L_DEBUG, cLog::cLog::LOG_FILE::SCRIPT );
		cerr << oss.str() << endl;
		return false;
	}
}

int AppCommandInterface::commandFlag()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		bool val;
		if (setFlag( args.begin()->first, args.begin()->second, val/*, trusted*/) == false)
			debug_message = _("Unrecognized or malformed flag argument");

		// rewrite command for recording so that actual state is known (rather than "toggle")
		if (args.begin()->second == "toggle") {
			std::ostringstream oss;
			oss << command << " " << args.begin()->first << " " << val;
			commandline = oss.str();
		}
	} else
		debug_message = _("Unrecognized or malformed command_flag behaviour");

	return executeCommandStatus();
}


int AppCommandInterface::commandGet()
{
	string argStatus = args["status"];
	if (!argStatus.empty()) {
		if (argStatus=="position") {
			stapp->tcpGetPosition();
		} else if (argStatus=="planets_position") {
			stcore->tcpGetPlanetsStatus();
		} else if (argStatus=="constellation") {
			stcore->tcpGetStatus(args["status"]);
		} else if (argStatus=="object") {
			stcore->tcpGetSelectedObjectInfo();
		} else
			debug_message = _("command 'get': unknown status value");
		return executeCommandStatus();
	} else
		debug_message = _("command 'get': unknown argument");

	return executeCommandStatus();
}

int AppCommandInterface::commandSearch()
{
	string argName = args["name"];
	string argMaxObject = args["maxObject"];
	if (!argName.empty()) {
		if (!argMaxObject.empty()) {
			stcore->tcpGetListMatchingObjects(argName, evalInt(argMaxObject));
		} else {
			stcore->tcpGetListMatchingObjects(argName);
		}
	} else
		debug_message = _("command 'search' : missing name argument");

	return executeCommandStatus();
}


int AppCommandInterface::commandPlanetScale()
{
	string argName = args["name"];
	string argScale = args["scale"];
	if (!argName.empty() && !argScale.empty()) {
		coreIO->planetSetSizeScale(argName, evalDouble(argScale));
	} else
		debug_message = _("command 'planet_scale' : missing name or scale argument");

	return executeCommandStatus();
}

int AppCommandInterface::commandWait(unsigned long int &wait)
{
	if ( args["duration"]!="") {
		float fdelay = evalDouble(args["duration"]);
		if (fdelay > 0) wait = (int)(fdelay*1000);
	} else if ( args["action"]=="reset_timer") {
		stapp->scriptMgr->resetTimer();
	} else {
		debug_message = _("command_'wait' : unrecognized or malformed argument name.");
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandPersonal()
{
	string argAction = args["action"];
	if (!argAction.empty()) {
		if (argAction=="load") {
			string fileName=args["filename"];
			if (!fileName.empty())
				fileName = "personal.txt";
			if ( !Utility::isAbsolute(fileName))
				fileName = stapp->scriptMgr->getScriptPath() + fileName;
			coreIO->personalLoad(fileName);
			return executeCommandStatus();
		}
		if (argAction=="clear") {
			coreIO->personalClear();
			return executeCommandStatus();
		}
		debug_message = "command_personal: Unknown 'action' value";
		return executeCommandStatus();
	}
	debug_message = "command_'personal' : unrecognized or malformed argument";

	return executeCommandStatus();
}

int AppCommandInterface::commandSkyDraw()
{
	if (args["action"]=="clear") {
		coreIO->skyDrawClear();
		return executeCommandStatus();
	} else if (args["points"]!="") {
		coreIO->skyDrawAddPoints(args["points"]);
		return executeCommandStatus();
	} else {
		debug_message = "command_'sky_draw' : unrecognized or malformed argument name";
		return executeCommandStatus();
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandDso()
{
	string argAction = args["action"];
	string argPath = args["path"];
	string argName = args["name"];

	if (!argAction.empty()) {
		if (argAction=="load") {
			string path;
			if (!argPath.empty())
				path = argPath;
			else
				path = stapp->scriptMgr->getScriptPath() + argPath;

			//TODO faire que loadNebula gère comme body ses arguments et renvoie un string
			bool status = stcore->loadNebula(evalDouble(args["ra"]), evalDouble(args["de"]), evalDouble(args["magnitude"]),
			                                evalDouble(args["angular_size"]), evalDouble(args["rotation"]), argName,
			                                path + args["filename"], args["credit"], evalDouble(args["texture_luminance_adjust"]),
			                                evalDouble(args["distance"]),args["constellation"], args["type"]);
			if (status==false)
				debug_message = "Error loading nebula.";
			return executeCommandStatus();
		}

		if (argAction == "drop" && !argName.empty() ) {
			// Delete an existing nebulae, but only if was added by a script!
			debug_message = stcore->removeNebula(argName);
			return executeCommandStatus();
		}

		if (argAction == "clear") {
			// drop all nebulae that are not in the original config file
			debug_message = stcore->removeSupplementalNebulae();
			return executeCommandStatus();
		}

		debug_message = _("Command 'dso': unknown action value");
		return executeCommandStatus();
	}

	string argHidden = args["hidden"];
	if ( !argHidden.empty() ) {
		string argType = args["type"];
		if (!argType.empty() ) {
			if (argType=="all")
				if (isTrue(argHidden)) coreIO->dsoHideAll();
				else
					coreIO->dsoShowAll();
			else
				coreIO->dsoSelectType(isTrue(argHidden),argType);

			return executeCommandStatus();
		}

		string argConstellation = args["constellation"];
		if (!argConstellation.empty()) {
			if (argConstellation=="all")
				if (isTrue(argHidden)) coreIO->dsoHideAll();
				else
					coreIO->dsoShowAll();
			else
				coreIO->dsoSelectConstellation(isTrue(argHidden),argConstellation);
			return executeCommandStatus();
		}

		if ( !argName.empty()  ) {
			coreIO->dsoSelectName(argName, isTrue(argHidden));
			return executeCommandStatus();
		}

		debug_message = _("Command 'dso': case hidden unknown argument");
		return executeCommandStatus();
	}

	debug_message = _("command 'dso' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandPersoneq()
{
	string argAction = args["action"];
	if ( !argAction.empty()) {
		if (argAction=="load") {
			string fileName=args["filename"];
			if (fileName.empty())
				fileName = "personal.txt";
			if ( !Utility::isAbsolute(fileName))
				fileName = stapp->scriptMgr->getScriptPath() + fileName;
			coreIO->personeqLoad(fileName);
			return executeCommandStatus();
		}
		if (argAction=="clear") {
			coreIO->personeqClear();
			return executeCommandStatus();
		}
		debug_message = "command_personeq: Unknown 'action' value";
		return executeCommandStatus();
	}
	debug_message = "command_'personeq' : unrecognized or malformed argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandMovetocity()
{
	string argName = args["name"];
	string argCountry = args["country"];
	if (!argName.empty() || !argCountry.empty()) {
		double lon=0.0, lat=0.0;
		int alt=0.0;
		stcore->mCity->getCoordonnatemCity(argName,argCountry, lon, lat, alt);
		//cout << lon << ":" << lat << ":" << alt << endl;
		if (!((lon==0.0) & (lat ==0.0) & (alt ==-100.0))) {//there is nothing in (0,0,-100) it the magic number to say NO CITY
			int delay = (int)(1000.*evalDouble(args["duration"]));
			stcore->moveObserver(lat,lon,alt,delay,argName);
		}
	} else
		debug_message = "command_'movetocity' : unknown argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandBodyTrace()
{
	string argPen = args["pen"];
	if (!argPen.empty()) {
		if (isTrue(argPen)) {
			coreIO->bodyTraceSetPen(true);
			coreIO->bodyTraceSetFlag(true);
		}
		else {
			if (isFalse(argPen)) {
			coreIO->bodyTraceSetPen(false);
			coreIO->bodyTraceSetFlag(false);
			}
			else {
				if (argPen =="toggle") {
					coreIO->bodyTraceSetPen(! coreIO->bodyTraceGetPen());
				}
				else{
					debug_message= _("Command 'body_trace': unknown pen value");
					return executeCommandStatus();
				}
			}
		}
	}
	if (args["action"]=="clear") {
		coreIO->bodyTraceClear();
		return executeCommandStatus();
	}
	if (args["target"]!="") {
		coreIO->bodyTraceBodyChange(args["target"]);
		return executeCommandStatus();
	}
	if (args["hide"]!="") {
		coreIO->bodyTraceHide(args["hide"]);
		return executeCommandStatus();
	}
	debug_message = _("command 'body_trace' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandSuntrace()
{
	string argPen = args["pen"];
	if (!argPen.empty()) {
		coreIO->bodyTraceBodyChange(args["Sun"]);
		if (isTrue(argPen)) { //pen =="true" || pen=="on") {
			coreIO->bodyTraceSetPen(true);
		} else if (isFalse(argPen)) { //pen =="false" || pen=="off") {
			coreIO->bodyTraceSetPen(false);
		} else if (argPen =="toggle") {
			coreIO->bodyTraceSetPen(! coreIO->bodyTraceGetPen());
		}
	}
	if (args["action"]=="clear") {
		coreIO->bodyTraceBodyChange("Sun");
		coreIO->bodyTraceClear();
	}
	if (args["hide"]!="") {
		coreIO->bodyTraceBodyChange("Sun");
		coreIO->bodyTraceHide(args["hide"]);
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandColor()
{
	//gestion de la couleur
	Vec3f Vcolor;
	string argValue = args["value"];
	string argR= args["r"];
	string argG= args["g"];
	string argB= args["b"];
	AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG, argB);
	if (!testColor)
		return executeCommandStatus();

	//gestion du nom de la couleur a modifier
	string argProperty = args["property"];
	if  (argProperty!="") {
		if(argProperty == "constellation_lines")
			coreIO->constellationSetColorLine( Vcolor );
		else if(argProperty == "constellation_names")
			coreIO->constellationSetColorNames( Vcolor );
		else if(argProperty == "constellation_art")
			coreIO->constellationSetColorArt( Vcolor );
		else if(argProperty == "constellation_boundaries")
			coreIO->constellationSetColorBoundaries( Vcolor );
		else if(argProperty == "cardinal_points")
			coreIO->cardinalsPointsSetColor( Vcolor );
		else if(argProperty == "planet_orbits")
			coreIO->planetSetDefaultColor("orbit", Vcolor );
		else if(argProperty == "planet_names")
			coreIO->planetSetDefaultColor("label", Vcolor );
		else if(argProperty == "planet_trails")
			coreIO->planetSetDefaultColor("trail", Vcolor );
		else if(argProperty == "azimuthal_grid")
			coreIO->skyGridMgrSetColor("GRID_ALTAZIMUTAL", Vcolor );
		else if(argProperty == "equator_grid")
			coreIO->skyGridMgrSetColor("GRID_EQUATORIAL", Vcolor );
		else if(argProperty == "ecliptic_grid")
			coreIO->skyGridMgrSetColor("GRID_ECLIPTIC", Vcolor );
		else if(argProperty == "galactic_grid")
			coreIO->skyGridMgrSetColor("GRID_GALACTIC", Vcolor );
		else if(argProperty == "equator_line")
			coreIO->skyGridMgrSetColor("LINE_EQUATOR", Vcolor );
		else if(argProperty == "galactic_line")
			coreIO->skyLineMgrSetColor("LINE_GALACTIC_EQUATOR", Vcolor );
		else if(argProperty == "ecliptic_line")
			coreIO->skyLineMgrSetColor("LINE_ECLIPTIC", Vcolor );
		else if(argProperty == "meridian_line")
			coreIO->skyLineMgrSetColor("LINE_MERIDIAN", Vcolor );
		else if(argProperty == "zenith_line")
			coreIO->skyLineMgrSetColor("LINE_ZENITH", Vcolor );
		else if(argProperty == "polar_circle")
			coreIO->skyLineMgrSetColor("LINE_CIRCLE_POLAR", Vcolor );
		else if(argProperty == "ecliptic_center")
			coreIO->skyLineMgrSetColor("LINE_ECLIPTIC_POLE", Vcolor );
		else if(argProperty == "galactic_pole")
			coreIO->skyLineMgrSetColor("LINE_GALACTIC_POLE", Vcolor );
		else if(argProperty == "galactic_center")
			coreIO->skyLineMgrSetColor("LINE_GALACTIC_CENTER", Vcolor );
		else if(argProperty == "vernal_points")
			coreIO->skyLineMgrSetColor("LINE_VERNAL", Vcolor );
		else if(argProperty == "analemma")
			coreIO->skyLineMgrSetColor("LINE_ANALEMMA", Vcolor );
		else if(argProperty == "analemma_line")
			coreIO->skyLineMgrSetColor("LINE_ANALEMMALINE", Vcolor );
		else if(argProperty == "greenwich_line")
			coreIO->skyLineMgrSetColor("LINE_GREENWICH", Vcolor );
		else if(argProperty == "aries_line")
			coreIO->skyLineMgrSetColor("LINE_ARIES", Vcolor );
		else if(argProperty == "zodiac")
			coreIO->skyLineMgrSetColor("LINE_ZODIAC", Vcolor );
		else if(argProperty == "personal")
			coreIO->personalSetColor( Vcolor );
		else if(argProperty == "personeq")
			coreIO->personeqSetColor( Vcolor );
		else if(argProperty == "vertical_line")
			coreIO->skyLineMgrSetColor("LINE_VERTICAL", Vcolor );
		else if(argProperty == "nebula_names")
			coreIO->nebulaSetColorLabels( Vcolor );
		else if(argProperty == "nebula_circle")
			coreIO->nebulaSetColorCircle( Vcolor );
		else if(argProperty == "precession_circle")
			coreIO->skyLineMgrSetColor("LINE_PRECESSION", Vcolor );
		else if(argProperty == "text_usr_color")
			coreIO->textSetDefaultColor( Vcolor );
		else if ((argProperty == "star_table") && (args["index"] !="" ))
			coreIO->starSetColorTable(evalInt(args["index"]), Vcolor );
		else {
			debug_message = _("Command 'color': unknown property");
			executeCommandStatus(); // renvoie de l'erreur
		}
		return executeCommandStatus(); // as well
	}

	debug_message = _("Command 'color': unknown expected argument 'property'");
	return executeCommandStatus();
}

int AppCommandInterface::commandIlluminate()
{
	string argHP = args["hp"];
	string argDisplay = args["display"];

	if (!argHP.empty() && isTrue(argDisplay)) {
		string select_type, identifier;
		select_type = "hp";
		identifier = argHP;
		stcore->selectObject(select_type, identifier);
		double ra, de;
		stcore->getDeRa(&ra,&de);
		stcore->unSelect();

		string ang_size=args["size"];
		if (ang_size=="") ang_size="0.0";

		double r=1.0, g=1.0, b=1.0, rotation=0.0;

		if (args["r"]!="")
			r=evalDouble(args["r"]);
		if (args["g"]!="")
			g=evalDouble(args["g"]);
		if (args["b"]!="")
			b=evalDouble(args["b"]);

		if (args["rotation"]!="")
			rotation = evalDouble(args["rotation"]);

		string argFileName = args["filename"];
		if (!argFileName.empty()) {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::IMAGE);
			if (!myFile.exist()) {
				debug_message = _("command 'illuminate': filename not found");
				return executeCommandStatus();
			}
			//TODO fix error
			coreIO->illuminateLoad(myFile.toString(), ra, de, evalDouble(ang_size), "I-"+identifier, r, g, b,rotation);
		} else
			coreIO->illuminateLoad("", ra, de, evalDouble(ang_size), "I-"+identifier, r, g, b, rotation);
		return executeCommandStatus();
	}

	if (!argHP.empty() && isFalse(argDisplay)) {
		string identifier;
		identifier = "I-"+argHP;
		debug_message = coreIO->illuminateRemove( identifier);
		return executeCommandStatus();
	}

	if (args["action"]=="clear") {
		debug_message=coreIO->illuminateRemoveAll();
		return executeCommandStatus();
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandPrint()
{
	string argName = args["name"];
	string argValue = args["value"];
	if (!argValue.empty()) {
		stringstream oss;

		if (argName.empty())
			argName = "NONE";

		oss << "[" << argName <<"] " << argValue;
		Log.write(oss.str(),  cLog::LOG_TYPE::L_WARNING, cLog::cLog::LOG_FILE::SCRIPT);
		Log.write(oss.str(),  cLog::LOG_TYPE::L_WARNING);
	} else
		debug_message = "command 'print': missing value";

	return executeCommandStatus();
}

int AppCommandInterface::commandSet()
{
	if (args["atmosphere_fade_duration"]!="") coreIO->atmosphereSetFadeDuration(evalDouble(args["atmosphere_fade_duration"]));
	else if (args["auto_move_duration"]!="") stcore->setAutomoveDuration( evalDouble(args["auto_move_duration"]));
	else if (args["constellation_art_fade_duration"]!="") coreIO->constellationSetArtFadeDuration(evalDouble(args["constellation_art_fade_duration"]));
	else if (args["constellation_art_intensity"]!="") coreIO->constellationSetArtIntensity(evalDouble(args["constellation_art_intensity"]));
	else if (args["light_pollution_limiting_magnitude"]!="") stcore->setLightPollutionLimitingMagnitude(evalDouble(args["light_pollution_limiting_magnitude"]));
	else if (args["heading"]!="") {
		if (args["heading"]=="default") {
			stcore->setDefaultHeading();
		}
		else {
			float fdelay = evalDouble(args["duration"]);
			double heading = evalDouble(args["heading"]);
			if (fdelay <= 0) fdelay = 0;
			if (args["heading"][0] == '+') {
				heading += stcore->getHeading();
				if (heading > 180) heading -= 360;
				stringstream oss;
				oss << "FROM: " << stcore->getHeading() << " TO: " << heading;
				Log.write( oss.str(),cLog::LOG_TYPE::L_INFO, cLog::cLog::LOG_FILE::SCRIPT );
			}
			if (args["heading"][0] == '-') {
				heading += stcore->getHeading();
				if (heading < -180) heading += 360;
				stringstream oss;
				oss << "FROM: " << stcore->getHeading() << " TO: " << heading;
				Log.write( oss.str(),cLog::LOG_TYPE::L_INFO, cLog::cLog::LOG_FILE::SCRIPT );
			}
			stcore->setHeading(heading, (int)(fdelay*1000));
		}
	} else if (args["home_planet"]!="") {
		if (args["home_planet"]=="default") stcore->setHomePlanet("Earth"); else stcore->setHomePlanet(args["home_planet"]);
	} else if (args["landscape_name"]!="") {
		if ( args["landscape_name"]=="default") stcore->setInitialLandscapeName();
		else
			stcore->setLandscape(args["landscape_name"]);
	} else if (args["line_width"]!="") stcore->setLineWidth(evalDouble(args["line_width"]));
	else if (args["max_mag_nebula_name"]!="") coreIO->nebulaSetMaxMagHints(evalDouble(args["max_mag_nebula_name"]));
	else if (args["max_mag_star_name"]!="") coreIO->starSetMaxMagName(evalDouble(args["max_mag_star_name"]));
	else if (args["moon_scale"]!="") {
		stcore->setMoonScale(evalDouble(args["moon_scale"]));
	} else if (args["sun_scale"]!="") {
		stcore->setSunScale(evalDouble(args["sun_scale"]));
	} else if (args["milky_way_texture"]!="") {
		if(args["milky_way_texture"]=="default") coreIO->milkyWayRestoreDefault();
		else {
			if (args["milky_way_intensity"]!="")
				coreIO->milkyWayChange(stapp->scriptMgr->getScriptPath() + args["milky_way_texture"], evalDouble(args["milky_way_intensity"]) );
			else
				coreIO->milkyWayChange(stapp->scriptMgr->getScriptPath() + args["milky_way_texture"], 1.f );
		}
	} else if (args["sky_culture"]!="") {
		if (args["sky_culture"]=="default") stcore->setInitialSkyCulture();
		else
			stcore->setSkyCultureDir(args["sky_culture"]);
	} else if (args["sky_locale"]!="") {
		if ( args["sky_locale"]=="default") stcore->setInitialSkyLocale();
		else
			stcore->setSkyLanguage(args["sky_locale"]);
	} else if (args["ui_locale"]!="") stapp->setAppLanguage(args["ui_locale"]);
	else if (args["star_mag_scale"]!="") coreIO->starSetMagScale(evalDouble(args["star_mag_scale"]));
	else if (args["star_size_limit"]!="") coreIO->starSetSizeLimit(evalDouble(args["star_size_limit"]));
	else if (args["planet_size_limit"]!="") stcore->setPlanetsSizeLimit(evalDouble(args["planet_size_limit"]));
	else if (args["star_scale"]!="") {
		float scale = evalDouble(args["star_scale"]);
		coreIO->starSetScale(scale);
		coreIO->planetsSetScale(scale);
	} else if (args["star_twinkle_amount"]!="") coreIO->starSetTwinkleAmount(evalDouble(args["star_twinkle_amount"]));
	else if (args["star_fader_duration"]!="") coreIO->starSetDuration(evalDouble(args["star_fader_duration"]));
	else if (args["star_limiting_mag"]!="") coreIO->starSetLimitingMag(evalDouble(args["star_limiting_mag"]));
	else if (args["time_zone"]!="") stapp->getSpaceDate()->setCustomTimezone(args["time_zone"]);
	else if (args["ambient_light"]!="") {
		if (args["ambient_light"]=="increment") {
			coreIO->uboSetAmbientLight(coreIO->uboGetAmbientLight()+0.01);
		}
		else if (args["ambient_light"]=="decrement"){
			coreIO->uboSetAmbientLight(coreIO->uboGetAmbientLight()-0.01);
		}
		else{
			coreIO->uboSetAmbientLight(evalDouble(args["ambient_light"]));
		}
	} else if (args["text_fading_duration"]!="") coreIO-> textFadingDuration(Utility::strToInt(args["text_fading_duration"]));
	else if (args["milky_way_fader_duration"]!="") coreIO->milkyWaySetDuration(evalDouble(args["milky_way_fader_duration"]));
	else if (args["milky_way_intensity"]!="") {
		if (args["milky_way_intensity"]=="default")
			coreIO->milkyWayRestoreIntensity();
		else
			coreIO->milkyWaySetIntensity(evalDouble(args["milky_way_intensity"]));
		// safety feature to be able to turn back on
		if (coreIO->milkyWayGetIntensity()) coreIO->milkyWaySetFlag(true);
	} else if (args["zoom_offset"]!="") {
		stcore->setViewOffset(evalDouble(args["zoom_offset"]));
	}
	else if(args["startup_time_mode"]!="") stapp->StartupTimeMode = args["startup_time_mode"];
	else if(args["date_display_format"]!="") stapp->getSpaceDate()->setDateFormatStr(args["date_display_format"]);
	else if(args["time_display_format"]!="") stapp->getSpaceDate()->setTimeFormatStr(args["time_display_format"]);
	else if(args["mode"]!="") stcore->switchMode(args["mode"]);
	else if(args["screen_fader"]!="") stapp->screenFader->setIntensity(evalDouble(args["screen_fader"]));
	else if(args["stall_radius_unit"]!="") coreIO->cameraSetRotationMultiplierCondition(evalDouble(args["stall_radius_unit"]));
	else if(args["tully_color_mode"]!="") coreIO->tullySetColor(args["tully_color_mode"]);
	else {
		debug_message = "command_'set': unknown argument";
		Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::cLog::LOG_FILE::SCRIPT );
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandShutdown()
{
	if (args["action"] =="now")	{
		stapp->quit();
	} else
		debug_message = "Bad shutdown request.";

	return executeCommandStatus();
}

int AppCommandInterface::commandConfiguration()
{
	string argAction = args["action"];
	if (!argAction.empty()) {
		if(argAction=="load") {
			stapp->init();
			return executeCommandStatus();
		} else if(argAction=="save") {
			stapp->saveCurrentConfig(AppSettings::Instance()->getConfigFile());
			return executeCommandStatus();
		} else
			debug_message = "command 'configuration': unknown action value";
	}

	string argModule = args["module"];

	if (!argModule.empty()){
		if (argModule=="star_lines"){

			if (argAction == "clear") {
				coreIO->starLinesClear();
				return executeCommandStatus();
			}

			string argName = args["name"];
			if (argName.empty()){
				debug_message = "command 'star_lines' missing name parameter";
				return executeCommandStatus();
			}
			bool binaryMode = Utility::strToBool(args["binary_mode"],false);

			if (argAction == "load") {
				if (binaryMode)
					coreIO->starLinesLoadBinCat(argName);
				else
					coreIO->starLinesLoadCat(argName);
				return executeCommandStatus();
			} else
				debug_message = "command 'configuration': unknown starNavigator action argument";
		} else
		if (argModule=="star_navigator"){

			if (argAction == "clear") {
				coreIO->starNavigatorClear();
				return executeCommandStatus();
			}

			string argName = args["name"];
			if (argName.empty()){
				debug_message = "command 'configuration', star_navigator missing name argument";
				return executeCommandStatus();
			}

			bool binaryMode = Utility::strToBool(args["binary_mode"],false);

			if (argAction == "load") {
				string argMode = args["mode"];

				if (argMode.empty()){
					debug_message = "command 'configuration', star_navigator missing mode argument";
					return executeCommandStatus();
				} else {
					if (argMode == "raw") {
						coreIO->starNavigatorLoadRaw(argName);
						return executeCommandStatus();
					} else 
					if (argMode == "sc") {
						coreIO->starNavigatorLoad(argName, binaryMode);
						return executeCommandStatus();
					} else 
					if (argMode == "other") {
						coreIO->starNavigatorLoadOther(argName);
						return executeCommandStatus();
					} else {
						debug_message = "command 'configuration': unknown starNavigator mode parameter";
						return executeCommandStatus();
					}
				}
			} else
			if (argAction == "save") {
				coreIO->starNavigatorSave(argName, binaryMode);
			} else
				debug_message = "command 'configuration': unknown starNavigator action argument";
		} else
			debug_message = "command 'configuration': unknown module argument";
	}
	debug_message = "command 'configuration': unknown argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandConstellation()
{
	string argName = args["name"];
	transform(argName.begin(),argName.end(),argName.begin(), ::toupper);
	if (argName.empty()) {
		debug_message = "command 'constellation': missing name";
		return executeCommandStatus();
	}

	string argIntensity = args["intensity"];
	if (!argIntensity.empty()) {
		coreIO->constellationSetArtIntensity(argName, evalDouble(argIntensity));
		return executeCommandStatus();
	}

	string type = args["type"];
	if (type.empty()) {
		debug_message = "command 'constellation': missing type";
		return executeCommandStatus();
	}

	Vec3f Vcolor;
	string argColor =  args["color"];
	string argR= args["r"];
	string argG= args["g"];
	string argB= args["b"];
	AppCommandColor testColor(Vcolor, debug_message, argColor, argR,argG, argB);
	if (!testColor) {
		//~ cout << "color read error" << endl;
		return executeCommandStatus();
	} else
		debug_message.clear();

	if (type=="line") {
		coreIO->constellationSetLineColor(argName, Vcolor);
		return executeCommandStatus();
	} else if (type=="label") {
		coreIO->constellationSetColorNames(argName, Vcolor);
		return executeCommandStatus();
	} else {
		debug_message = "command 'constellation': unknown type";
		return executeCommandStatus();
	}
}

int AppCommandInterface::commandExternalMplayer()
{
	string argAction = args["action"];
	string argFileName = args["filename"];
	if (!argAction.empty()) {
		if (argAction=="play" && args["filename"]!="") {
			if (Utility::isAbsolute(args["filename"]))
				media->externalPlay(args["filename"]);
			else
				media->externalPlay(stapp->scriptMgr->getScriptPath()+args["filename"]);
			return executeCommandStatus();
		}
		if (argAction=="stop") {
			media->externalStop();
			return executeCommandStatus();
		}
		if (argAction=="pause") {
			media->externalPause();
			return executeCommandStatus();
		}
		if (argAction=="reset") {
			media->externalReset();
			return executeCommandStatus();
		}
		debug_message = _("Command 'externalMplayer': unknown action value");
		return executeCommandStatus();
	}

	string argJumpRelative=args["jump_relative"];
	if (!argJumpRelative.empty()) {
		media->externalJumpRelative(evalDouble(argJumpRelative));
		return executeCommandStatus();
	}

	if (args["jump_absolute"]!="") {
		media->externalJumpAbsolute(evalDouble(args["jump_absolute"]));
		return executeCommandStatus();
	}
	if (args["speed"]!="") {
		media->externalSpeed(evalDouble(args["speed"]));
		return executeCommandStatus();
	}
	if (args["volume"]!="") {
		media->externalVolume(evalDouble(args["volume"]));
		return executeCommandStatus();
	}
	if (args["execute"]!="") {
		media->externalExecute(args["execute"]);
		return executeCommandStatus();
	}
	debug_message= _("command 'externalmplayer' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandExternalViewer()
{
	string argAction = args["action"];
	string argFileName = args["filename"];

	if (argAction=="play" && !argFileName.empty()) {
		if (argFileName.size()<5) {
			debug_message = _("command 'externalviewer' : fileName too short");
			return executeCommandStatus();
		}

		string action1="NONE";;
		string extention=argFileName.substr(argFileName.length()-3,3);

		if (extention=="avi" || extention=="mov" || extention=="mpg" || extention=="mp4") {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::VIDEO);
			if (myFile)
				action1="mplayer -fs -osdlevel 0 "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer media fileName not found";
		} else if (extention=="mp3" || extention=="ogg") {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::AUDIO);
			if (myFile)
				action1="cvlc "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer audio fileName not found";
		} else if (extention==".sh") {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::DATA);
			if (myFile)
				action1="sh "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer shell script fileName not found";

		} else if (extention=="swf") {
			FilePath myFile  = FilePath(argFileName);
			if (myFile)
				action1="gnash "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer swf fileName not found";
		} else if (extention=="png") {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::IMAGE);
			if (myFile)
				action1="qiv "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer png fileName not found";
		}

		if (action1 != "NONE") {
			if (!CallSystem::useSystemCommand(action1))
				debug_message = "command 'externalviewer': system error";
			return executeCommandStatus();
		}
		return executeCommandStatus();
	}

	if (argAction=="stop") {
		CallSystem::killAllPidFromVLC();
		if (!media->externalMplayerIsAlive())
			CallSystem::killAllPidFromMPlayer();
		return executeCommandStatus();
	}

	debug_message = _("command 'externalviewer' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandClear()
{
	string argState = args["state"];

	if (argState == "variable") {
		deleteVar();
		return executeCommandStatus();
	}

	// TODO move to stelcore
	// set sky to known, standard states (used by scripts for simplicity)
	executeCommand("set home_planet Earth");

	if (argState == "natural") {
		executeCommand("flag atmosphere on");
		executeCommand("flag landscape on");
	} else {
		executeCommand("flag atmosphere off");
		executeCommand("flag landscape off");
	}

	// turn off all labels
	executeCommand("flag azimuthal_grid off");
	executeCommand("flag meridian_line off");
	executeCommand("flag zenith_line off");
	executeCommand("flag polar_circle off");
	executeCommand("flag ecliptic_center off");
	executeCommand("flag galactic_pole off");
	executeCommand("flag galactic_center off");
	executeCommand("flag vernal_points off");
	executeCommand("flag analemma off");
	executeCommand("flag analemma_line off");
	executeCommand("flag aries_line off");
	executeCommand("flag zodiac off");
	executeCommand("flag personal off");
	executeCommand("flag personeq off");
	executeCommand("flag greenwich_line off");
	executeCommand("flag vertical_line off");
	executeCommand("flag cardinal_points off");
	executeCommand("flag constellation_art off");
	executeCommand("flag constellation_drawing off");
	executeCommand("flag constellation_names off");
	executeCommand("flag constellation_boundaries off");
	executeCommand("flag ecliptic_line off");
	executeCommand("flag equatorial_grid off");
	executeCommand("flag equator_line off");
	executeCommand("flag galactic_line off");
	executeCommand("flag tropic_lines off");
	executeCommand("flag circumpolar_circle off");
	executeCommand("flag precession_circle off");
	executeCommand("flag fog off");
	executeCommand("flag nebula_hints off");
	executeCommand("flag nebula_names off");
//	executeCommand("flag nebula_text_names off");
	executeCommand("flag object_trails off");
	executeCommand("flag planet_names off");
	executeCommand("flag planet_orbits off");
	executeCommand("flag planets_orbits off");
	executeCommand("flag satellites_orbits off");
	executeCommand("flag show_tui_datetime off");
	executeCommand("flag star_names off");
	executeCommand("flag show_tui_short_obj_info off");

	// make sure planets, stars, etc. are turned on!
	// milkyway is left to user, for those without 3d cards
	executeCommand("flag stars on");
	executeCommand("flag planets on");
	executeCommand("flag nebulae on");

	// also deselect everything, set to default fov and real time rate
	executeCommand("deselect");
	executeCommand("timerate rate 1");
	executeCommand("zoom auto initial");

	return executeCommandStatus();
}

int AppCommandInterface::commandMeteors()
{
	string argZhr=args["zhr"];
	if (! argZhr.empty()) {
		stcore->setMeteorsRate(evalInt(args["zhr"]));
	} else
		debug_message = "command 'meteors' : no zhr argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandLandscape()
{
	string argAction = args["action"];
	if (!argAction.empty()) {
		if (argAction == "load") {
			// textures are relative to script
			args["path"] = stapp->scriptMgr->getScriptPath();
			stcore->loadLandscape(args); //TODO retour d'erreurs
		} else
			debug_message = "command 'landscape' : invalid action parameter";
	} else
		debug_message = "command 'landscape' : unknown argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandText()
{
	string argAction = args["action"];

	if (argAction=="clear") {
		coreIO->textClear();
		return executeCommandStatus();
	}

	string argName = args["name"];
	if (argName.empty()) {
		debug_message = _("Command 'text': argument 'name' needed");
		return executeCommandStatus();
	}

	if (argAction=="drop") {
		coreIO->textDel(argName);
		return executeCommandStatus();
	}

	string argDisplay = args["display"];
	string argString = args["string"];

	if (!argAction.empty()) {
		if (argString.empty()) {
			debug_message = _("Command 'text': argument 'string' needed");
			return executeCommandStatus();
		}

		if (argAction=="update") {
			coreIO->textNameUpdate(argName, argString);
			return executeCommandStatus();
		} else if (argAction=="load") {
			string argAzimuth = args["azimuth"];
			string argAltitude = args["altitude"];
			if( !argAzimuth.empty() && !argAltitude.empty()) {
				float azimuth = evalDouble(argAzimuth);
				float altitude = evalDouble(argAltitude);
				int durationText = 1000*evalDouble(args["duration"]);
				printf("Durée d'apparition du texte : %i\n", durationText);
				string argSize = args["size"];

				//gestion de la couleur
				Vec3f Vcolor;
				string argValue = args["color_value"];
				string argR= args["r"];
				string argG= args["g"];
				string argB= args["b"];
				AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG, argB);
				if (testColor)
					coreIO->textAdd(argName,argString, altitude, azimuth, argSize, Vcolor, durationText);
				else {
					debug_message.clear();
					coreIO->textAdd(argName,argString, altitude, azimuth, argSize, durationText);
				}
				// test si l'utilisateur spécifie argDisplay
				if (!argDisplay.empty()) {
					if ( isTrue(argDisplay) )
						coreIO->textDisplay(argName,true);
					else
						coreIO->textDisplay(argName,false);
					return executeCommandStatus();
				}
				return executeCommandStatus();
			} else {
				debug_message = _("Command 'text': parameter 'azimuth' or 'altitude' needed");
				return executeCommandStatus();
			}
		}
	}

	// test argDisplay en commande indépendante
	if (!argDisplay.empty()) {
		if ( isTrue(argDisplay) )
			coreIO->textDisplay(argName,true);
		else
			coreIO->textDisplay(argName,false);
		return executeCommandStatus();
	}

	debug_message = _("Command 'text': unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandSkyCulture()
{
	string argPath = args["path"];
	if (!argPath.empty() && args["action"]=="load") {
		if (!stcore->loadSkyCulture(argPath))
			debug_message = "Error loading sky culture from path specified.";
	} else
		debug_message = "command_sky_culture : path or action missing";
	return executeCommandStatus();
}

int AppCommandInterface::commandScript()
{
	string argAction = args["action"];
	string filen = args["filename"];
	if (!argAction.empty()) {
		if (argAction=="end") {
			stapp->scriptMgr->cancelScript();
			coreIO->textClear(); // del all usr text
			media->audioMusicHalt();
			media->imageDropAllNoPersistent();
			swapCommand = false;
		} else if (argAction=="play" && !filen.empty()) {
			int le=-1;

			string file_with_path = FilePath(filen, FilePath::TFP::SCRIPT);
			string new_script_path="";

			for (unsigned int i=0; i<=file_with_path.length(); i++) {
				if (file_with_path[i]=='/') {
					le=i;
				}
			}
			for (int i=0; i<=le; i++) {
				new_script_path+=file_with_path[i];
			}

			if( !stapp->scriptMgr->playScript(file_with_path) ) {
				debug_message = "Unable to execute script : " + file_with_path;
				Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::cLog::LOG_FILE::SCRIPT );
			} else {
				stapp->scriptMgr->setPath(new_script_path);
			}
		} else if (argAction=="record") {
			stapp->scriptMgr->recordScript(filen);
			recordable = 0;  // don't record this command!
		} else if (argAction=="cancelrecord") {
			stapp->scriptMgr->cancelRecordScript();
			recordable = 0;  // don't record this command!
		} else if (argAction=="pause" && !stapp->scriptMgr->isPaused()) {
			stapp->scriptMgr->pauseScript();
		} else if (argAction=="pause" || argAction=="resume") {
			stapp->scriptMgr->resumeScript();
		} else if (argAction=="faster") {
			stapp->scriptMgr->fasterScript();
		} else if (argAction=="slower") {
			stapp->scriptMgr->slowerScript();
		} else
			debug_message = "command_script : unknown parameter from 'action' argument";
		return executeCommandStatus();
	}
	debug_message = "command_'script' : missing action argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandAudio()
{
	//gestion du volume
	string argVolume = args["volume"];
	if (!argVolume.empty()) {
		if (argVolume == "increment") {
			media->audioVolumeIncrement();
		} else if (argVolume == "decrement") {
			media->audioVolumeDecrement();
		} else
			media->audioSetVolume(evalInt(argVolume));
		return executeCommandStatus();
	}

	//gestion de la pause des audio dans les scripts
	string argMusicPause= args["nopause"];
	if (!argMusicPause.empty()) {
		media->audioSetMusicToPause(isTrue(args["nopause"]));
		return executeCommandStatus();
	}

	//gestion des actions
	string argAction = args["action"];
	string argFileName = args["filename"];

	if (!argAction.empty()) {
		if (argAction =="drop") {
			media->audioMusicDrop();
		} else if (argAction=="sync") {
			media->audioMusicSync();
		} else if (argAction=="pause") {
			media->audioMusicPause();
		} else if (argAction=="resume") {
			media->audioMusicResume();
		} else if (argAction=="play"){
			if (!argFileName.empty() ) {
				if (FilePath myFile  = FilePath(argFileName, FilePath::TFP::AUDIO)) {
					media->audioMusicLoad(myFile);
					media->audioMusicPlay(isTrue(args["loop"]));
					return executeCommandStatus();
				} else {
					debug_message = _("command 'audio': filename not found");
					return executeCommandStatus();
				}
			} else {
				debug_message = _("command 'audio': filename not found");
				return executeCommandStatus();
			}
		}
	} else {
		debug_message = _("command 'audio': unknown action value");
		return executeCommandStatus();
	}

	debug_message= _("command 'audio' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandImage(/*bool trusted*/)
{
	string argAction = args["action"];
	if (argAction=="purge") {
		media->imageDropAll();
		return executeCommandStatus();
	}

	string argName = args["name"];
	if (argName.empty()) {
		debug_message = _("Image argument name required.");
		return executeCommandStatus();
	}

	string argFileName = args["filename"];
	if (argAction=="drop") {
		media->imageDrop(evalString(args["name"]));
		return executeCommandStatus();
	}

	if (argAction=="twice") {
		media->imageClone(argName,2);
		return executeCommandStatus();
	}
	if (argAction=="thrice") {
		media->imageClone(argName,3);
		return executeCommandStatus();
	}


	if (argAction=="load" && !argFileName.empty()) {
		FilePath myFile  = FilePath(evalString(argFileName), FilePath::TFP::IMAGE);
		if (!myFile.exist()) {
			debug_message = _("command 'image': filename not found");
			return executeCommandStatus();
		}

		string argCoordinate = args["coordinate_system"];
		bool mipmap = 0; // Default off for historical reasons
		if (isTrue(args["mipmap"]))
			mipmap = 1;

		//TODO récupérer une erreur compréhensible plutot qu'un int ?
		int status = media->imageLoad(myFile.toString(), evalString(argName), argCoordinate, mipmap);
		if (status!=1) {
			debug_message = _("Unable to load image: ") + argName;
			return executeCommandStatus();
		}
	}

	if (media->imageSet(evalString(argName)) != true) {
		debug_message = _("Unable to find image: ") + argName;
		return executeCommandStatus();
	}

	//initialisation de toutes les variables
	string argDuration = args["duration"];
	string argAlpha = args["alpha"];
	string argScale = args["scale"];
	string argRotation = args["rotation"];
	string argXpos = args["xpos"];
	string argYpos = args["ypos"];
	string argAltitude = args["altitude"];
	string argAzimuth = args["azimuth"];
	string argPersistent = args["persistent"];

	if (!argAlpha.empty())
		media->imageSetAlpha(evalDouble(argAlpha), evalDouble(argDuration));

	if (!argScale.empty())
		media->imageSetScale(evalDouble(argScale), evalDouble(argDuration));

	if (!argRotation.empty())
		media->imageSetRotation(evalDouble(argRotation), evalDouble(argDuration));

	if (!argXpos.empty() || !argYpos.empty())
		media->imageSetLocation(evalDouble(argXpos), !argXpos.empty(),
		                        evalDouble(argYpos), !argYpos.empty(),
		                        evalDouble(argDuration));

	// for more human readable scripts, as long as someone doesn't do both...
	if (!argAltitude.empty() || !argAzimuth.empty() )
		media->imageSetLocation(evalDouble(argAltitude), !argAltitude.empty(),
		                        evalDouble(argAzimuth), !argAzimuth.empty(),
		                        evalDouble(argDuration));

	if (!argPersistent.empty()) {
		if (isTrue(argPersistent))
			media->imageSetPersistent(true);
		else
			media->imageSetPersistent(false);
	}


	string argKeyColor = args["keycolor"];
	if (!argKeyColor.empty()) {
		if (isTrue(argKeyColor))
			media->imageSetKeyColor(true);
		else
			media->imageSetKeyColor(false);
	}

	Vec3f Vcolor;
	string argValue = args["color_value"];
	string argR= args["r"];
	string argG= args["g"];
	string argB= args["b"];
	AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG,argB);
	if (testColor) {
		string argIntensity = args["intensity"];
		if (!argIntensity.empty())
			media->imageSetKeyColor(Vcolor,Utility::strToDouble(argIntensity)) ;
		else
			media->imageSetKeyColor(Vcolor) ;
	} else
		debug_message.clear();

	return executeCommandStatus();
}

int AppCommandInterface::commandSelect()
{
	// default is to deselect current object
	stcore->unSelect();

	string select_type, identifier;

	if (args["constellation"]=="zodiac") {
		stcore->selectZodiac();
		return executeCommandStatus();
	}

	if (args["hp"]!="") {
		select_type = "hp";
		identifier = args["hp"];
	} else if (args["star"]!="") {
		select_type = "star";
		identifier = args["star"];
	} else if (args["planet"]!="") {
		select_type = "planet";
		identifier = args["planet"];
		if (args["planet"] == "home_planet")
			identifier = stcore->getObservatory()->getHomePlanetEnglishName();
			//~ identifier = stcore->getObservatory()->getHomePlanet()->getEnglishName();
	} else if (args["nebula"]!="") {
		select_type = "nebula";
		identifier = args["nebula"];
	} else if (args["constellation"]!="") {
		select_type = "constellation";
		identifier = args["constellation"];
	} else if (args["constellation_star"]!="") {
		select_type = "constellation_star";
		identifier = args["constellation_star"];
	} else {
		select_type = "";
		debug_message= "command 'select' : no object found";
		return executeCommandStatus();
	}

	stcore->selectObject(select_type, identifier);

	// determine if selected object pointer should be displayed
	if (isFalse(args["pointer"]))
		stcore->setFlagSelectedObjectPointer(false);
	else
		stcore->setFlagSelectedObjectPointer(true);

	return executeCommandStatus();
}

int AppCommandInterface::commandDeselect()
{
	string argConstellation = args["constellation"];
	if ( !argConstellation.empty())
		stcore->unsetSelectedConstellation(argConstellation);
	else
		stcore->deselect();
	return executeCommandStatus();
}


int AppCommandInterface::commandComment()
{
	swapCommand = true;
	return executeCommandStatus();
}


int AppCommandInterface::commandUncomment()
{
	swapCommand = false;
	return executeCommandStatus();
}


int AppCommandInterface::commandLook()
{
	string argAz  = args["azimuth"];
	string argAlt = args["altitude"];

	if(!argAz.empty() && !argAlt.empty()){

		string argTime = args["duration"];

		if(argTime.empty()){
			coreIO->lookAt(stod(argAz), stod(argAlt));
		}
		else{
			coreIO->lookAt(stod(argAz), stod(argAlt), stod(argTime));
		}

		return executeCommandStatus();
	}

	//change direction of view
	string argD_az  = args["delta_azimuth"];
	string argD_alt = args["delta_altitude"];
	if (!argD_az.empty() || !argD_alt.empty()) {
		// immediately change viewing direction
		stcore->panView(evalDouble(argD_az), evalDouble(argD_alt), evalDouble(args["duration"]));
	} else {
		debug_message = _("Command 'look_at': wrong argument");
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandStarLines()
{
	if (args["action"]=="drop") {
		coreIO->starLinesDrop();
		return executeCommandStatus();
	}
	if (args["load"]!="") {
		coreIO->starLinesLoadData(stapp->scriptMgr->getScriptPath() + args["load"]);
		return executeCommandStatus();
	}
	if (args["asterism"]!="") {
		coreIO->starLinesLoadAsterism(args["asterism"]);
		return executeCommandStatus();
	}
	debug_message = _("Command 'star_lines': wrong argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandPosition()
{
	string argAction = args["action"];
	if (argAction == "save") {
		stcore->getmBackup();
		return executeCommandStatus();
	}
	if (argAction == "load") {
		stcore->setmBackup();
		return executeCommandStatus();
	}
	debug_message = _("Command 'position': unknown parameter");
	return executeCommandStatus();
}

int AppCommandInterface::commandZoom(unsigned long int &wait)
{
	double duration = Utility::strToPosDouble(args["duration"]);
	string argAuto = args["auto"];
	string argManual = args["manual"];

	if (!argAuto.empty()) {
		// auto zoom using specified or default duration
		if (args["duration"]=="") duration = stcore->getAutoMoveDuration();

		if (argAuto=="out") {
			if (isTrue(argManual)) stcore->autoZoomOut(duration, 0, 1);
			else stcore->autoZoomOut(duration, 0, 0);
		} else if (argAuto=="initial") stcore->autoZoomOut(duration, 1, 0);
		else if (isTrue(argManual)) {
			stcore->autoZoomIn(duration, 1);  // have to explicity allow possible manual zoom
		} else stcore->autoZoomIn(duration, 0);

	} else if (args["fov"]!="") {
		// zoom to specific field of view
		stcore->zoomTo( evalDouble(args["fov"]), evalDouble(args["duration"]));

	} else if (args["delta_fov"]!="") stcore->setFov(stcore->getFov() + evalDouble(args["delta_fov"]));
	// should we record absolute fov instead of delta? isn't usually smooth playback
	else if (args["center"]=="on") {
		float cdelay=5;
		if ( args["duration"]!="") cdelay = evalDouble(args["duration"]);
		stcore->gotoSelectedObject();  // center view to selected objet
		if (cdelay > 0) wait = (int)(cdelay*1000);
	} else {
		debug_message = _("Command 'zoom': unknown argument");
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandTimerate()
{
	string argRate = args["rate"];
	string argAction = args["action"];
	string argStep = args["step"];

	// NOTE: accuracy issue related to frame rate
	if (!argRate.empty()) {
		coreIO->timeSetSpeed(evalDouble(argRate)*JD_SECOND);
		stapp->temp_time_velocity = coreIO->timeGetSpeed();
		stapp->FlagTimePause = 0;
	} else if (argAction=="pause") {
		// TODO why is this in stelapp?  should be in stelcore - Rob
		stapp->FlagTimePause = !stapp->FlagTimePause;
		if (stapp->FlagTimePause) {
			// TODO pause should be all handled in core methods
			stapp->temp_time_velocity = coreIO->timeGetSpeed();
			coreIO->timeSetSpeed(0);
		} else {
			coreIO->timeSetSpeed(stapp->temp_time_velocity);
		}
	} else if (argAction=="resume") {
		stapp->FlagTimePause = 0;
		coreIO->timeSetSpeed(stapp->temp_time_velocity);

	} else if (argAction=="increment") {
		// speed up time rate
		stapp->FlagTimePause = 0;
		double s = coreIO->timeGetSpeed();

		double sstep = 2.;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>=JD_SECOND) s*=sstep;
		else if (s<-JD_SECOND) s/=sstep;
		else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
		else if (s>=-JD_SECOND && s<0.) s=0.;
		coreIO->timeSetSpeed(s);
		stapp->temp_time_velocity = coreIO->timeGetSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);

	} else if (argAction=="sincrement") {
		// speed up time rate
		stapp->FlagTimePause = 0;
		double s = coreIO->timeGetSpeed();
		double sstep = 1.05;
		Observer *observatory = stcore->getObservatory();
		if ((abs(s)<3) && (observatory->getAltitude()>150E9)) s=3;
		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>=JD_SECOND) s*=sstep;
		else if (s<-JD_SECOND) s/=sstep;
		else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
		else if (s>=-JD_SECOND && s<0.) s=0.;
		coreIO->timeSetSpeed(s);
		stapp->temp_time_velocity = coreIO->timeGetSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else if (argAction=="decrement") {
		stapp->FlagTimePause = 0;
		double s = coreIO->timeGetSpeed();

		double sstep = 2.;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>JD_SECOND) s/=sstep;
		else if (s<=-JD_SECOND) s*=sstep;
		else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
		else if (s>0. && s<=JD_SECOND) s=0.;
		coreIO->timeSetSpeed(s);
		stapp->temp_time_velocity = coreIO->timeGetSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else if (argAction=="sdecrement") {
		stapp->FlagTimePause = 0;
		double s = coreIO->timeGetSpeed();
		double sstep = 1.05;
		Observer *observatory = stcore->getObservatory();
		if ((abs(s)<3) && (observatory->getAltitude()>150E9)) s=-3;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>JD_SECOND) s/=sstep;
		else if (s<=-JD_SECOND) s*=sstep;
		else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
		else if (s>0. && s<=JD_SECOND) s=0.;
		coreIO->timeSetSpeed(s);
		stapp->temp_time_velocity = coreIO->timeGetSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else
		debug_message = _("Command 'time_rate': unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandMoveto()
{
	string argLat = args["lat"];
	string argLon = args["lon"];
	string argAlt = args["alt"];

	if(argLat.empty()) argLat = args["latitude"];
	if(argLon.empty()) argLon = args["longitude"];
	if(argAlt.empty()) argAlt = args["altitude"];


	if (argLat.empty() && argLon.empty() && argAlt.empty()) {
		debug_message = "command 'move_to' : missing lat && lon && alt";
		return executeCommandStatus();
	}

	Observer *observatory = stcore->getObservatory();

	double lat = observatory->getLatitude();
	double lon = observatory->getLongitude();
	double alt = observatory->getAltitude();

	string name = observatory->getName();
	string argName = args["name"];
	int delay;

	if (!argName.empty()) name = argName;

	if (!argLat.empty()) {
		if (argLat=="default")
			lat = observatory->getDefaultLatitude();
		else
			lat = evalDouble(argLat);
	}
	if (!argLon.empty()) {
		if (argLon=="default")
			lon = observatory->getDefaultLongitude();
		else
			lon = evalDouble(argLon);
	}
	if (!argAlt.empty()) {
		if (argAlt=="default") alt = observatory->getDefaultAltitude();
		else {
			if (argAlt[0] == '+' || argAlt[0] == '-')
				alt += evalDouble(argAlt);
			else
				alt = evalDouble(argAlt);
		}
	}

	delay = (int)(1000.*evalDouble(args["duration"]));

	//TODO recevoir les erreurs de moveObserver
	stcore->moveObserver(lat,lon,alt,delay,name);

	return executeCommandStatus();
}

int AppCommandInterface::commandMultiplier()
{
	// script rate multiplier
	string argRate = args["rate"];
	if (!argRate.empty()) {
		stapp->setTimeMultiplier(evalDouble(argRate));
		if (!stapp->FlagTimePause)
			stapp->temp_time_velocity = coreIO->timeGetSpeed();
		return executeCommandStatus();
	}

	string argAction = args["action"];
	if (!argAction.empty()) {
		if (argAction=="increment") {
			// speed up script rate
			double s = stapp->getTimeMultiplier();
			double sstep = 10.0;

			if( !args["step"].empty() )
				sstep = evalDouble(args["step"]);

			stapp->setTimeMultiplier(s*sstep);
			if (!stapp->FlagTimePause)
				stapp->temp_time_velocity = coreIO->timeGetSpeed();

			// for safest script replay, record as absolute amount
			commandline = "multiplier rate " + Utility::doubleToStr(s*sstep);
			return executeCommandStatus();
		}
		if (argAction=="decrement") {
			// slow rate
			double s = stapp->getTimeMultiplier();
			double sstep = 10.0;

			if( !args["step"].empty() )
				sstep = evalDouble(args["step"]);

			if (!stapp->FlagTimePause)
				stapp->temp_time_velocity = coreIO->timeGetSpeed();
			stapp->setTimeMultiplier(s/sstep);

			// for safest script replay, record as absolute amount
			commandline = "multiplier rate " + Utility::doubleToStr(s/sstep);
			return executeCommandStatus();
		}
		debug_message = _("Command 'multiplier_rate': unknown action value");
		return executeCommandStatus();
	}
	debug_message = _("Command 'multiplier_rate': unknown argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandMedia()
{
	string argAction = args["action"];
	if (!argAction.empty() ) {

		if (argAction == "play") {
			string type = args["type"];
			if (type.empty()) {
				debug_message = "Command 'media' argument action need argument 'type'";
				return executeCommandStatus();
			}
			string videoName = args["videoname"];
			string audioName = args["audioname"];
			string argName =  args["name"];
			string argPosition = args["position"];

			FilePath::TFP localRepertory;
			if (type == "VR360" || type == "VRCUBE")
				localRepertory = FilePath::TFP::VR360;
			else
				localRepertory = FilePath::TFP::MEDIA;

			FilePath fileVideo = FilePath(videoName, localRepertory);
			if (!fileVideo.exist()) {
				debug_message = _("command 'media': file videoname not found");
				return executeCommandStatus();
			}

			if (!audioName.empty()) {
				if ( audioName =="auto" ) {
					// On teste si un fichier de langue existe on prend videoName et on rajoute -fr par exemple à la place de son extention et on rajoute apres ogg
					audioName = videoName;
					if (audioName.size()>5) {
						audioName[audioName.size()-1]='.';
						audioName[audioName.size()-2]=stcore->getSkyLanguage()[1];
						audioName[audioName.size()-3]=stcore->getSkyLanguage()[0];
						audioName[audioName.size()-4]='_';
						audioName = audioName+"ogg";
		
						FilePath fileAudio = FilePath(audioName, FilePath::TFP::MEDIA);
						if (fileAudio.exist()) {
								Log.write("command 'media':: succesfull locale audio "+audioName, cLog::LOG_TYPE::L_INFO, cLog::cLog::LOG_FILE::SCRIPT);
								media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition );
							}
						else
							Log.write("command 'media':: locale audio not found "+audioName, cLog::LOG_TYPE::L_WARNING, cLog::cLog::LOG_FILE::SCRIPT);
						}
				} else {
					// si l'audio existe sous forme -fr.ogg alors on le modifie en appliquant la langue de la sky_culture
					if (audioName.size()>8 && audioName[audioName.size()-7]=='-') { // internationalisation possible
						FilePath fileAudio = FilePath(audioName, stcore->getSkyLanguage() );
						if (!fileAudio.exist()) {
							Log.write("command 'media':: locale audio not found ", cLog::LOG_TYPE::L_WARNING, cLog::cLog::LOG_FILE::SCRIPT);
							media->playerPlay(type, fileVideo.toString(), argName, argPosition );
						} else
							media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition );
					} else { //fichier simple sans internationalisation
						FilePath fileAudio = FilePath(audioName, localRepertory);
						if (!fileAudio.exist()) {
							Log.write("command 'media':: audio not found ", cLog::LOG_TYPE::L_WARNING, cLog::cLog::LOG_FILE::SCRIPT);
							media->playerPlay(type, fileVideo.toString(), argName, argPosition);
						} else
							media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition);
					}
				}
			} else {
					media->playerPlay(type, fileVideo.toString(), argName, argPosition);
				}

			Vec3f Vcolor;
			string argValue = args["color_value"];
			string argR= args["r"];
			string argG= args["g"];
			string argB= args["b"];
			AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG,argB);
			if (testColor) {
				string argIntensity = args["intensity"];
				if (!argIntensity.empty())
					media->setKeyColor(Vcolor,Utility::strToDouble(argIntensity)) ;
				else
					media->setKeyColor(Vcolor) ;
			} else
				debug_message.clear();
		
			string argKeyColor = args["keycolor"];
			if (!argKeyColor.empty()) {
				if (isTrue(argKeyColor))
					media->setKeyColor(true);
				else
					media->setKeyColor(false);
			}
			return executeCommandStatus();

		} else if (argAction == "stop") {
			media->playerStop();
			return executeCommandStatus();
		} else if (argAction == "pause") {
			media->playerPause();
			return executeCommandStatus();
		} else if (argAction == "jump") {
			media->playerJump(evalDouble(args["value"]));
			return executeCommandStatus();
		}
	}

	debug_message = _("command 'media': unknown parameter");

	return executeCommandStatus();
}


int AppCommandInterface::commandDomemasters()
{
	string argAction = args["action"];
	if (!argAction.empty()) {
		if (argAction == "snapshot") {
			stapp->takeScreenshot();
			return executeCommandStatus();
		}
		if (argAction == "record") {
			if (stapp->get_m_OutputFrameNumber() == 0) {
				stapp->fixScriptFps();
				stapp->set_m_OutputFrameNumber(1);
			} else {
				stapp->set_m_OutputFrameNumber(0);
				stapp->fixMaxFps();
			}
			return executeCommandStatus();
		} else
			debug_message = _("Command 'domemasters': unknown action value");
	}
	debug_message = _("command 'domemasters': unknown argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandDate()
{
	//cas du jday
	string argJday = args["jday"];
	if (!argJday.empty() ) {
		//TODO coreIO doit renvoyer un code rectour erreur
		coreIO->setJDay( evalDouble(argJday) );
		return executeCommandStatus();
	}

	//cas du local
	string argLocal = args["local"];
	if (!argLocal.empty() ) {
		// ISO 8601-like format [[+/-]YYYY-MM-DD]Thh:mm:ss (no timzone offset, T is literal)
		double jd;
		string new_date;

		if (argLocal[0] == 'T') {
			// set time only (don't change day)
			string sky_date = stapp->getSpaceDate()->getISO8601TimeLocal(coreIO->getJDay());
			new_date = sky_date.substr(0,10) + argLocal;
		} else new_date = argLocal;

		if (SpaceDate::StringToJday( new_date, jd )) {
			coreIO->setJDay(jd - (stapp->getSpaceDate()->getGMTShift(jd) * JD_HOUR));
		} else {
			debug_message = _("Error parsing date local");
		}
		return executeCommandStatus();
	}

	//cas de l'utc
	string argUtc = args["utc"];
	if (!argUtc.empty()) {
		double jd;
		if (SpaceDate::StringToJday(argUtc, jd ) ) {
			coreIO->setJDay(jd);
		} else {
			debug_message = _("Error parsing date utc");
		}
		return executeCommandStatus();
	}

	//cas du relative
	string argRelative = args["relative"];
	if (!argRelative.empty()) { // value is a float number of days
		double days = evalDouble(argRelative);
		//~ const Body* home = stcore->getObservatory()->getHomePlanet();
		const Body* home = stcore->getObservatory()->getHomeBody();
		if (home==nullptr) {
			debug_message = _("Error date local, vous devez être sur un astre pour utiliser l'argument relative");
			return executeCommandStatus();
		}
		float sol_local_day = home->getSolLocalDay();
		if (abs(sol_local_day)>366.0) sol_local_day=1.0;
		coreIO->setJDay(coreIO->getJDay() + days*sol_local_day );
		return executeCommandStatus();
	}

	//cas du relative_year
	string argRelativeYear = args["relative_year"];
	if (!argRelativeYear.empty()) {
		int years = evalInt(argRelativeYear);
		stcore->setJDayRelative(years,0);
		return executeCommandStatus();
	}

	//cas du relative_month
	string argRelativeMonth = args["relative_month"];
	if (!argRelativeMonth.empty()) {
		int months = evalInt(argRelativeMonth);
		stcore->setJDayRelative(0, months);
		return executeCommandStatus();
	}

	//cas du sidereal
	string argSidereal = args["sidereal"];
	if (!argSidereal.empty()) { // value is a float number of sidereal days
		double days = evalDouble(argSidereal);
		//~ const Body* home = stcore->getObservatory()->getHomePlanet();
		const Body* home = stcore->getObservatory()->getHomeBody();
		if (home==nullptr) {
			debug_message = _("Error date local, vous devez être sur un astre pour utiliser l'argument sideral");
			return executeCommandStatus();
		}
		//TODO wtf ???
		//if (home->getEnglishName() != "Solar System Observer")
		float sol_sidereal_day = home->getSiderealDay();
		if (abs(sol_sidereal_day)>366.0) sol_sidereal_day=1.0;
		days *= sol_sidereal_day;
		// stcore->getTimeMgr()/*Navigation()*/->setJDay(stcore->getTimeMgr()->getJDay() + days );
		coreIO->setJDay(coreIO->getJDay()+days);
		return executeCommandStatus();
	}

	//cas du load
	string argLoad = args["load"];
	if (!argLoad.empty()) {
		if (argLoad=="current") {
			// set date to current date
			coreIO->setJDay(SpaceDate::JulianFromSys());
		} else if (argLoad=="preset") {
			// set date to preset (or current) date, based on user setup
			// TODO: should this record as the actual date used?
			if (stapp->StartupTimeMode=="preset" || stapp->StartupTimeMode=="Preset")
				coreIO->setJDay(stapp->PresetSkyTime - stapp->getSpaceDate()->getGMTShift(stapp->PresetSkyTime) * JD_HOUR);
			else coreIO->setJDay(SpaceDate::JulianFromSys());
		} else
			debug_message = _("Command 'date': unknown load value");
		return executeCommandStatus();
	}

	//cas du Sun
	string argSun = args["sun"];
	if (!argSun.empty()) {
		if (argSun=="set") {
			double tmp=coreIO->dateSunSet(coreIO->getJDay(), coreIO->observatoryGetLongitude(), coreIO->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreIO->setJDay(tmp);
		} else if (argSun=="rise") {
			double tmp=coreIO->dateSunRise(coreIO->getJDay(), coreIO->observatoryGetLongitude(), coreIO->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreIO->setJDay(tmp);
		} else if (argSun=="meridian") {
			double tmp=coreIO->dateSunMeridian(coreIO->getJDay(), coreIO->observatoryGetLongitude(), coreIO->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreIO->setJDay(tmp);
		} else if (argSun=="midnight") {
			double tmp=coreIO->dateSunMeridian(coreIO->getJDay(), coreIO->observatoryGetLongitude()+180, -coreIO->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreIO->setJDay(tmp);
		} else
			_("Command 'date': unknown sun value");
		return executeCommandStatus();
	}
	debug_message = _("command 'date' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandBody()
{
	//gestion des actions
	string argAction = args["action"];
	string argName = args["name"];
	string argMode = args["mode"];

	// traitement des OJM
	if ((argMode=="in_universe" || argMode=="in_galaxy") && !argAction.empty()) {
		if (argAction =="load") {
			string argFileName = args["filename"];
			argFileName = argFileName +"/"+argFileName +".ojm";
			Vec3f Position( evalDouble(args["pos_x"]), evalDouble(args["pos_y"]), evalDouble(args["pos_z"] ));
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::MODEL3D);
			coreIO->BodyOJMLoad(argMode, argName, myFile.toString(), myFile.getPath() , Position, evalDouble(args["scale"]));
			return executeCommandStatus();
		}
		if (argAction =="remove") {
			coreIO->BodyOJMRemove(argMode, argName);
			return executeCommandStatus();
		}
		if (argAction =="clear") {
			coreIO->BodyOJMRemoveAll(argMode);
			return executeCommandStatus();
		}
	}

	if (!argAction.empty()) {
		if (argAction == "load" ) {
			// textures relative to script
			args["path"] = stapp->scriptMgr->getScriptPath();
			// Load a new solar system object
			debug_message = stcore->addSolarSystemBody(args);

		} else if (argAction == "drop" && argName != "") {
			// Delete an existing object, but only if was added by a script!
			debug_message  = stcore->removeSolarSystemBody( argName );
		} else if (argAction == "clear") {
			// drop all bodies that are not in the original config file
			string error_string = stcore->removeSupplementalSolarSystemBodies();
			if (error_string != "" ) {
				debug_message = error_string;
				Log.write( debug_message,cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT );
			}
		} else if (argAction == "initial") {
			stcore->initialSolarSystemBodies();
		} else {
			debug_message = "command 'body' : unknown action argument";
		}
		return executeCommandStatus();
	}

	//gestion des actions
	if (!argName.empty() ) {

		//sous cas hidden
		string argHidden = args["hidden"];
		if (!argHidden.empty()) {
			if (isTrue(argHidden)) {
				stcore->setPlanetHidden(args["name"], true);
			} else if (isFalse(argHidden)) {
				stcore->setPlanetHidden(args["name"], false);
			} else if (argHidden == "toggle") {
				stcore->setPlanetHidden(args["name"], !stcore->getPlanetHidden(args["name"]));
			} else
				debug_message = _("Command 'body': unknown hidden value");
			return executeCommandStatus();
		}

		//sous cas orbit
		string argOrbit = args["orbit"];
		if (!argOrbit.empty()) {
			if (isTrue(argOrbit)) {
				coreIO->planetsSetFlagOrbits(args["name"], true);
			} else if (isFalse(argOrbit)) {
				coreIO->planetsSetFlagOrbits(args["name"], false);
			} else
				debug_message = _("Command 'body': unknown orbit value");
			return executeCommandStatus();
		}


		string argColor = args["color"];
		string argName = args["name"];
		if (!argColor.empty()) {
			//gestion de la couleur
			Vec3f Vcolor;
			string argR= args["r"];
			string argG= args["g"];
			string argB= args["b"];
			string colorValue = args["color_value"];
			AppCommandColor testColor(Vcolor, debug_message, colorValue, argR,argG, argB);
			if (!testColor)
				return executeCommandStatus();

			coreIO->planetSetColor(argName, argColor, Vcolor);
			//~ if (argColor == "orbit") {
				//~ coreIO->planetSetColorOrbits(args["name"],Vcolor);
			//~ } else if (argColor == "name") {
				//~ coreIO->planetSetColorName(args["name"],Vcolor);
			//~ } else if (argColor == "trail") {
				//~ coreIO->planetSetColorTrail(args["name"],Vcolor);
			//~ } else
				//~ debug_message = _("Command 'body': unknown color value");
			return executeCommandStatus();
		}
		debug_message = _("Command 'body': case name unknown argument");
		return executeCommandStatus();
	}

	debug_message = _("Command 'body': unknown argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandCamera(unsigned long int &wait)
{
	//gestion des actions
	string argAction = args["action"];
	string argName = args["name"];

	if (argAction.empty()) {
		debug_message = "command 'camera' : action argument";
		return executeCommandStatus();
	}

	if(argAction == "align_with"){

		string argBody = args["body"];

		if (argBody.empty()) {
			debug_message = "command 'align_with' : missing body";
			return executeCommandStatus();
		}

		string argDuration = args["duration"];
		double duration =0;

		if ( ! argDuration.empty()) {
			duration = stod(argDuration);
		}

		bool result = coreIO->cameraAlignWithBody(argBody, duration);

		if (!result)
			debug_message = "error align_with_body body";

		return executeCommandStatus();
	}


	if(argAction == "transition_to"){
		string argTarget = args["target"];

		if (argTarget.empty()) {
			debug_message = "command 'transition_to' : missing target";
			return executeCommandStatus();
		}

		if(argTarget == "point"){
			bool result = coreIO->cameraTransitionToPoint("temp_point");

			if (!result)
				debug_message = "error transition_to point";
			return executeCommandStatus();
		}

		if(argTarget == "body"){
			argName = args["name"];

			if (argName.empty()) {
				debug_message = "command 'transition_to' : missing target name";
				return executeCommandStatus();
			}

			bool result = coreIO->cameraTransitionToBody(argName);

			if (!result)
				debug_message = "error transition_to body";
			return executeCommandStatus();
		}

		debug_message = "command 'transition_to' : unknown target arg";
		return executeCommandStatus();
	}

	if(argAction == "move_to"){

		string argTarget = args["target"];

		if (argTarget.empty()) {
			debug_message = "command 'move_to' : missing target";
			return executeCommandStatus();
		}

		if(argTarget == "point"){
			string argX = args["x"];
			string argY = args["y"];
			string argZ = args["z"];
			string argTime = args["duration"];

			if(argX.empty() || argY.empty() || argZ.empty()){
				debug_message = "command 'move_to point' : missing a coordinate";
				return executeCommandStatus();
			}

			bool result;

			if(argTime.empty())
				result = coreIO->cameraMoveToPoint(stod(argX), stod(argY), stod(argZ));
			else {
				result = coreIO->cameraMoveToPoint(stod(argX), stod(argY), stod(argZ),stod(argTime));
				wait = evalInt(argTime)*1000;
			}

			if (!result)
				debug_message = "error move_to point";
			return executeCommandStatus();
		}

		if(argTarget == "body"){
			string argBodyName = args["body_name"];
			string argTime = args["duration"];

			if(argBodyName.empty() || argTime.empty()){
				debug_message = "command 'move_to body' : missing a body_name or time";
				return executeCommandStatus();
			}

			string argAltitude = args["altitude"];

			bool result;

			if(argAltitude.empty())
				result = coreIO->cameraMoveToBody(argBodyName, stod(argTime));
			else
				result = coreIO->cameraMoveToBody(argBodyName, stod(argTime), stod(argAltitude));

			if (!result)
				debug_message = "error move_to body";
			else
				wait = evalInt(argTime)*1000;

			return executeCommandStatus();
		}

		debug_message = "command 'camera' : unknown target arg";
		return executeCommandStatus();
	}

	if(argAction == "save"){
		bool result;

		string argFileName = args["filename"];

		if (argFileName.empty())
			result = coreIO->cameraSave();
		else
			result = coreIO->cameraSave(argFileName);

		if (!result)
			debug_message = "error saving camera";
		return executeCommandStatus();
	}

	if(argAction == "load"){
		string argFileName = args["filename"];

		if(argFileName.empty()){
			debug_message = "command 'camera load' : missing file name";
			return executeCommandStatus();
		}

		bool result = coreIO->loadCameraPosition(argFileName);

		if (!result)
			debug_message = "error loading CameraAnchor";
		return executeCommandStatus();
	}

	if(argAction == "lift_off"){

		string altStr = args["altitude"];
		string durationStr = args["duration"];

		if(altStr.empty()){
			debug_message = "command 'camera lift_off' : missing altitude";
			return executeCommandStatus();
		}

		if(durationStr.empty()){
			durationStr = "3";
		}

		ostringstream command;

		command << "moveto altitude " + altStr + " duration " + durationStr << endl;
		command << "wait duration " + durationStr << endl;
		command << "camera action transition_to target point" << endl;

		return stapp->scriptMgr->addScriptFirst(command.str());
	}

	if (argName.empty()) {
		debug_message = "command 'camera' : missing name";
		return executeCommandStatus();
	}

	if (argAction == "create" ) {
		// load an anchor via script
		bool result = coreIO->cameraAddAnchor(args);
		if (!result)
			debug_message = "error creating CameraAnchor";
		return executeCommandStatus();
	}

	if (argAction == "drop") {
		// Delete an existing anchor
		bool result = coreIO->cameraRemoveAnchor(argName);
		if (!result)
			debug_message = "error drop CameraAnchor";
		return executeCommandStatus();
	}

	if (argAction == "switch") {
		// change the anchor
		bool result = coreIO->cameraSwitchToAnchor(argName);
		if (!result)
			debug_message = "error switch CameraAnchor";
		return executeCommandStatus();
	}

	if(argAction == "follow_rotation"){
		string valueStr = args["value"];

		bool value = valueStr == "true";

		bool result = coreIO->cameraSetFollowRotation(argName, value);
		if (!result)
			debug_message = "error camera follow_rotation";
		return executeCommandStatus();
	}

	debug_message = "command 'camera' : unknown action argument";
	return executeCommandStatus();
}

//
// variable management
//

string AppCommandInterface::evalString (const string &var)
{
	var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return var;
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return var_it->second;
}

double AppCommandInterface::evalDouble (const string &var)
{
	//~ cout << "evalDouble "<< var;
	if (var.empty())
		return 0.0;

	var_it = variables.find(var);
	if (var_it == variables.end()) //pas trouvé donc on renvoie la valeur de la chaine
		return Utility::strToDouble(var);
	else // trouvé on renvoie la valeur de ce qui est stocké en mémoire
		return Utility::strToDouble(var_it->second);
}


int AppCommandInterface::evalInt (const string &var)
{
	double tmp=evalDouble(var);
	return (int) tmp;
}


int AppCommandInterface::commandDefine()
{
	if (args.begin() != args.end()) {
		string mArg = args.begin()->first;
		string mValue = args.begin()->second;

		//~ printf("Command define :  %s => %s\n",mArg.c_str(), mValue.c_str());
		if (mValue == "random") {
			float value = (float)rand()/RAND_MAX* (max_random-min_random)+ min_random;
			variables[mArg] = Utility::floatToStr(value);
		} else {
			//~ printf("mValue = %s\n", mValue.c_str());
			//~ cout << "Cette valeur de mValue vaut " << evalDouble(mValue) << endl;
			variables[mArg] = Utility::doubleToString( evalDouble(mValue) );
		}
	} else {
		debug_message = "Unexpected error in command_define";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandAdd()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		string mArg = args.begin()->first;
		string mValue = args.begin()->second;
		var_it = variables.find(mArg);

		if (var_it == variables.end()) { //pas trouvé donc on renvoie la valeur de la chaine
			debug_message = "not possible to operate with undefined variable";
			return executeCommandStatus();
		} else { // trouvé on renvoie la valeur de ce qui est stocké en mémoire
			double tmp = Utility::strToDouble( variables[mArg] ) +evalDouble (mValue);
			variables[mArg] = Utility::floatToStr(tmp);
			return executeCommandStatus();
		}
	} else { //est ce que ce cas peut vraiment se produire ?
		debug_message = "unexpected error in command_addition";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandMultiply()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {

		string mArg = args.begin()->first;
		string mValue = args.begin()->second;

		var_it = variables.find(mArg);
		if (var_it == variables.end()) {
			debug_message = "not possible to operate with undefined variable";
			return executeCommandStatus();
		} else {
			double tmp = Utility::strToDouble( variables[mArg] ) * Utility::strToDouble (mValue);
			variables[mArg] = Utility::floatToStr(tmp);
			return executeCommandStatus();
		}
	} else {
		debug_message = "unexpected error in command__addition";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandStruct()
{
	// if case
	string argIf = args["if"];
	if (!argIf.empty()) {
		if (argIf=="else") {
			swapIfCommand = ! swapIfCommand;
			return executeCommandStatus();
		}
		if (argIf=="end") {
			swapIfCommand = false;
			return executeCommandStatus();
		}
		if (evalDouble(argIf) == 0)
			swapIfCommand = true;

		return executeCommandStatus();
	}

	//comment case
	string argComment = args["comment"];
	if (!argComment.empty()) {
		if (isTrue(argComment)) {
			swapCommand = true;
			return executeCommandStatus();
		} else {
			swapCommand = false;
			return executeCommandStatus();
		}
	}

	//loop case
	string argLoop = args["loop"];
	if (!argLoop.empty()) {
		if (argLoop =="end") {
			swapCommand = false; //cas ou nbrLoop était inférieur à 1
			stapp->scriptMgr->setLoop(false);
			stapp->scriptMgr->initIterator();
			return executeCommandStatus();
		}

		int nbrLoop = evalInt(argLoop);
		if (nbrLoop < 1) {
			swapCommand = true;
			return executeCommandStatus();
		} else if (nbrLoop >1) {
			stapp->scriptMgr->setLoop(true);
			stapp->scriptMgr->setNbrLoop(nbrLoop-1);
			return executeCommandStatus();
		}
	}

	if (args["print"] =="var") {
		printVar();
	}

	return executeCommandStatus();
}


int AppCommandInterface::commandRandom()
{
	bool status = false;
	string argMin = args["min"];
	string argMax = args["max"];

	if (!argMin.empty()) {
		min_random = evalDouble(argMin);
		status = true;
	}
	if (!argMax.empty()) {
		max_random = evalDouble(argMax);
		status = true;
	}
	if (status == false)
		debug_message= _("unknown random parameter");
	return executeCommandStatus();
}


void AppCommandInterface::printVar()
{
	if (variables.size() ==0) {
		std::cout << "No variable available" << std::endl;
		return;
	}

	for (var_it=variables.begin(); var_it!=variables.end(); ++var_it) {
		cout << "-----------------" << endl;
		std::cout << var_it->first << " => " << var_it->second << '\n';
	}
}

void AppCommandInterface::deleteVar()
{
	variables.clear();
}
