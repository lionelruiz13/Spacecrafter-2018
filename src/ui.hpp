/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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

// Class which handles the User Interface
// TODO : get rid of the SDL macro def and types
// need the creation of an interface between s_gui and SDL

#ifndef _UI_H
#define _UI_H

namespace s_gui {

enum S_GUI_VALUE {
	S_GUI_MOUSE_LEFT,
	S_GUI_MOUSE_RIGHT,
	S_GUI_MOUSE_MIDDLE,
	S_GUI_MOUSE_WHEELUP,
	S_GUI_MOUSE_WHEELDOWN,
	S_GUI_PRESSED,
	S_GUI_RELEASED
};
}


#include "ui_tui.hpp"
#include "app_settings.hpp"

#define TUI_SCRIPT_MSG "Select and exit to run."

class App;
class Core;
class Media;
class CoreIO;
class s_font;
class ScriptMgr;
class AppCommandInterface;
class JoypadController;

enum MODIFIER { NONE, CTRL, KWIN, SUPER, SHIFT, ALT };

class UI {
	friend class AppCommandInterface;
	friend class App;

public:
	UI(Core *, CoreIO* , App * _app, Media* _media);	// Create the ui. Need to call init() before use
	virtual ~UI();		// Delete the ui
	void init(const InitParser& conf);		// Initialize the ui.
	void draw(void);		// Display the ui
	void updateTimeouts(int delta_time);		// Update changing values

	void drawGravityUi(void);	// Draw simple gravity text ui.
	// Handle mouse clics
	int handleClic(Uint16 x, Uint16 y, s_gui::S_GUI_VALUE button, s_gui::S_GUI_VALUE state);
	// Handle mouse move
	int handleMove(int x, int y);
	// Handle key press and release
	int handleKeys(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state);

	// Text UI
	void initTui(void);
	void localizeTui();
	void drawTui(void);		// Display the tui
	int handleKeysTui(SDL_Scancode key, s_tui::S_TUI_VALUE state);

	// Update all the tui widgets with values taken from the core parameters
	void tuiUpdateWidgets(void);
	void tuiUpdateIndependentWidgets(void); // For widgets that aren't tied directly to current settings

	double getMouseCursorTimeout() {
		return MouseCursorTimeout;
	}

	bool getFlagShowGravityUi() {
		return FlagShowGravityUi;
	}
	bool getFlagShowTuiDateTime() {
		return FlagShowTuiDateTime;
	}
	bool getFlagShowTuiShortObjInfo() {
		return FlagShowTuiShortObjInfo;
	}

	// Whether tui menu is currently displayed
	void setFlagShowTuiMenu(const bool flag);
	bool getFlagShowTuiMenu() {
		return FlagShowTuiMenu;
	}

	//! handle Joystick Hat
	void handleJoyHat(SDL_JoyHatEvent E);


	void handleInputs(SDL_Event E);

	void setHandleKeyOnVideo(bool videoOn) {
		handleKeyOnVideo = videoOn;
	}

	void moveMouseAlt(double x);

	void moveMouseAz(double x);

	void moveLat(double x);

	void moveLon(double x);

	void raiseHeight(double x);

	void lowerHeight(double x);

	void zoomIn();

	void stopZoomIn();

	void rightClick();

	void leftClick();

	void zoomOut();

	void stopZoomOut();

	void lowerHeight();

	void stopLowerHeight();

	void raiseHeight();

	void stopRaiseHeight();

	void speedDecrease();

	void stopSpeedDecrease();

	void speedIncrease();

	void stopSpeedIncrease();

	void pauseScript();

	void centerMouse();

	void turnUp();

	void stopTurnUp();

	void turnDown();

	void stopTurnDown();

	void turnRight();

	void stopTurnRight();

	void turnLeft();

	void stopTurnLeft();

	void executeCommand(std::string command);

	void handleDeal();

	int handleKeysOnVideo(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state);

	int handlKkeysOnTui(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state);

	int handleKeyPressed(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state);

	int handleKeysReleased(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state);

private:
	Core * core;		// The main core can be accessed because UI is a friend class (TODO fix that)
	CoreIO * coreIO;    // toutes les fonctions entrées sorties de Core
	App * app;			// The main application instance
	Media * media;

	// Flags and variables
	int FlagShowFps;
	int FlagShowLatLon;
	int FlagShowFov;
	int FlagNumberPrint;
	int PosDateTime;
	int PosObjectInfo;
	int PosMenuM;
	int FlagShowPlanetname;

	// Text UI
	s_font * tuiFont;		// The standard tui font - separate from gui so can reload on the fly
	float FontSizeGeneral;
	std::string FontNameGeneral;
	std::string FontNameMenu;
	float FontSizeMenuTui;

	bool FlagEnableTuiMenu;
	bool FlagShowGravityUi;
	bool FlagShowTuiMenu;
	bool FlagShowTuiDateTime;
	bool FlagShowTuiShortObjInfo;

	// Mouse control options
	double MouseCursorTimeout;  // seconds to hide cursor when not used.  0 means no timeout
	bool is_dragging, has_dragged;
	int previous_x, previous_y;
	bool FlagMouseUsableInScript;

	////////////////////////////////////////////////////////////////////////////
	// Text UI components
	s_tui::Branch* tui_root;

	// Menu branches
	s_tui::MenuBranch* tui_menu_location;
	s_tui::MenuBranch* tui_menu_time;
	s_tui::MenuBranch* tui_menu_general;
	s_tui::MenuBranch* tui_menu_stars;
	s_tui::MenuBranch* tui_menu_colors;
	s_tui::MenuBranch* tui_menu_effects;
	s_tui::MenuBranch* tui_menu_scripts;
	s_tui::MenuBranch* tui_menu_administration;

	// 1. Location
	s_tui::DecimalItem* tui_location_latitude;
	s_tui::DecimalItem* tui_location_longitude;
	s_tui::IntegerItemLogstep* tui_location_altitude;
	s_tui::MultiSet2Item<std::string>* tui_location_planet;
	s_tui::DecimalItem* tui_location_heading;

	// 2. Time & Date
	s_tui::TimeZoneitem* tui_time_settmz;
	s_tui::TimeItem* tui_time_skytime;
	s_tui::TimeItem* tui_time_presetskytime;
	s_tui::MultiSet2Item<std::string>* tui_time_startuptime;
	s_tui::MultiSetItem<std::string>* tui_time_displayformat;
	s_tui::MultiSetItem<std::string>* tui_time_dateformat;
	s_tui::MultiSet2Item<std::string>* tui_time_day_key;

	// 3. General
	s_tui::MultiSetItem<std::string>* tui_general_landscape;
	s_tui::MultiSet2Item<std::string>* tui_general_sky_culture;
	s_tui::MultiSetItem<std::string>* tui_general_sky_locale;

	// 4. Stars
	s_tui::BooleanItem* tui_stars_show;
	s_tui::DecimalItem* tui_star_labelmaxmag;
	s_tui::DecimalItem* tui_stars_twinkle;
	s_tui::DecimalItem* tui_star_magscale;
	s_tui::DecimalItem* tui_star_limitingmag;
	//  Colors
	s_tui::VectorItem* tui_colors_const_line_color;
	s_tui::VectorItem* tui_colors_const_label_color;
	s_tui::VectorItem* tui_colors_cardinal_color;
	s_tui::VectorItem* tui_colors_const_boundary_color;
	s_tui::VectorItem* tui_colors_planet_names_color;
	s_tui::VectorItem* tui_colors_planet_orbits_color;
	s_tui::VectorItem* tui_colors_satellite_orbits_color;
	s_tui::VectorItem* tui_colors_object_trails_color;
	s_tui::VectorItem* tui_colors_meridian_color;
	s_tui::VectorItem* tui_colors_azimuthal_color;
	s_tui::VectorItem* tui_colors_equatorial_color;
	s_tui::VectorItem* tui_colors_equator_color;
	s_tui::VectorItem* tui_colors_ecliptic_color;
	s_tui::VectorItem* tui_colors_nebula_label_color;
	s_tui::VectorItem* tui_colors_nebula_circle_color;
	s_tui::VectorItem* tui_colors_precession_circle_color;
	s_tui::VectorItem* tui_colors_circumpolar_circle_color;
	s_tui::DecimalItem* tui_colors_const_art_intensity;
	s_tui::VectorItem* tui_colors_const_art_color;

	// 5. Effects
	s_tui::BooleanItem* tui_effect_pointobj;
	s_tui::DecimalItem* tui_effect_object_scale;
	s_tui::DecimalItem* tui_effect_star_size_limit;
	s_tui::DecimalItem* tui_effect_planet_size_limit;
	s_tui::DecimalItem* tui_effect_zoom_duration;

	s_tui::DecimalItem* tui_effect_milkyway_intensity;
	s_tui::DecimalItem* tui_effect_nebulae_label_magnitude;
	s_tui::BooleanItem* tui_effect_manual_zoom;
	s_tui::DecimalItem* tui_effect_cursor_timeout;
	s_tui::DecimalItem* tui_effect_light_pollution;
	s_tui::BooleanItem* tui_effect_light_travel;
	s_tui::DecimalItem* tui_effect_view_offset;
	s_tui::BooleanItem* tui_effect_antialias;
	s_tui::DecimalItem* tui_effect_line_width;

	// 6. Scripts
	s_tui::ListItem<std::string>* tui_scripts_basis;
	s_tui::ListItem<std::string>* tui_scripts_deepsky;
	s_tui::ListItem<std::string>* tui_scripts_navigation;
	s_tui::ListItem<std::string>* tui_scripts_planets;
	s_tui::ListItem<std::string>* tui_scripts_shows;
	s_tui::ListItem<std::string>* tui_scripts_custom;


	// 7. Administration
	s_tui::ActionConfirmItem* tui_admin_loaddefault;
	s_tui::ActionConfirmItem* tui_admin_savedefault;
	s_tui::ActionConfirmItem* tui_admin_shutdown;
	s_tui::MultiSetItem<std::string>* tui_admin_setlocale;

	s_tui::Display* tui_admin_info; // display about info
	s_tui::Display* tui_admin_user; // display about user

	// Tui Callbacks
	void tuiCb1(void);									// Update all the core flags and params from the tui
	void tuiCbSettimezone(void);						// Set time zone
	void tuiCbSetTimeDisplayFormat(void);				// Set 12/24h format
	void tuiCbAdminLoadDefault(void);				// Load default configuration
	void tuiCbAdminSaveDefault(void);				// Save default configuration
	void tuiCbAdminSetLocale(void);					// Set locale for UI (not sky)
	void tuiCbAdminShutdown(void);					// Shut down

	void tuiCbViewportRelated();   					// Change heading or view offset
	void tuiCbTuiGeneralChangeLandscape(void);		// Select a new landscape skin
	void tuiCbTuiGeneralChangeSkyCulture(void);  	// select new sky culture
	void tuiCbTuiGeneralChangeSkyLocale(void);  	// select new sky locale

	void tuiCbDayKey(); 								// update day key mode

	void tuiCbScriptsDeepsky();             			// changed deepsky script selection
	void tuiCbScriptsNavigation();             		// changed navigation script selection
	void tuiCbScriptsPlanets();             			// changed planets script selection
	void tuiCbScriptsShows();             			// changed shows script selection
	void tuiCbScriptsCustom();             			// changed custom script selection
	void tuiCbScriptsBasis();             			// changed basis script selection


	void tuiCbEffectsMilkywayIntensity();       	// change milky way intensity
	void tuiCbEffectsNebulaeLabelMagnitude();  	// change nebula label limiting magnitude
	void tuiCbSetlocation();        					// change observer position
	void tuiCbStars();        						// change star parameters
	void tuiCbEffects();        						// change effect parameters
	void tuiCbSkyTime();       						// change effect parameters
	void tuiCbChangeColor();        					// change colors
	void tuiCbLocationChangePlanet();  				// change planet of observer

	Vec3f text_ui; 										// Color info text
	Vec3f text_tui_root; 								// Color menu text

	double MouseTimeLeft;  // for cursor timeout (seconds)

	enum MODIFIER key_Modifier;

	double KeyTimeLeft;  // for shift timeout (seconds)
	int SpecialNumber;       // For direct script access

	//Joystick
	JoypadController * joypadController = nullptr;

	void handleJoyAddStick();
	void handleJoyRemoveStick();

	bool fullscreen; //! wether if we are in fullscreen or not
	int posMouseX, posMouseY;
	bool handleKeyOnVideo = false;   //! permet de basculer le mode clavier lors des videos
};

#endif  //_UI_H
