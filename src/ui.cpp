/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which handles the User Interface

#include <iostream>
#include <iomanip>
#include <algorithm>
#include "app.hpp"
#include "ui.hpp"
#include "log.hpp"
#include "script_mgr.hpp"
#include "app_command_interface.hpp"
#include "screenFader.hpp"
#include "joypad_controller.hpp"
#include "call_system.hpp"
#include "core.hpp"
#include "core_io.hpp"
#include "media.hpp"

using namespace s_gui;
using namespace std;

static const double CoeffMultAltitude = 0.02;
static const double DURATION_COMMAND = 0.1;


////////////////////////////////////////////////////////////////////////////////
//								CLASS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

string default_landscape = "";
string current_landscape = "";
UI::UI(Core * _core, CoreIO* _coreIO, App * _app, Media* _media) :
	tuiFont(nullptr),
	FlagShowTuiMenu(0),
	tui_root(nullptr),
	key_Modifier(NONE) ,
	KeyTimeLeft(0)
{
	if (!_core) {
		Log.write("UI.CPP CRITICAL : In stel_ui constructor, invalid core.",cLog::LOG_TYPE::L_ERROR);
		exit(-1);
	}
	core = _core;
	coreIO = _coreIO;
	media = _media;
	app = _app;
	is_dragging = false;

	text_ui = Vec3f(0.5,1.0,0.5);
	text_tui_root = Vec3f(.5,.7,1.0);
}

/**********************************************************************************/
UI::~UI()
{
	delete tuiFont;
	tuiFont = nullptr;
	if (tui_root) delete tui_root;
	tui_root=nullptr;
}

////////////////////////////////////////////////////////////////////////////////
void UI::init(const InitParser& conf)
{
	// Ui section
	FlagShowFps			= conf.getBoolean("gui:flag_show_fps");
	FlagShowLatLon      = conf.getBoolean("gui:flag_show_latlon");
	FlagShowFov			= conf.getBoolean("gui:flag_show_fov");
	FlagNumberPrint		= conf.getInt("gui:flag_number_print");

	FontSizeGeneral		= conf.getDouble ("font","font_general_size");
	FontNameGeneral     = AppSettings::Instance()->getUserFontDir() +conf.getStr("font", "font_general_name");
	MouseCursorTimeout  = conf.getDouble("gui","mouse_cursor_timeout");
	PosDateTime			= conf.getInt("gui","datetime_display_position");
	PosObjectInfo		= conf.getInt("gui","object_info_display_position");	
	PosMenuM			= conf.getInt("gui","menu_display_position");
	FlagShowPlanetname	= conf.getBoolean("gui:flag_show_planetname");
	// Text ui section
	FontSizeMenuTui     = conf.getDouble ("font","font_menutui_size");
	FlagEnableTuiMenu = conf.getBoolean("tui:flag_enable_tui_menu");
	FlagShowGravityUi = conf.getBoolean("tui:flag_show_gravity_ui");
	FlagShowTuiDateTime = conf.getBoolean("tui:flag_show_tui_datetime");
	FlagShowTuiShortObjInfo = conf.getBoolean("tui:flag_show_tui_short_obj_info");
	FlagMouseUsableInScript = conf.getBoolean("gui","flag_mouse_usable_in_script");
	FontNameMenu = AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_menu_name");

	// set up mouse cursor timeout
	MouseTimeLeft = MouseCursorTimeout*1000;

	default_landscape = coreIO->observatoryGetLandscapeName();
	current_landscape = coreIO->observatoryGetLandscapeName();
	Log.write("Landscape : "+default_landscape ,cLog::LOG_TYPE::L_INFO);
}

/*******************************************************************/
void UI::draw(void)
{
	if (FlagShowGravityUi) drawGravityUi();
	if (getFlagShowTuiMenu()) drawTui();
}

/*******************************************************************************/
int UI::handleMove(int x, int y)
{
	// Do not allow use of mouse while script is playing otherwise script can get confused
	if (app->scriptMgr->isPlaying() && ! FlagMouseUsableInScript) return 0;

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	if (is_dragging) {
		if ((has_dragged || sqrtf((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y))>4.)) {
			has_dragged = true;
			core->setFlagTracking(false);
			core->dragView(previous_x, previous_y, x, y);
			previous_x = x;
			previous_y = y;
			return 1;
		}
	}
	return 0;
}

/*******************************************************************************/
int UI::handleClic(Uint16 x, Uint16 y, S_GUI_VALUE button, S_GUI_VALUE state)
{
	// Do not allow use of mouse while script is playing otherwise script can get confused
	if (app->scriptMgr->isPlaying() && ! FlagMouseUsableInScript) return 0;

	// Make sure object pointer is turned on (script may have turned off)
	core->setFlagSelectedObjectPointer(true);

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	switch (button) {
		case S_GUI_MOUSE_RIGHT :
			break;
		case S_GUI_MOUSE_LEFT :
			if (state==S_GUI_PRESSED) {
				is_dragging = true;
				has_dragged = false;
				previous_x = x;
				previous_y = y;
			} else {
				is_dragging = false;
			}
			break;
		case S_GUI_MOUSE_MIDDLE :
			break;
		case S_GUI_MOUSE_WHEELUP :
			core->zoomTo(core->getAimFov()-app->MouseZoom*core->getAimFov()/60., 0.2);
			return 1;
		case S_GUI_MOUSE_WHEELDOWN :
			core->zoomTo(core->getAimFov()+app->MouseZoom*core->getAimFov()/60., 0.2);
			return 1;
		default:
			break;
	}

	// Manage the event for the main window
	{
		// Deselect the selected object
		if (button==S_GUI_MOUSE_RIGHT && state==S_GUI_RELEASED) {
			app->commander->executeCommand("select");
			return 1;
		}
		if (button==S_GUI_MOUSE_MIDDLE && state==S_GUI_RELEASED) {
			if (core->getFlagHasSelected()) {
				core->gotoSelectedObject();
				core->setFlagTracking(true);
			}
		}
		if (button==S_GUI_MOUSE_LEFT && state==S_GUI_RELEASED && !has_dragged) {
			// CTRL + left clic = right clic for 1 button mouse
			if (SDL_GetModState() & KMOD_CTRL) {
				app->commander->executeCommand("select");
				return 1;
			}
			// Try to select object at that position
			core->findAndSelect(x, y);
		}
	}
	return 0;
}

// Update changing values
void UI::updateTimeouts(int delta_time)
{
//	 handle mouse cursor timeout
	if (MouseCursorTimeout > 0) {
		if (MouseTimeLeft > delta_time) MouseTimeLeft -= delta_time;
		else {
			// hide cursor
			MouseTimeLeft = 0;
			SDL_ShowCursor(0);
		}
	}

//	 handle key_Modifier cursor timeout
	if (key_Modifier != NONE) {
		if (KeyTimeLeft > delta_time) KeyTimeLeft -= delta_time;
		else {
			key_Modifier = NONE;
		}
	}
}


void UI::setFlagShowTuiMenu(const bool flag)
{

	if (flag && !FlagShowTuiMenu) {
		tuiUpdateIndependentWidgets();
	}

	FlagShowTuiMenu = flag;
}

void UI::handleJoyHat(SDL_JoyHatEvent E)
{
	// Mouvement d'un chapeau Nous devons donc utiliser le champ jhat
	//printf("Mouvement du chapeau %d du joystick %d\n",E.jhat.hat,E.jhat.which);
	switch (E.value) {
		case SDL_HAT_CENTERED:
			core->turnUp(0);
			core->turnRight(0);
			core->turnDown(0);
			core->turnLeft(0);
			break;
		case SDL_HAT_DOWN :
			core->turnDown(1);
			break;
		case SDL_HAT_LEFT :
			core->turnLeft(1);
			break;
		case SDL_HAT_RIGHT :
			core->turnRight(1);
			break;
		case SDL_HAT_UP :
			core->turnUp(1);
			break;
		default:
			break;
	}
}

void UI::moveMouseAlt(double x)
{
	int distZ=sqrt((posMouseX-app->getDisplayWidth()/2)*(posMouseX-app->getDisplayWidth()/2)+(posMouseY-app->getDisplayHeight()/2)*(posMouseY-app->getDisplayHeight()/2));
	//cout << x << " " << app->getDisplayHeight() << " " << distZ << endl;
	if (distZ<1)
		distZ=1;
		
	if(abs(x) < 1){
		x = x>0 ? 1 : -1;
	}
		
	posMouseX = posMouseX+x*(posMouseY-app->getDisplayHeight()/2)/distZ;
	posMouseY = posMouseY-x*(posMouseX-app->getDisplayWidth()/2)/distZ;
	
	app->warpMouseInWindow(posMouseX, posMouseY);
	
	handleMove(posMouseX , posMouseY);
}

void UI::moveMouseAz(double x)
{
	int distZ=sqrt((posMouseX-app->getDisplayWidth()/2)*(posMouseX-app->getDisplayWidth()/2)+(posMouseY-app->getDisplayHeight()/2)*(posMouseY-app->getDisplayHeight()/2));
        //cout << x << " " << app->getDisplayWidth() << " " << distZ << endl;
	if (distZ<1)
		distZ=1;
	posMouseX = posMouseX+x*(posMouseX-app->getDisplayWidth()/2-1)/distZ; //-1 put to avoid no movement if already centered
	posMouseY = posMouseY+x*(posMouseY-app->getDisplayHeight()/2-1)/distZ; //-1 put to avoid no movement if already centered
	app->warpMouseInWindow(posMouseX, posMouseY);
	handleMove(posMouseX , posMouseY);
}

void UI::moveLat(double x)
{
	core->moveRelLonObserver(x,DURATION_COMMAND);
}

void UI::moveLon(double x)
{
	if (core->getSelectedPlanetEnglishName()!="")
		core->moveRelLatObserver(-x,DURATION_COMMAND);
	else
		core->moveRelLatObserver(x,DURATION_COMMAND);
}

void UI::lowerHeight(double x)
{
	double latimem = coreIO->observatoryGetAltitude();
	latimem = -latimem*(CoeffMultAltitude*x);
	//~ stringstream oss;
	//~ oss << "moveto altitude " << latimem << " duration 1";
	//~ app->commander->executeCommand(oss.str());
	core->moveRelAltObserver(latimem, DURATION_COMMAND);
}

void UI::raiseHeight(double x)
{
	double latimem = coreIO->observatoryGetAltitude();
	latimem = latimem*(CoeffMultAltitude*x);
	//~ stringstream oss;
	//~ oss << "moveto altitude " << latimem << " duration 1";
	//~ app->commander->executeCommand(oss.str());
	core->moveRelAltObserver(latimem, DURATION_COMMAND);
}

void UI::handleJoyAddStick()
{
	joypadController = new JoypadController(this);
	joypadController->init("joypad.ini");
}

void UI::handleJoyRemoveStick()
{
	delete joypadController;
	joypadController = nullptr;
}

void UI::raiseHeight()
{
	core->raiseHeight(1);
}

void UI::stopZoomIn()
{
	core->zoomIn(0);
}

void UI::stopZoomOut()
{
	core->zoomOut(0);
}

void UI::stopLowerHeight()
{
	core->lowerHeight(0);
}

void UI::stopRaiseHeight()
{
	core->raiseHeight(0);
}

void UI::stopSpeedDecrease()
{
	app->speedDecrease(0);
}

void UI::stopSpeedIncrease()
{
	app->speedIncrease(0);
}

void UI::centerMouse()
{
	app->warpMouseInWindow( app->getDisplayWidth()/2 , app->getDisplayHeight()/2 );
}

void UI::stopTurnLeft()
{
	core->turnLeft(0);
}

void UI::stopTurnRight()
{
	core->turnRight(0);
}

void UI::stopTurnUp()
{
	core->turnUp(0);
}

void UI::stopTurnDown()
{
	core->turnDown(0);
}

void UI::zoomIn()
{
	core->zoomIn(1);
}

void UI::zoomOut()
{
	core->zoomOut(1);
}

void UI::lowerHeight()
{
	lowerHeight(1);
}

void UI::speedDecrease()
{
	if(app->scriptMgr->isPlaying())
		app->scriptMgr->slowerScript();
	else
		app->speedDecrease(1);
}

void UI::speedIncrease()
{
	if(app->scriptMgr->isPlaying())
		app->scriptMgr->slowerScript();
	else
		app->speedIncrease(1);
}

void UI::turnLeft()
{
	core->turnLeft(1);
}

void UI::turnRight()
{
	core->turnRight(1);
}

void UI::turnUp()
{
	core->turnUp(1);
}

void UI::turnDown()
{
	core->turnDown(1);
}

void UI::leftClick()
{
	handleClic(posMouseX, posMouseY, S_GUI_MOUSE_LEFT, S_GUI_PRESSED);
	handleClic(posMouseX, posMouseY, S_GUI_MOUSE_LEFT, S_GUI_RELEASED);
}

void UI::rightClick()
{
	handleClic(posMouseX, posMouseY, S_GUI_MOUSE_RIGHT, S_GUI_PRESSED);
	handleClic(posMouseX, posMouseY, S_GUI_MOUSE_RIGHT, S_GUI_RELEASED);
}

void UI::executeCommand(string command)
{
	app->commander->executeCommand(command);
}

void UI::pauseScript()
{
	if ( app->scriptMgr->isPlaying() ) {
		app->commander->executeCommand( "script action pause");
		app->time_multiplier = 1;
	} else
		app->commander->executeCommand( "timerate action pause");;
}

void UI::handleInputs(SDL_Event E)
{

	enum s_gui::S_GUI_VALUE bt;

	switch (E.type) {		// And Processing It

		case SDL_QUIT:
			app->setAlive(false);
			break;

		case SDL_JOYDEVICEADDED:
			handleJoyAddStick();
			break;

		case SDL_JOYDEVICEREMOVED:
			handleJoyRemoveStick();
			break;

		case SDL_JOYAXISMOTION :
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYHATMOTION :
			joypadController->handle(E);
			break;

		case SDL_WINDOWEVENT:
			switch(E.window.event) {
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					app->setVisible(true); // Activity's been gained
					break;

				case SDL_WINDOWEVENT_FOCUS_LOST:
					app->setVisible(true); //switch true because L.R. need it
					break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			// Convert the name from GLU to my GUI
			switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=s_gui::S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=s_gui::S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=s_gui::S_GUI_MOUSE_MIDDLE;
					break;
				default :
					bt=s_gui::S_GUI_MOUSE_LEFT;
			}
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_PRESSED);
			break;

		case SDL_MOUSEWHEEL:
			if(E.wheel.y>0)
				bt=s_gui::S_GUI_MOUSE_WHEELUP;
			else
				bt=s_gui::S_GUI_MOUSE_WHEELDOWN;
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_PRESSED);
			break;

		case SDL_MOUSEBUTTONUP:
			// Convert the name from GLU to my GUI
			switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=s_gui::S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=s_gui::S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=s_gui::S_GUI_MOUSE_MIDDLE;
					break;
				default :
					bt=s_gui::S_GUI_MOUSE_LEFT;
			}
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_RELEASED);
			break;

		case SDL_MOUSEMOTION:
			posMouseX=E.motion.x;
			posMouseY=E.motion.y;
			handleMove(E.motion.x,E.motion.y);
			break;

		case SDL_KEYDOWN:
			// Rescue escape in case of lock : CTRL + ESC forces brutal quit
			if (E.key.keysym.scancode==SDL_SCANCODE_ESCAPE && (SDL_GetModState() & KMOD_CTRL)) {
				app->quit();
				break;
			}
			// Send the event to the gui and stop if it has been intercepted
			handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_PRESSED);
			break;

		case SDL_KEYUP:
			handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_RELEASED);
			break;
	}
}

void UI::handleDeal()
{
	if(joypadController!=nullptr)
		joypadController->handleDeal();
}

/*******************************************************************************/
// LSS HANDLE KEYS
// TODO replace this with flexible keymapping feature
// odd extension to prevent compilation from makefile but inclusion in make dist
int flag_compass = 0;
int flag_triangle = 0;
int flag_creu = 0;
int flag_f9 = 0;

bool antipodes = false;

int UI::handleKeysOnVideo(SDL_Scancode key, Uint16 mod, Uint16 unicode, S_GUI_VALUE state)
{

	int retVal = 0;

	switch(key) {
		case SDL_SCANCODE_SPACE :
			media->playerPause();
			//~ return 0;
			break;
		case SDL_SCANCODE_ESCAPE :
			handleKeyOnVideo = false;
			media->playerStop();
			break;
		case SDL_SCANCODE_A :
			handleKeyOnVideo = false;
			media->playerStop();
			break;
		case SDL_SCANCODE_G :
			handleKeyOnVideo = false;
			media->playerStop();
			break;
		case SDL_SCANCODE_J :
			media->playerInvertflow();
			break;
		case SDL_SCANCODE_K :
			if ( app->scriptMgr->isPlaying() ) {
				app->commander->executeCommand( "script action resume");
				app->time_multiplier = 1;
			} else
				media->playerPause();
			break;
		case SDL_SCANCODE_LEFT :
			media->playerJump(-10.0);
			break;
		case SDL_SCANCODE_RIGHT :
			media->playerJump(10.0);
			break;
		case SDL_SCANCODE_DOWN :
			// JUMP BEGINNING
			//media->playerRestart(); //bug : crash software
			break;
		case SDL_SCANCODE_TAB :
			handleKeyOnVideo = false;
			break;
		default:
			retVal = 1;
			break;
	}

	return retVal;

}

int UI::handlKkeysOnTui(SDL_Scancode key, Uint16 mod, Uint16 unicode, S_GUI_VALUE state)
{
	s_tui::S_TUI_VALUE tuiv;
	if (state == s_gui::S_GUI_PRESSED)
		tuiv = s_tui::S_TUI_PRESSED;
	else
		tuiv = s_tui::S_TUI_RELEASED;

	if (state==s_gui::S_GUI_PRESSED && key==SDL_SCANCODE_SEMICOLON) {
		// leave tui menu
		FlagShowTuiMenu = false;

		// If selected a script in tui, run that now
		if (app->getSelectedScript()!="")
			app->scriptMgr->playScript(app->getSelectedScriptDirectory()+app->getSelectedScript()); //, SelectedScriptDirectory);
		// clear out now
		app->setSelectedScriptDirectory("");
		app->setSelectedScript("");
		return 1;
	}

	return handleKeysTui(key, tuiv);
}

int UI::handleKeyPressed(SDL_Scancode key, Uint16 mod, Uint16 unicode, S_GUI_VALUE state)
{

	int retVal = 0;

	std::ostringstream oss;
	double latimem;

	//TODO fixe ceci
	string IDIR = AppSettings::Instance()->getScriptDir();
	string SDIR = AppSettings::Instance()->getScriptDir();
	string ADIR = AppSettings::Instance()->getAudioDir();
	string VDIR = AppSettings::Instance()->getVideoDir();
	if (core->getFlagNav()) {
		SDIR = SDIR + "navigation/";
		ADIR = ADIR + "navigation/";
		VDIR = VDIR + "navigation/";
	}

	if (key == SDL_SCANCODE_A && (mod & COMPATIBLE_KMOD_CTRL)) {
		app->quit();
	}

	if(!app->scriptMgr->isPlaying())
		app->time_multiplier = 1;  // if no script in progress always real time

	switch (key) {

		case SDL_SCANCODE_LEFT :
			core->turnLeft(1);
			break;

		case SDL_SCANCODE_RIGHT :
			core->turnRight(1);
			break;

		case SDL_SCANCODE_UP :
			if (mod & KMOD_CTRL)
				core->zoomIn(1);
			else
				core->turnUp(1);
			break;

		case SDL_SCANCODE_DOWN :
			if (mod & KMOD_CTRL)
				core->zoomOut(1);
			else
				core->turnDown(1);
			break;

		case SDL_SCANCODE_PAGEUP :
			core->zoomIn(1);
			break;

		case SDL_SCANCODE_PAGEDOWN :
			core->zoomOut(1);
			break;

		case SDL_SCANCODE_RALT :
			key_Modifier=ALT;
			KeyTimeLeft = 120*1000;
			break;

		case SDL_SCANCODE_GRAVE :
			if (key_Modifier != SUPER) {
				key_Modifier=SUPER;
				KeyTimeLeft = 3*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RGUI :
		case SDL_SCANCODE_LGUI :
			if (key_Modifier != KWIN) {
				key_Modifier=KWIN;
				KeyTimeLeft = 120*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RSHIFT :
		case SDL_SCANCODE_LSHIFT :
			if (key_Modifier != SHIFT) {
				key_Modifier=SHIFT;
				KeyTimeLeft = 120*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RCTRL :
		case SDL_SCANCODE_LCTRL :
			key_Modifier=CTRL;
			KeyTimeLeft = 120*1000; // 2 min
			break;

		case SDL_SCANCODE_BACKSLASH :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(IDIR+"internal/clear_mess.sts");
					break;
				case SUPER:
					app->commander->executeCommand( "flag nebula_names toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					coreIO->nebulaSetFlagNames(true);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_SEMICOLON :
			switch(key_Modifier) {
				case NONE:
					if (FlagEnableTuiMenu) setFlagShowTuiMenu(true);
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_COMMA :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"02.mp3 action play");
					break;
				case SUPER:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"06.mp3 action play");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"10.mp3 action play");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_PERIOD :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"03.mp3 action play");
					break;
				case SUPER:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"07.mp3 action play");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"11.mp3 action play");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_SLASH :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"04.mp3 action play");
					break;
				case SUPER:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"08.mp3 action play");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"12.mp3 action play");
					break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_A:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag cardinal_points toggle");
					break;
				case SUPER:
					if (flag_creu != 1) {
						app->scriptMgr->playScript(IDIR+"internal/windrose.sts");
					} else core->setLandscape(current_landscape);
					flag_creu = (flag_creu+1)%2;
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					app->quit();
					break;
				case SHIFT:
					if (core->getFlagNav()) {
						if (!app->scriptMgr->isPlaying()) {
							string s;
							stringstream out;
							out << flag_compass+1;
							s = out.str();
							if (flag_compass == 4) {
								core->setLandscape(current_landscape);
							} else app->scriptMgr->playScript(SDIR+"fscripts/windrose/0"+s+".sts");
							flag_compass = (flag_compass+1)%5;
							flag_triangle = 0;
							flag_f9 = 0;
						}
					}
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_B:
			switch(key_Modifier) {
				case NONE:
					if (core->getMeteorsRate()==10) app->commander->executeCommand("meteors zhr 10000");
					else app->commander->executeCommand("meteors zhr 10");
					break;
				case SUPER:
					if (core->getMeteorsRate()<=10000) app->commander->executeCommand("meteors zhr 150000");
					else app->commander->executeCommand("meteors zhr 10");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_C:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag equatorial_grid toggle");
					key_Modifier= NONE;
					break;
				case SUPER:
					app->commander->executeCommand( "flag circumpolar_circle toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand( "flag aries_line toggle");
					key_Modifier= NONE;
					break;
				case SHIFT:
					if (core->getFlagNav()) app->commander->executeCommand( "flag greenwich_line toggle");
					break;
				case CTRL :
					app->commander->executeCommand( "flag vernal_points toggle");
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_D:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag equator_line toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag tropic_lines toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					if (app->get_m_OutputFrameNumber() == 0) {
						app->fixScriptFps();
						app->set_m_OutputFrameNumber(1);
					} else {
						app->set_m_OutputFrameNumber(0);
						app->fixMaxFps();
					}
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag satellites_orbits toggle");
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/equator_poles.sts");
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_E:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag constellation_art toggle");
					break;
				case SUPER:
					core->selectZodiac();
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag constellation_pick toggle");
					break;
				case KWIN:
					break;
				case CTRL:
					app->scriptMgr->playScript(IDIR+"internal/sky_culture3.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_F  :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag moon_scaled toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag sun_scaled toggle");
					app->scriptMgr->playScript(IDIR+"internal/big_planets.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->scriptMgr->playScript(IDIR+"internal/bodies-asteroids-501ex.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(IDIR+"internal/bodies-kuiper.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					app->commander->executeCommand( "flag oort toggle");
					app->scriptMgr->playScript(IDIR+"internal/comet.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_G :
			if ( app->scriptMgr->isPlaying() )
				app->commander->executeCommand( "script action end");
			else
				app->commander->executeCommand( "timerate rate 0");
			break;

		case SDL_SCANCODE_H :
			switch(key_Modifier) {
				case NONE:
					if ( app->scriptMgr->isPlaying() ) {
						app->commander->executeCommand( "script action pause");
						app->time_multiplier = 1;
					} else
						app->commander->executeCommand( "timerate action pause");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/personal.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_I :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "date sidereal -1");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->setJDayRelative(0,-1);  //fast 1 month backward
					break;
				case KWIN:
					app->screenFader->downGrade(0.05);
					break;
				case CTRL :
					app->commander->executeCommand( "date relative -1");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_J :
			switch(key_Modifier) {
				case NONE:
					if(app->scriptMgr->isPlaying())
						app->scriptMgr->slowerScript();
					else
						app->commander->executeCommand( "timerate action decrement");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/proper_demotion.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "date sun rise");
					app->scriptMgr->playScript(IDIR+"internal/date_shift_minus.sts");
					break;
				case KWIN:
					latimem = coreIO->observatoryGetAltitude();
					latimem = latimem-1000000;
					oss << "moveto altitude " << latimem << " duration 1";
					app->commander->executeCommand(oss.str());
					break;
				case CTRL:
					core-> setJDayRelative(-20,0); //fast 20 years backward
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_K :
			switch(key_Modifier) {
				case NONE:
					if ( app->scriptMgr->isPlaying() ) {
						app->commander->executeCommand( "script action resume");
						app->time_multiplier = 1;
					} else
						app->commander->executeCommand( "timerate rate 1");
					break;
				case SUPER: {
					double jd = coreIO->getJDay();
					ln_date current_date,temps;
					SpaceDate::JulianToDate(jd,&current_date);
					temps=current_date;
					jd = coreIO->getJDay();
					SpaceDate::JulianToDate(jd,&current_date);
					current_date.hours=22;
					current_date.minutes=0;
					current_date.seconds=0;
					coreIO->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years,current_date.months,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
					key_Modifier= NONE;
				}
				break;
				case CTRL :
					app->commander->executeCommand( "date sun midnight");
					break;
				case SHIFT:
					app->commander->executeCommand( "date sun meridian");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_L :
			switch(key_Modifier) {
				case NONE:
					if(app->scriptMgr->isPlaying())
						app->scriptMgr->fasterScript();
					else
						app->commander->executeCommand( "timerate action increment");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/proper_motion.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					latimem = coreIO->observatoryGetAltitude();
					latimem = latimem+1000000;
					oss << "moveto altitude " << latimem << " duration 1";
					app->commander->executeCommand(oss.str());
					break;
				case SHIFT:
					app->commander->executeCommand( "date sun set");
					app->scriptMgr->playScript(IDIR+"internal/date_shift_plus.sts");
					break;
				case CTRL :
					core-> setJDayRelative(20, 0); //fast 20 years forward
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_M :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"01.mp3 action play");
					break;
				case SUPER:
					app->commander->executeCommand( "external_viewer filename "+ADIR+"05.mp3 action play");
					key_Modifier= NONE;
					break;
				case KWIN:
					Log.write(CallSystem::getRamInfo(),cLog::LOG_TYPE::L_INFO, cLog::LOG_FILE::SCRIPT);
					break;
				case CTRL :
					app->commander->executeCommand( "external_viewer filename "+ADIR+"09.mp3 action play");
				default:
					break;
			}
			break;

		case SDL_SCANCODE_N :
			switch(key_Modifier) {
				case NONE:
					// if ( system("killall vlc 2>/dev/null")) printf("error stop vlc music\n");
					CallSystem::killAllPidFromVLC();
					media->audioMusicHalt();
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/chut.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(IDIR+"internal/navigation.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_O :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "date sidereal 1");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					app->screenFader->upGrade(0.05);
					break;
				case SHIFT:
					core->setJDayRelative(0,1);  //fast 1 month forward
					break;
				case CTRL :
					app->commander->executeCommand( "date relative 1");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_P :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "date sidereal 7");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->setJDayRelative(1,0); //fast 1 year forward
					break;
				case KWIN:
					break;
				case CTRL :
					app->commander->executeCommand( "date relative 7");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_Q :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag constellation_drawing toggle");
					break;
				case SUPER:
					app->commander->executeCommand("set sky_culture western-asterisms");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag star_lines toggle");
					break;
				case KWIN:
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/sky_culture1.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_R:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag constellation_boundaries toggle");
					break;
				case SUPER:
					app->commander->executeCommand("flag zodiac toggle");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag aries_line toggle");
					break;
				case KWIN:
					if (app->scriptMgr->isRecording()) {
						app->commander->executeCommand( "script action cancelrecord");
					} else {
						app->commander->executeCommand( "script action record");
					}
					key_Modifier= NONE;
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/sky_culture4.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_S:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag ecliptic_line toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag precession_circle toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->takeScreenshot();
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag planets_orbits toggle");
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/ecliptic_poles.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_T :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("flag object_trails toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag body_trace toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand( "body_trace action clear");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "body_trace pen toggle");
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/trace.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_U :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "date sidereal -7");
					key_Modifier= NONE;
					break;
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->setJDayRelative(-1,0); //fast 1 year backward
					break;
				case KWIN:
					break;
				case CTRL :
					app->commander->executeCommand( "date relative -7");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_V :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag show_tui_datetime toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag show_tui_short_obj_info toggle");
					key_Modifier= NONE;
					break;
				case CTRL :
					app->commander->executeCommand( "flag show_latlon toggle");
					break;
				case KWIN:
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_W :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag constellation_names toggle");
					break;
				case SUPER:
					app->commander->executeCommand("flag zenith_line toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand("flag zodiacal_light toggle");
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/sky_culture2.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_X :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag meridian_line toggle");
					break;
				case SUPER:
					app->commander->executeCommand("flag azimuthal_grid toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					if (core->getFlagNav()) app->commander->executeCommand( "flag vertical_line toggle");
					key_Modifier= NONE;
					break;
				case SHIFT :
					app->commander->executeCommand("flag planets_axis toggle");
					break;
				case CTRL :
					app->scriptMgr->playScript(IDIR+"internal/mire.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_Y :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("flag analemma_line toggle");
					break;
				case SUPER:
					app->commander->executeCommand("flag galactic_center toggle");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( "flag analemma toggle");
					break;
				case KWIN: {
					string planet = core->getSelectedPlanetEnglishName();
					if(planet!="") app->commander->executeCommand(string("body_trace target " + planet+ string(" pen on")));
					break;
					}
				case CTRL :
					app->commander->executeCommand("select planet home_planet pointer off");
					app->commander->executeCommand("flag track_object on");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_Z :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag atmosphere toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "flag landscape toggle");
					key_Modifier= NONE;
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/panorama2.sts");
					current_landscape = coreIO->observatoryGetLandscapeName();
					break;
				case KWIN:
					break;
				case CTRL :
					app->commander->executeCommand( "flag clouds toggle");
					app->scriptMgr->playScript(SDIR+"fscripts/panorama4.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_0 :
			switch(key_Modifier) {
				case NONE:
					core->moveRelLatObserver(45,7000);  //latitude , duration
					break;
				case SUPER:
					app->commander->executeCommand("moveto lat 90 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K0.sts");
					break;
				case CTRL :
					core->moveRelLatObserver(30,5000);  //latitude , duration
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_1 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("flag star_names toggle");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/white_room.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W13.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/13.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K1.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_2 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("flag planet_names toggle");
					break;
				case SUPER:
					app->commander->executeCommand("flag planet_orbits toggle");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W14.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/14.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K2.sts");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_3 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag nebula_hints toggle");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/deepsky_drawings.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W15.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/15.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K3.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_4 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag fog toggle");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/orange_fog.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W16.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/16.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K4.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_5 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag planets toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "body action clear");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W17.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/17.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K5.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_6 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag stars toggle");
					break;
				case SUPER:
					app->commander->executeCommand("deselect");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W18.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->scriptMgr->playScript(SDIR+"fscripts/18.sts");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K6.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_7 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag milky_way toggle");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/milkyway.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand( "flag color_inverse toggle");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->commander->executeCommand( "flag stars_trace toggle");
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K7.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_8 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("flag nebulae toggle");
					break;
				case SUPER:
					app->commander->executeCommand( "nebula action clear");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand( "flag dso_pictograms toggle");
					key_Modifier= NONE;
					break;
				case CTRL:
					app->commander->executeCommand( "flag nebula_names toggle");
					break;
				case SHIFT:
					app->scriptMgr->playScript(SDIR+"fscripts/K8.sts");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_9 :
			switch(key_Modifier) {
				case NONE:
					core->moveRelLatObserver(-45,7000);  //latitude , duration
					break;
				case SUPER:
					app->commander->executeCommand("moveto lat -90 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(IDIR+"internal/takeoff.sts");
					key_Modifier= NONE;
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/K9.sts");
					break;
				case CTRL :
					core->moveRelLatObserver(-30,5000);  //latitude , duration
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_MINUS :
			switch(key_Modifier) {
				case NONE:
					if (core->getFlagManualAutoZoom()) app->commander->executeCommand( "zoom auto out manual 1");
					else app->commander->executeCommand( "zoom auto initial");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/zoom.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					break;
				case CTRL :
					app->commander->executeCommand( "zoom auto out");
					app->commander->executeCommand( "zoom fov 60 duration 5");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_EQUALS :
			switch(key_Modifier) {
				case NONE:
					if(core->getFlagManualAutoZoom()) app->commander->executeCommand( "zoom auto in manual 1");
					else app->commander->executeCommand( "zoom auto in");
					break;
				case SUPER:
					app->commander->executeCommand( "zoom auto out");
					app->commander->executeCommand( "zoom fov 10 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					break;
				case CTRL :
					app->commander->executeCommand( "zoom auto in");
					app->commander->executeCommand( "zoom fov 1 duration 5");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_SPACE :
			switch(key_Modifier) {
				case NONE:
					if ( app->scriptMgr->isPlaying() ) {
						app->commander->executeCommand( "script action pause");
						app->time_multiplier = 1;
					} else
						app->commander->executeCommand( "timerate action pause");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL :
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_NONUSBACKSLASH :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "flag landscape toggle");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/panorama1.sts");
					current_landscape = coreIO->observatoryGetLandscapeName();
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					app->scriptMgr->playScript(SDIR+"fscripts/panorama3.sts");
					current_landscape = coreIO->observatoryGetLandscapeName();
					break;
				case CTRL :
					core->setLandscape(default_landscape);
					current_landscape = coreIO->observatoryGetLandscapeName();
					flag_compass = 0;
					flag_triangle = 0;
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_LEFTBRACKET :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( string("flag track_object toggle"));
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/takeoff.sts");
					key_Modifier= NONE;
					break;
				case KWIN: {
					app->scriptMgr->playScript(IDIR+"internal/fly_to_selected.sts");
/*
					  string planetsel = core->getSelectedPlanetEnglishName();
					  if(planetsel!="") {
						string planalt="100000";
						if ((planetsel!="Jupiter") && (planetsel!="Saturn") && (planetsel!="Uranus") && (planetsel!="Neptune")) planalt="10000";
						app->commander->executeCommand("flag track_object on");
						app->commander->executeCommand("flag atmosphere off");
						app->commander->executeCommand("flag landscape off");
						app->commander->executeCommand("wait duration 3");
						app->commander->executeCommand("moveto altitude 20000000 duration 3");
						app->commander->executeCommand("wait duration 3");
						app->commander->executeCommand( string("select planet ") + planetsel + string(" pointer off"));
						app->commander->executeCommand("wait duration 0");
						app->commander->executeCommand("camera action transition_to target point name Space");
						app->commander->executeCommand("wait duration 0");
						app->commander->executeCommand( string("camera action move_to target body body_name ") + planetsel + string(" duration 3 altitude ") + planalt );
						app->commander->executeCommand("wait duration 3");
						app->commander->executeCommand( string("camera action transition_to target body name ") + planetsel);
						app->commander->executeCommand("wait duration 3");
					  } 
*/					} break;
				case SHIFT:
					app->commander->executeCommand("position action save");
					break;
				case CTRL : {
					string planet = core->getSelectedPlanetEnglishName();
					if(planet!="") app->commander->executeCommand( string("set home_planet ") + planet + string(" duration 10"));
				}
				break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_RIGHTBRACKET :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("date load preset");
					break;
				case SUPER:
					app->init();
					app->scriptMgr->playScript(IDIR+"internal/initial.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("date load current");
					app->commander->executeCommand("date sun set");
					app->commander->executeCommand("date relative 0.07");
					break;
				case KWIN:
					break;
				case CTRL: {
					double jd = coreIO->getJDay();
					ln_date current_date,temps;
					SpaceDate::JulianToDate(jd,&current_date);
					temps=current_date;
					app->commander->executeCommand("date load preset");
					jd = coreIO->getJDay();
					SpaceDate::JulianToDate(jd,&current_date);
					current_date.hours=temps.hours;
					current_date.minutes=temps.minutes;
					current_date.seconds=temps.seconds;
					coreIO->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years,current_date.months,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
				}
				break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_APOSTROPHE :
			switch(key_Modifier) {
				case NONE:
					core->setFlagLockSkyPosition(!core->getFlagLockSkyPosition());
					break;
				case SHIFT:
					app->commander->executeCommand("position action save");
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/clear_mess.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->commander->executeCommand( string("flag track_object toggle"));
					key_Modifier= NONE;
					break;
				case CTRL: {
					string planet = core->getSelectedPlanetEnglishName();
					if(planet!="")
						app->commander->executeCommand( string("set home_planet ") + planet + string(" duration 10"));
				}
				break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_ESCAPE:
			break;

		case SDL_SCANCODE_INSERT:
			app->commander->executeCommand("position action save");
			break;
		case SDL_SCANCODE_DELETE:
			app->commander->executeCommand("position action load");
			break;
		case SDL_SCANCODE_HOME:
			app->init();
			app->scriptMgr->playScript(IDIR+"internal/initial.sts");
			break;
		case SDL_SCANCODE_END:
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(IDIR+"internal/initial_night.sts");
					break;
				case SHIFT:
					break;
				case SUPER:
					app->scriptMgr->playScript(IDIR+"internal/initial_dawn.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;
		case SDL_SCANCODE_RETURN:
			app->commander->executeCommand("deselect");
			break;
		case SDL_SCANCODE_KP_1 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M01.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M11.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveHeadingRelative(-0.2);
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S01.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					core->moveHeadingRelative(-1);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_2 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M02.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M12.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S02.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveRelLatObserver(-0.5,DURATION_COMMAND);  //latitude , duration
					break;
				case CTRL:
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(0.,-1.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_3 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M03.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M13.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					latimem = coreIO->observatoryGetAltitude();
					latimem = latimem/5;
					oss << "moveto altitude " << latimem << " duration 1";
					app->commander->executeCommand(oss.str());
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S03.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(0.,0.0,-1.0);
					break;

				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_4 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M04.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M14.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveRelLonObserver(-0.5,DURATION_COMMAND);  //longitude , duration
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S04.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(-1.,0.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_5 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M05.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M15.sts");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S05.sts");
					key_Modifier= NONE;
					break;
				case SHIFT: {
					string newplanet = core->getSelectedPlanetEnglishName();
					if (newplanet!="") app->commander->executeCommand( string("set home_planet ") + newplanet);
				}
				break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_6 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M06.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M16.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveRelLonObserver(0.5,DURATION_COMMAND);  //longitude , duration
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S06.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(1.,0.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_7 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M07.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M17.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveHeadingRelative(0.2);
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S07.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					core->moveHeadingRelative(1);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_8 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M08.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M18.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					core->moveRelLatObserver(0.5,DURATION_COMMAND);  //latitude , duration
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S08.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(0.,1.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_9 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M09.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M19.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					latimem = coreIO->observatoryGetAltitude();
					latimem = latimem*5;
					oss << "moveto altitude " << latimem << " duration 1";
					app->commander->executeCommand(oss.str());
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S09.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					app->commander->executeCommand( "flag galactic_grid toggle");
					break;
				case ALT:
					coreIO->cameraMoveRelativeXYZ(0.,0.0,1.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_0 :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M00.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M10.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand( string("flag track_object toggle"));
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S10.sts");
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_TAB :
			switch(key_Modifier) {
				case NONE:
					handleKeyOnVideo = true;
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_PERIOD :
			switch(key_Modifier) {
				case NONE:
					app->scriptMgr->playScript(SDIR+"fscripts/M20.sts");
					break;
				case SUPER:
					app->scriptMgr->playScript(SDIR+"fscripts/M21.sts");
					key_Modifier= NONE;
					break;
				case SHIFT: {
					string coreplanet = core->getSelectedPlanetEnglishName();
					if (coreplanet!="") {
						app->commander->executeCommand( string("set home_planet ") + coreplanet);
						app->commander->executeCommand( string("select planet ')+coreplanet+string(' pointer off"));
						app->commander->executeCommand( string("wait duration 0.01"));
						app->commander->executeCommand( string("body name _ action drop"));
						app->commander->executeCommand( string("body action load name _ parent Sun radius 1 oblateness 0.0 albedo 0.0 lighting false halo false color 1.0,1.0,1.0 rot_periode 1000000000 tex_map none.png coord_func ')+coreplanet+string('_special"));
						app->commander->executeCommand( string("set home_planet _ duration 0"));
						app->commander->executeCommand( string("select planet _ pointer off"));
						app->commander->executeCommand( string("flag track_object on"));
					}
				}
				break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S11.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_PLUS :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( string("set home_planet Earth"));
					app->commander->executeCommand( string("moveto lat default lon default alt default"));
					app->commander->executeCommand( string("zoom auto out manual 1"));
					break;
				case SUPER:
					latimem = coreIO->observatoryGetLatitude();
					latimem = -latimem;
					oss << "moveto lat " << latimem;
					latimem = coreIO->observatoryGetLongitude();
					latimem = 180+latimem;
					oss << " lon " << latimem << " duration 0";
					app->commander->executeCommand(oss.str());
					if (!antipodes) {
						app->scriptMgr->playScript(IDIR+"internal/antipodes.sts");
					} else {
						core->setLandscape(current_landscape);
					}
					antipodes = !antipodes;
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S12.sts");
					key_Modifier= NONE;
					break;
				case CTRL :
					latimem = coreIO->observatoryGetLatitude();
					latimem = -latimem;
					oss << "moveto lat " << latimem << " duration 0";
					app->commander->executeCommand(oss.str());
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_MINUS :
			switch(key_Modifier) {
				case NONE:
					SDL_ShowCursor(1);
					app->warpMouseInWindow( app->getDisplayWidth()/2 , app->getDisplayHeight()/2 );
					break;
				case SUPER:
					SDL_ShowCursor(0);
					app->warpMouseInWindow( app->getDisplayWidth()/2 , app->getDisplayHeight() );
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S13.sts");
					key_Modifier= NONE;
					break;
				case CTRL :

					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_MULTIPLY :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("audio volume increment");
					break;
				case SUPER:
					app->commander->executeCommand("audio volume 100");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S14.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("set ambient_light increment");
					break;
				case CTRL:
					app->commander->executeCommand("define a 1");
					app->commander->executeCommand("script action resume");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_DIVIDE :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand("audio volume decrement");
					break;
				case SUPER:
					app->commander->executeCommand("audio volume 0");
					key_Modifier= NONE;
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/S15.sts");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("set ambient_light decrement");
					break;
				case CTRL :
					app->commander->executeCommand("define a 0");
					app->commander->executeCommand("script action resume");
					break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_F1:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"01.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"13.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"25.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/01.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W01.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_F2 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"02.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"14.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"26.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/02.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W02.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F3 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"03.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"15.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"27.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/03.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W03.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F4:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"04.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"16.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"28.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/04.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W04.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F5:
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"05.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"17.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"29.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/05.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W05.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F6 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"06.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"18.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"30.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/06.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W06.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F7 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"07.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"19.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"31.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/07.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W07.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F8 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"08.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"20.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"32.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/08.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W08.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F9 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"09.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"21.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"33.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/09.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W09.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F10 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"10.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"22.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"34.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/10.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W10.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F11 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"11.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"23.avi action play");
					key_Modifier= NONE;
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/11.sts");
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"35.avi action play");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W11.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F12 :
			switch(key_Modifier) {
				case NONE:
					app->commander->executeCommand( "media type VIEWPORT videoname "+VDIR+"12.avi action play");
					break;
				case SUPER:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"24.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					app->commander->executeCommand("media type VIEWPORT videoname "+VDIR+"36.avi action play");
					break;
				case CTRL :
					app->scriptMgr->playScript(SDIR+"fscripts/12.sts");
					break;
				case KWIN:
					app->scriptMgr->playScript(SDIR+"fscripts/W12.sts");
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

			retVal=1;

		default:
			break;
	}  // end S_GUI_PRESSED

	return retVal;
}

int UI::handleKeysReleased(SDL_Scancode key, Uint16 mod, Uint16 unicode, S_GUI_VALUE state)
{

	int retVal = 0;

	switch (key) {

		case SDL_SCANCODE_LEFT :
			core->turnLeft(0);
			break;

		case SDL_SCANCODE_RIGHT :
			core->turnRight(0);
			break;

		case SDL_SCANCODE_UP :
			if (mod & KMOD_CTRL)
				core->zoomIn(0);
			else
				core->turnUp(0);
			break;

		case SDL_SCANCODE_DOWN :
			if(mod & KMOD_CTRL)
				core->zoomOut(0);
			else
				core->turnDown(0);
			break;

		case SDL_SCANCODE_PAGEUP :
			core->zoomIn(0);
			break;

		case SDL_SCANCODE_PAGEDOWN :
			core->zoomOut(0);
			break;

		case SDL_SCANCODE_RALT:
			KeyTimeLeft=0;
			key_Modifier = NONE;
			break;

		case SDL_SCANCODE_RCTRL :
		//~ break;
		case SDL_SCANCODE_LCTRL :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;
		case SDL_SCANCODE_RSHIFT :
		//~ break;
		case SDL_SCANCODE_LSHIFT :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;
		case SDL_SCANCODE_RGUI :
		//~ break;
		case SDL_SCANCODE_LGUI :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;
		default:
			retVal = 1;
			break;
	}

	return retVal;
}

int UI::handleKeys(SDL_Scancode key, Uint16 mod, Uint16 unicode, S_GUI_VALUE state)
{

	if (FlagShowTuiMenu) {
		return handlKkeysOnTui(key, mod, unicode, state);
	}

	//gestion touches video
	if (handleKeyOnVideo && state==S_GUI_PRESSED) { //videoStatus
		return handleKeysOnVideo(key,mod,unicode,state);
	}

	if (state==S_GUI_PRESSED) {
		return handleKeyPressed(key, mod, unicode, state);
	}

	if (state==S_GUI_RELEASED) {
		switch (key) {

			case SDL_SCANCODE_LEFT :
				core->turnLeft(0);
				break;

			case SDL_SCANCODE_RIGHT :
				core->turnRight(0);
				break;

			case SDL_SCANCODE_UP :
				if (mod & KMOD_CTRL)
					core->zoomIn(0);
				else
					core->turnUp(0);
				break;

			case SDL_SCANCODE_DOWN :
				if(mod & KMOD_CTRL)
					core->zoomOut(0);
				else
					core->turnDown(0);
				break;

			case SDL_SCANCODE_PAGEUP :
				core->zoomIn(0);
				break;

			case SDL_SCANCODE_PAGEDOWN :
				core->zoomOut(0);
				break;

		case SDL_SCANCODE_RALT:
			KeyTimeLeft=0;
			key_Modifier = NONE;
			break;

			case SDL_SCANCODE_RCTRL :
			//~ break;
			case SDL_SCANCODE_LCTRL :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;
			case SDL_SCANCODE_RSHIFT :
			//~ break;
			case SDL_SCANCODE_LSHIFT :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;
			case SDL_SCANCODE_RGUI :
			//~ break;
			case SDL_SCANCODE_LGUI :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;
			default:
				break;
		}
	}

	return 1;
}

