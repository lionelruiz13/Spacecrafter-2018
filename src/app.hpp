/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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

#ifndef APP_H
#define APP_H


#include <SDL2/SDL_thread.h>
#include <queue>

#include "app_settings.hpp"
#include "shader.hpp"
#include "stateGL.hpp"
#include "clock.hpp"
#include "space_date.hpp"
#include "spacecrafter.hpp"

// mac seems to use KMOD_META instead of KMOD_CTRL
#ifdef MACOSX
#define COMPATIBLE_KMOD_CTRL KMOD_META
#else
#define COMPATIBLE_KMOD_CTRL KMOD_CTRL
#endif

// Predeclaration of some classes
class AppCommandInterface;
class ScriptMgr;
class SDLFacade;
class s_font;
class s_texture;
class Media;
class UI;
class Core;
class CoreIO;
class SaveScreen;
class Mkfifo;
class ServerSocket;
class ScreenFader;
class EventManager;
class EventHandler;

/**
@author Fabien Chereau
*/
class App {
	friend class UI;
	friend class AppCommandInterface;
	friend class EvenHandler;

public:
	App( SDLFacade* const );
	~App();

	//! Initialize application and core
	void init(void);

	//! Update all object according to the delta time
	void update(int delta_time);

	//! Draw all
	void draw(int delta_time);

	//! Start the main loop until the end of the execution
	void startMainLoop(void) {
		start_main_loop();
	}

	//! get the time_multiplier for script warning do not confuse this with sky time rate
	int getTimeMultiplier() {
		return time_multiplier;
	}

	void setTimeMultiplier( int mult ) {
		time_multiplier = mult;
	}

	//! Update speed
	void speedIncrease(int saction) {
		saction ? deltaSpeed=1 : deltaSpeed=0;
	}
	void speedDecrease(int saction) {
		saction ? deltaSpeed=-1 : deltaSpeed=0;
	}

	//! Quit the application
	void quit(void);

	//! @brief Set the application language
	//! This applies to GUI, console messages etc..
	//! This function has no permanent effect on the global locale
	//! @param newAppLocaleName The name of the language (e.g fr) to use for GUI, TUI and console messages etc..
	void setAppLanguage(const std::string& newAppLangName);

	std::string getAppLanguage();

	// for use by TUI
	void saveCurrentConfig(const std::string& confFile);

	double getMouseCursorTimeout();

	//! Required because stelcore doesn't have access to the script manager anymore!
	//! Record a command if script recording is on
	void recordCommand(std::string commandline);

	//! ordonne de prendre un screenshot.
	void takeScreenshot() {
		flagScreenshot=true;
	}

	void fixScriptFps() {
		internalClock->fixScriptFps();
	}

	void fixMaxFps() {
		internalClock->fixMaxFps();
	}

	//! Use by masterput, poor but he does his job
	void masterput();

	//! return tcpPosition
	void tcpGetPosition();

	//! Set flag for activating or not TCP
	void setEnableTcp(bool b) {
		enable_tcp=b;
	}

	//! Get flag for activating or not TCP
	bool getEnableTcp(void) const {
		return enable_tcp;
	}

	//! Set flag for activating or not MASTERPUT
	void setFlagMasterput(bool b) {
		flagMasterput=b;
	}

	//! Get flag for activating or not MASTERPUT
	bool getFlagMasterput(void) const {
		return flagMasterput;
	}

	//! get frame m_OutputFrameNumber for screencapture
	unsigned int get_m_OutputFrameNumber() {
		return m_OutputFrameNumber;
	}

	//! set frame m_OutputFrameNumber for screencapture
	void set_m_OutputFrameNumber(unsigned int a) {
		m_OutputFrameNumber=a;
	}

	//! Get display width
	int getDisplayWidth(void) const {
		return width;
	}

	//! Get display height
	int getDisplayHeight(void) const {
		return height;
	}

	void initSplash();		//! dessine le splash au démarrage

	void initDebugGL(bool _debug);

	//! place la souris aux coordonnées fractionnaire (x,y) de l'écran
	void warpMouseInWindow(float x, float y);

	SpaceDate* getSpaceDate() {
		return spaceDate;
	}

	void setAlive(bool alive) {
		isAlive = alive;
	}

	void setVisible(bool visible) {
		AppVisible = visible;
	}

	std::string getSelectedScript() {
		return SelectedScript;
	}

	std::string getSelectedScriptDirectory() {
		return SelectedScriptDirectory;
	}

	void setSelectedScript(std::string filename) {
		SelectedScript = filename;
	}

	void setSelectedScriptDirectory(std::string path) {
		SelectedScriptDirectory = path;
	}

private:
	//! création des shaders
	void createShader();
	//! suppression des shaders
	void deleteShader();
	//! dessine la première couche du tracé opengl sur le logiciel
	void drawFirstLayer();
	//! rempli en noir l'extérieur du dôme
	void drawViewportShape();
	//! dessine le rendu final du logiciel en inversant les couleurs
	void drawColorInverse();
	//! Réalise une capture d'image de la frame en cours.
	void writeScreenshot(std::string filename);
	//! Run the main program loop
	void start_main_loop(void); //cyp
	//! Return the next sequential screenshot filename to use
	std::string getNextScreenshotFilename();
	//! run any incoming command from shared memory interface
	void updateSharedInData( void );
	//! mise à jour de l'accélération du temps
	void updateDeltaSpeed();

	bool isAlive; 					//indique si l'application doit s'arrêter ou pas
	bool AppVisible;				// say if your App Is Visible or not
	bool videoStatus; 				// indique si une video est en cours de visionnage
	bool initialized;				//! has the init method been called yet?
	bool flagColorInverse;			//! indique si les couleurs de l'écran sont inversées
	bool flagEnableDragMouse;  		//!indique si le clic gauche de la souris permet de modifier le viewport
	bool flagScreenshot;   //! indique si on doit prendre une capture d'écran.

	// Script related
	std::string SelectedScript;  //! script filename (without directory) selected in a UI to run when exit UI
	std::string SelectedScriptDirectory;  //! script directory for same

	double PresetSkyTime;
	std::string StartupTimeMode;				//! Can be "now" or "preset"
	std::string DayKeyMode;						//! calendar or sidereal
	int MouseZoom;
	unsigned int deltaTime; 			//! représente la durée théorique d'une frame
	int FlagTimePause;
	double temp_time_velocity;			//! Used to store time speed while in pause
	int time_multiplier;  //! used for adjusting delta_time for script speeds

	//communication with other processus
	bool enable_tcp;
	bool enable_mkfifo;
	bool flagMasterput;
	bool debugGL; // say if openGL is in debug mode

	// Main elements of the stel_app
	AppSettings* settings = nullptr; 				//! base pour les constantes du logiciel
	AppCommandInterface * commander = nullptr;	//! interface to perform all UI and scripting actions
	ScriptMgr * scriptMgr = nullptr;				//! manage playing and recording scripts
	Media* media = nullptr;						//!< media manager
	UI * ui = nullptr;							//! The main User Interface
	ScreenFader *screenFader = nullptr;			//! gestion des fondus

	//! The assicated Core instance
	Core* core = nullptr;
	CoreIO* coreIO = nullptr;
	SDLFacade* mSdl = nullptr;
	SaveScreen* saveScreen = nullptr;
	#if LINUX
	Mkfifo* mkfifo = nullptr;
	#endif
	ServerSocket * tcp = nullptr;
	Clock* internalClock = nullptr;				//! getion fine du frameRate

	SpaceDate * spaceDate = nullptr;			    //Handles dates and conversions
	EventManager *eventManager = nullptr;
	EventHandler *eventHandler = nullptr;
	unsigned int m_OutputFrameNumber; //! Number used for writing out video frames to files
	int deltaSpeed;			//!< variation de l'accélération du temps

	Uint16 width, height;  //! Contient la résolution w et h de la fenetre SDL
	SDL_Event	E;

	DataGL layer;
	shaderProgram* shaderViewportShape = nullptr;
	shaderProgram* shaderColorInverse = nullptr;
	DataGL dataGL;
};

#endif
