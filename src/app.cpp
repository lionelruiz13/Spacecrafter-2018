/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "app.hpp"
#include "utility.hpp"
#include "callbacks.hpp"
#include <exception>
#include "save_screen.hpp"
#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "log.hpp"
#include <unistd.h>
#include "s_texture.hpp"
#include <sstream>
#include <iomanip>

#include "app_command_interface.hpp"
#include "script_mgr.hpp"
#include "media.hpp"
#include "sdl_facade.hpp"
#include "perf_debug.hpp"
#include "glutils.hpp"
#include "mkfifo.hpp"
#include "io.hpp"
#include "ui.hpp"
#include "core.hpp"
#include "core_io.hpp"
#include "screenFader.hpp"
#include "call_system.hpp"
#include "event_manager.hpp"
#include "event_handler.hpp"
#include "spacecrafter.hpp"

EventManager* EventManager::instance = nullptr;

using namespace std;

App::App( SDLFacade* const sdl ) :
	initialized(false), FlagTimePause(0),
	m_OutputFrameNumber(0)
{
	mSdl = sdl;
	flagScreenshot=false;
	flagEnableDragMouse = false;
	flagMasterput =false;
	// window=mSdl->getWindow();
	mSdl->getResolution( &width, &height );

	SelectedScript = SelectedScriptDirectory = "";
	settings = AppSettings::Instance();

	media = new Media( width, height );
	saveScreen = new SaveScreen(min(width, height));
	screenFader =  new ScreenFader();

	core = new Core(settings, screenFader, width, height, media, mBoost::callback<void, string>(this, &App::recordCommand));
	coreIO = new CoreIO(core);

	screenFader->initShader();  //CYP ? pourquoi utiliser un init shader qui est indépendant du reste du programme ?

	ui = new UI(core, coreIO, this, media);
	commander = new AppCommandInterface(core, coreIO, this, media);
	scriptMgr = new ScriptMgr(commander, settings->getUserDir(), media);

	EventManager::Init();
	eventManager = EventManager::getInstance();
	eventHandler = new EventHandler(coreIO, eventManager);

	time_multiplier = 1;
	cout.precision(16);
	enable_tcp= false;
	enable_mkfifo= false;
	flagColorInverse= false;
	//~ flagStarsTrace = false;
	videoStatus = false;
	deltaSpeed=0;
	#if LINUX // special mkfifo
	mkfifo= new Mkfifo();
	#endif

	internalClock = new Clock();
	//~ statePosition = IN_SOLARSYSTEM;

	spaceDate = new SpaceDate();
}

App::~App()
{
	EventManager::End();
	// deleteVboVao();
	deleteShader();
	if (enable_tcp)
		delete tcp;
	#if LINUX // special mkfifo
	delete mkfifo;
	#endif
	delete ui;
	delete scriptMgr;
	delete media;
	delete commander;
	delete coreIO;
	delete core;
	delete saveScreen;
	delete internalClock;
	delete screenFader;
	delete spaceDate;
}

void App::quit(void)
{
	static SDL_Event Q;						// Send a SDL_QUIT event
	Q.type = SDL_QUIT;						// To the SDL event queue
	
	// std::string action = "killall vlc &";
	// if (!system(action.c_str())) Log.write("No action needed for VLC",cLog::LOG_TYPE::L_ERROR);
	CallSystem::killAllPidFromVLC();

	if (SDL_PushEvent(&Q) == -1) {			// Try to send the event
		stringstream oss;
		oss << "SDL_QUIT event can't be pushed: " << SDL_GetError() ;
		Log.write(oss.str(),  cLog::LOG_TYPE::L_ERROR);
		exit(-1);
	}
}

void App::warpMouseInWindow(float x, float y) {
	mSdl->warpMouseInWindow( x , y);
}

double App::getMouseCursorTimeout() {
	return ui->getMouseCursorTimeout();
}

std::string App::getAppLanguage() {
	return Translator::globalTranslator.getLocaleName();
}

void App::initSplash()
{
	shaderProgram *shaderSplash;
	DataGL Splash;

	int tmp=min(width, height);
	glViewport((width-tmp)/2, (height-tmp)/2, tmp, tmp);

	s_texture* tex_splash = new s_texture(/*1, */settings->getUserDir()+"textures/splash/spacecrafter.png" , TEX_LOAD_TYPE_PNG_ALPHA);

	shaderSplash = new shaderProgram();
	shaderSplash->init( "splash.vert", "splash.frag");

	glGenVertexArrays(1,&Splash.vao);
	glBindVertexArray(Splash.vao);

	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(0);

	float dataTex[]= {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
	glGenBuffers(1,&Splash.tex);
	glBindBuffer(GL_ARRAY_BUFFER,Splash.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, dataTex, GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	float dataPos[]= {-1.0,-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	glGenBuffers(1,&Splash.pos);
	glBindBuffer(GL_ARRAY_BUFFER,Splash.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, dataPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	shaderSplash->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_splash->getID());
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	shaderSplash->unuse();

	if (shaderSplash) delete shaderSplash;
	if (tex_splash) delete tex_splash;

	mSdl->glSwapWindow();	// And swap the buffers
}

//~ void App::init_debug(bool _debug)
//~ {
	//~ Log.setDebug(_debug);
//~ }

void App::initDebugGL(bool _debug)
{
	//initialisation debug GL.
	#ifdef LINUX
	if (_debug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLUtils::debugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		debugGL = true;
		Log.write("OpenGL debug enable");
	} else {
		Log.write("OpenGL debug disabled");
		debugGL = false;
		glDisable(GL_DEBUG_OUTPUT);
	}
	#else
	Log.write("OpenGL debug disabled on WIN32");
	debugGL = false;
	glDisable(GL_DEBUG_OUTPUT);
	#endif
}

//! Load configuration from disk
void App::init(void)
{
	if (!initialized)
		Translator::initSystemLanguage();

	// Initialize video device and other sdl parameters
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	// Clear screen, this fixes a strange artifact at loading time in the upper top corner.
	glClear(GL_COLOR_BUFFER_BIT);

	internalClock->setMaxFps(conf.getDouble ("video","maximum_fps"));
	internalClock->setScriptFps(conf.getDouble ("video","script_fps"));

	string appLocaleName = conf.getStr("localization", "app_locale"); //, "system");
	spaceDate->setTimeFormat(spaceDate->stringToSTimeFormat(conf.getStr("localization:time_display_format")));
	spaceDate->setDateFormat(spaceDate->stringToSDateFormat(conf.getStr("localization:date_display_format")));
	setAppLanguage(appLocaleName);

	// time_zone used to be in init_location section of config, so use that as fallback when reading config - Rob
	string tzstr = conf.getStr("localization", "time_zone");
	#if LINUX
	if (tzstr == "system_default") {
		spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
		// Set the program global intern timezones variables from the system locale
		tzset();
	} else {
	#endif
		if (tzstr == "gmt+x") {
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT);
		} else {
			// We have a custom time zone name
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
			spaceDate->setCustomTzName(tzstr);
		}
		#if LINUX
	}
		#endif
	mSdl->getResolution( &width, &height );

	core->setViewportMaskDisk(width/2, height/2, width, height, height/2);
	core->init(conf, width, height);

	// Navigation section
	PresetSkyTime 		= conf.getDouble ("navigation","preset_sky_time"); //,2451545.);
	StartupTimeMode 	= conf.getStr("navigation:startup_time_mode");	// Can be "now" or "preset"
	flagEnableDragMouse	= conf.getBoolean("navigation","flag_enable_drag_mouse");
	MouseZoom			= conf.getInt("navigation","mouse_zoom"); //,30);

	DayKeyMode = conf.getStr("navigation","day_key_mode"); //,"calendar");  // calendar or sidereal
	Log.write("Read daykeymode as <" + DayKeyMode + ">", cLog::LOG_TYPE::L_INFO);

	if (StartupTimeMode=="preset" || StartupTimeMode=="Preset")
		coreIO->setJDay(PresetSkyTime - spaceDate->getGMTShift(PresetSkyTime) * JD_HOUR);
	else core->setTimeNow();

	// initialisation of the User Interface
	ui->init(conf);
	ui->localizeTui( );
	if (!initialized) ui->initTui(); // don't reinit tui since probably called from there
	else ui->localizeTui();  // update translations/fonts as needed

	//set all color
	core->setColorScheme(settings->getConfigFile(), "color");
	core->setFontScheme();

	if (! initialized) {
		// initShader();
		// initShaderParams();
		this->createShader();
		bool mplayerEnable =conf.getBoolean("io:enable_mplayer");
		std::string mplayerFileName =conf.getStr("io:mplayer_name");
		std::string mplayerMkfifoName = conf.getStr("io:mplayer_mkfifo_name");
		media->externalInit(mplayerFileName, mplayerMkfifoName,mplayerEnable);
		media->createViewPort();
		media->createVR360(scriptMgr);
		media->createImageShader();

		flagMasterput=conf.getBoolean("main:flag_masterput");
		enable_tcp=conf.getBoolean("io","enable_tcp");
		enable_mkfifo=conf.getBoolean("io","enable_mkfifo");

		stringstream oss;
		Log.write(oss.str(), cLog::LOG_TYPE::L_INFO);


		if (enable_tcp) {
			int port = conf.getInt("io:tcp_port_in");
			int buffer_in_size=conf.getInt("io:tcp_buffer_in_size");
			Log.write("buffer TCP taille " + Utility::intToString(buffer_in_size));
			tcp = new ServerSocket(port, 16, buffer_in_size, IO_DEBUG_INFO, IO_DEBUG_ALL);
			tcp->open();
			core->tcpConfigure(tcp);
		}
		#if LINUX // special mkfifo
		if (enable_mkfifo) {
			string mkfifo_file_in = conf.getStr("io:mkfifo_file_in");
			int buffer_in_size=conf.getInt("io:mkfifo_buffer_in_size");
			Log.write("buffer MKFIFO taille "+ Utility::intToString(buffer_in_size));
			mkfifo->init(mkfifo_file_in, buffer_in_size);
		}
		#endif
		Log.write(CallSystem::getRamInfo());
		Log.mark();
		Log.write("End of loading SC");
		Log.write("End of loading SC",cLog::LOG_TYPE::L_INFO,cLog::LOG_FILE::SCRIPT);
		Log.mark();
	}

	// play startup script //, if available
	// if (!initialized && scriptMgr) 
	scriptMgr->playStartupScript();
	initialized = true;
}

void App::updateSharedInData( void )
{
	if (enable_mkfifo) {
		string out;
		#if LINUX // special mkfifo
		if (mkfifo->update(out)) {
			Log.write("dans app.cpp j'ai obtenu du mkfifo: " + out);
			commander->executeCommand(out);
		}
		#endif
	}
	if (enable_tcp) {
		string out;
		do {
			out = tcp->getInput();
			if (!out.empty()) {
				Log.write("dans app.cpp j'ai obtenu du tcp : " + out);
				commander->executeCommand(out);
				//tcp->setOutput("Feedback : " + out);
			}
		} while (!out.empty());
	}
}


void App::update(int delta_time)
{
	//~ AutoPerfDebug apd(&pd, "App::update$"); //Debug
	internalClock->addFrame();
	internalClock->addCalculatedTime(delta_time);

	if (internalClock->afterOneSecond()) {
		media->externalUpdate(delta_time);
	}

	// depile les remarques d'openGL
	if (debugGL)
		GLUtils::checkForOpenGLError(__FILE__,__LINE__);

	eventHandler->handleEvents();

	// change time rate if needed to fast forward scripts
	delta_time *= time_multiplier;

	// run command from a running script
	scriptMgr->update(delta_time);
	if (!scriptMgr->isPaused() || !scriptMgr->isFaster() )	media->audioUpdate(delta_time);

	// run any incoming command from shared memory interface
	updateSharedInData();

	ui->updateTimeouts(delta_time);
	ui->tuiUpdateWidgets();

	if (!scriptMgr->isPaused()) media->imageUpdate(delta_time);

	media->playerUpdate();
	media->faderUpdate(delta_time);

	core->updateMode();
	//~ switch(statePosition) {
		//~ case IN_SOLARSYSTEM :
			//~ core->updateInSolarSystem(delta_time);
			//~ break;

		//~ case IN_GALAXY:
			//~ core->updateInGalaxy(delta_time);
			//~ break;

		//~ case IN_UNIVERSE:
			//~ core->updateInUniverse(delta_time);
			//~ break;

		//~ default:
			//~ break;
	//~ }
	core->update(delta_time);

	//gestion variation de l'accélération
	if (deltaSpeed!=0) {
		printf("valeur de deltaSpeed : %i\n",deltaSpeed);
		updateDeltaSpeed();
	}
}

void App::createShader()
{
	shaderViewportShape =  nullptr;
	shaderViewportShape= new shaderProgram();
	shaderViewportShape->init( "viewportShape.vert", "viewportShape.frag");
	shaderViewportShape->setUniformLocation("radius");
	shaderViewportShape->setUniformLocation("decalage_x");

	shaderColorInverse =  nullptr;
	shaderColorInverse = new shaderProgram();
	shaderColorInverse->init( "colorInverse.vert", "colorInverse.frag");

	float viewportShapePoints[8];

	viewportShapePoints[0]= -1.0; // point en haut a gauche
	viewportShapePoints[1]= 1.0;

	viewportShapePoints[2]= 1.0;  // point en haut a droite
	viewportShapePoints[3]= 1.0;

	viewportShapePoints[4]= -1.0; // point en bas à gauche
	viewportShapePoints[5]= -1.0;

	viewportShapePoints[6]= 1.0;  // point en bas à droite
	viewportShapePoints[7]= -1.0;

	glGenBuffers(1,&dataGL.pos);
	glBindBuffer(GL_ARRAY_BUFFER,dataGL.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, viewportShapePoints,GL_STATIC_DRAW);

	glGenVertexArrays(1,&dataGL.vao);
	glBindVertexArray(dataGL.vao);

	glBindBuffer (GL_ARRAY_BUFFER, dataGL.pos);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
}

void App::deleteShader()
{
	if (shaderViewportShape) 
		delete shaderViewportShape;
	if (shaderColorInverse)
		delete shaderColorInverse;

	glDeleteBuffers(1, &layer.pos);
	glDeleteVertexArrays(1, &layer.vao);

	glDeleteBuffers(1, &dataGL.pos);
	glDeleteVertexArrays(1, &dataGL.vao);
}

//! dessine la première couche du tracé opengl sur le logiciel
void App::drawFirstLayer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

//! Fill with black around the circle
void App::drawViewportShape()
{
	StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	shaderViewportShape->use();
	shaderViewportShape->setUniform("radius" , min(width, height)/2);
	shaderViewportShape->setUniform("decalage_x" , (width -min(width, height))/2);

	glBindVertexArray(dataGL.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	shaderViewportShape->unuse();
}

void App::drawColorInverse()
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	shaderColorInverse->use();

	glBindVertexArray(dataGL.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	shaderColorInverse->unuse();
}

//! Main drawinf function called at each frame
void App::draw(int delta_time)
{
	//draw the first layer que si le mode starsTrace n'est pas actif
	//~ if (!flagStarsTrace)
		drawFirstLayer();

	//~ switch(statePosition) {
		//~ case IN_SOLARSYSTEM :
			//~ // Render all the main objects of the application
			//~ core->drawInSolarSystem(delta_time);
			//~ break;

		//~ case IN_GALAXY:
			//~ core->drawInGalaxy(delta_time);
			//~ break;

		//~ case IN_UNIVERSE:
			//~ core->drawInUniverse(delta_time);
			//~ break;

		//~ default:
			//~ break;
	//~ }
	core->draw(delta_time);

	// Draw the Graphical ui and the Text ui
	ui->draw();
	//inversion des couleurs pour un ciel blanc
	if (flagColorInverse)
		drawColorInverse();

	//draw video frame to classical viewport
	media->drawViewPort();

	// Fill with black around the circle
	drawViewportShape();

	screenFader->draw();
}

//! @brief Set the application locale. This apply to GUI, console messages etc..
void App::setAppLanguage(const std::string& newAppLocaleName)
{
	// Update the translator with new locale name
	Translator::globalTranslator = Translator(PACKAGE, settings->getLocaleDir(), newAppLocaleName);
	Log.write("Application locale is " + Translator::globalTranslator.getLocaleName(), cLog::LOG_TYPE::L_INFO);
	if (initialized)
		ui->localizeTui();
}

/*
//! Handle key press and release
int App::handleKeys(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{
	int retVal = 0;

	s_tui::S_TUI_VALUE tuiv;
	if (state == s_gui::S_GUI_PRESSED)
		tuiv = s_tui::S_TUI_PRESSED;
	else
		tuiv = s_tui::S_TUI_RELEASED;

	if (ui->FlagShowTuiMenu) {

		if (state==s_gui::S_GUI_PRESSED && key==SDL_SCANCODE_SEMICOLON) {
			// leave tui menu
			ui->FlagShowTuiMenu = false;

			// If selected a script in tui, run that now
			if (SelectedScript!="")
				scriptMgr->playScript(SelectedScriptDirectory+SelectedScript); //, SelectedScriptDirectory);
			// clear out now
			SelectedScriptDirectory = SelectedScript = "";
			return 1;
		}

		if( ui->handleKeysTui(key, tuiv) )
			return 1;

		return 0;
	}

	//gestion touches video
	if (handleKeyOnVideo) { //videoStatus
		if (state==s_gui::S_GUI_PRESSED)
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
					break;
			}
	} else {
		if (ui->handleKeys(key, mod, unicode, state))
			retVal = 1;
	}

	if (state == s_gui::S_GUI_PRESSED) {
		// Direction and zoom deplacements
		if ((key==SDL_SCANCODE_LEFT) & !(handleKeyOnVideo))
			core->turnLeft(1);
		if ((key==SDL_SCANCODE_RIGHT) & !(handleKeyOnVideo))
			core->turnRight(1);
		if ((key==SDL_SCANCODE_UP) & !(handleKeyOnVideo)) {
			if (mod & KMOD_CTRL)
				core->zoomIn(1);
			else
				core->turnUp(1);
		}
		if ((key==SDL_SCANCODE_DOWN) & !(handleKeyOnVideo)) {
			if (mod & KMOD_CTRL)
				core->zoomOut(1);
			else
				core->turnDown(1);
		}
		if (key==SDL_SCANCODE_PAGEUP)
			core->zoomIn(1);
		if (key==SDL_SCANCODE_PAGEDOWN)
			core->zoomOut(1);
	}
	else {
		// When a deplacement key is released stop moving
		if (key==SDL_SCANCODE_LEFT)
			core->turnLeft(0);
		if (key==SDL_SCANCODE_RIGHT)
			core->turnRight(0);
		if (mod & KMOD_CTRL) {
			if (key==SDL_SCANCODE_UP)
				core->zoomIn(0);
			if (key==SDL_SCANCODE_DOWN)
				core->zoomOut(0);
		} else {
			if (key==SDL_SCANCODE_UP)
				core->turnUp(0);
			if (key==SDL_SCANCODE_DOWN)
				core->turnDown(0);
		}
		if (key==SDL_SCANCODE_PAGEUP)
			core->zoomIn(0);
		if (key==SDL_SCANCODE_PAGEDOWN)
			core->zoomOut(0);
	}
	return retVal;
}
*/

//! For use by TUI - saves all current settings
//! @todo Put in stel_core?
void App::saveCurrentConfig(const string& confFile)
{
	// No longer resaves everything, just settings user can change through UI
	Log.write("Saving configuration file " + confFile + " ...", cLog::LOG_TYPE::L_INFO);
	InitParser conf;
	conf.load(confFile);

	// Main section
	conf.setStr	("main:version", string(VERSION));

	// localization section
	conf.setStr    ("localization:sky_culture", core->getSkyCultureDir());
	conf.setStr    ("localization:sky_locale", core->getSkyLanguage());
	conf.setStr    ("localization:app_locale", getAppLanguage());
	conf.setStr	("localization:time_display_format", spaceDate->getTimeFormatStr());
	conf.setStr	("localization:date_display_format", spaceDate->getDateFormatStr());
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_CUSTOM) {
		conf.setStr("localization:time_zone", spaceDate->getCustomTzName());
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT) {
		conf.setStr("localization:time_zone", "system_default");
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT) {
		conf.setStr("localization:time_zone", "gmt+x");
	}

	// Rendering section
	conf.setBoolean("rendering:flag_antialias_lines", core->getFlagAntialiasLines());
	conf.setDouble("rendering:line_width", core->getLineWidth());

	// viewing section
	conf.setBoolean("viewing:flag_constellation_drawing", coreIO->constellationGetFlagLines());
	conf.setBoolean("viewing:flag_constellation_name", coreIO->constellationGetFlagNames());
	conf.setBoolean("viewing:flag_constellation_art", coreIO->constellationGetFlagArt());
	conf.setBoolean("viewing:flag_constellation_boundaries", coreIO->constellationGetFlagBoundaries());
	conf.setBoolean("viewing:flag_constellation_pick", coreIO->constellationGetFlagIsolateSelected());
	conf.setDouble("viewing:moon_scale", core->getMoonScale());
	conf.setDouble("viewing:sun_scale", core->getSunScale());
	conf.setBoolean("viewing:flag_equatorial_grid", coreIO->skyGridMgrGetFlagShow("GRID_EQUATORIAL"));
	conf.setBoolean("viewing:flag_ecliptic_grid", coreIO->skyGridMgrGetFlagShow("GRID_ECLIPTIC"));
	conf.setBoolean("viewing:flag_galactic_grid", coreIO->skyGridMgrGetFlagShow("GRID_GALACTIC"));
	conf.setBoolean("viewing:flag_azimutal_grid", coreIO->skyGridMgrGetFlagShow("GRID_ALTAZIMUTAL"));
	conf.setBoolean("viewing:flag_equator_line", coreIO->skyGridMgrGetFlagShow("LINE_EQUATOR"));
	conf.setBoolean("viewing:flag_ecliptic_line", coreIO->skyLineMgrGetFlagShow("LINE_ECLIPTIC"));
	conf.setBoolean("viewing:flag_cardinal_points", coreIO->cardinalsPointsGetFlag());
	conf.setBoolean("viewing:flag_zenith_line", coreIO->skyLineMgrGetFlagShow("LINE_ZENITH"));
	conf.setBoolean("viewing:flag_polar_circle", coreIO->skyLineMgrGetFlagShow("LINE_CIRCLE_POLAR"));
	conf.setBoolean("viewing:flag_ecliptic_center", coreIO->skyLineMgrGetFlagShow("LINE_ECLIPTIC_POLE"));
	conf.setBoolean("viewing:flag_galactic_pole", coreIO->skyLineMgrGetFlagShow("LINE_GALACTIC_POLE"));
	conf.setBoolean("viewing:flag_galactic_center", coreIO->skyLineMgrGetFlagShow("LINE_GALACTIC_CENTER"));
	conf.setBoolean("viewing:flag_vernal_points", coreIO->skyLineMgrGetFlagShow("LINE_VERNAL"));
	conf.setBoolean("viewing:flag_analemma", coreIO->skyLineMgrGetFlagShow("LINE_ANALEMMA"));
	conf.setBoolean("viewing:flag_analemma_line", coreIO->skyLineMgrGetFlagShow("LINE_ANALEMMALINE"));
	conf.setBoolean("viewing:flag_aries_line", coreIO->skyLineMgrGetFlagShow("LINE_ARIES"));
	conf.setBoolean("viewing:flag_zodiac", coreIO->skyLineMgrGetFlagShow("LINE_ZODIAC"));

	conf.setBoolean("viewing:flag_greenwich_line", coreIO->skyLineMgrGetFlagShow("LINE_GREENWICH"));
	conf.setBoolean("viewing:flag_vertical_line", coreIO->skyLineMgrGetFlagShow("LINE_VERTICAL"));
	conf.setBoolean("viewing:flag_meridian_line", coreIO->skyLineMgrGetFlagShow("LINE_MERIDIAN"));
	conf.setBoolean("viewing:flag_precession_circle", coreIO->skyLineMgrGetFlagShow("LINE_PRECESSION"));
	conf.setBoolean("viewing:flag_circumpolar_circle", coreIO->skyLineMgrGetFlagShow("LINE_CIRCUMPOLAR"));
	conf.setBoolean("viewing:flag_tropic_lines", coreIO->skyLineMgrGetFlagShow("LINE_TROPIC"));
	conf.setBoolean("viewing:flag_moon_scaled", core->getFlagMoonScaled());
	conf.setBoolean("viewing:flag_sun_scaled", core->getFlagSunScaled());
	conf.setDouble ("viewing:constellation_art_intensity", coreIO->constellationGetArtIntensity());
	conf.setDouble ("viewing:constellation_art_fade_duration", coreIO->constellationGetArtFadeDuration());
	conf.setDouble("viewing:light_pollution_limiting_magnitude", core->getLightPollutionLimitingMagnitude());

	// Landscape section
	conf.setBoolean("landscape:flag_landscape", coreIO->landscapeGetFlag());
	conf.setBoolean("landscape:flag_atmosphere", coreIO->atmosphereGetFlag());
	conf.setBoolean("landscape:flag_fog", coreIO->fogGetFlag());

	// Star section
	conf.setDouble ("stars:star_scale", coreIO->starGetScale());
	conf.setDouble ("stars:star_mag_scale", coreIO->starGetMagScale());
	conf.setDouble("stars:max_mag_star_name", coreIO->starGetMaxMagName());
	conf.setBoolean("stars:flag_star_twinkle", coreIO->starGetFlagTwinkle());
	conf.setDouble("stars:star_twinkle_amount", coreIO->starGetTwinkleAmount());
	conf.setDouble("stars:star_limiting_mag", coreIO->starGetLimitingMag());

	// Color section
	conf.setStr    ("color:azimuthal_color", Utility::vec3fToStr(coreIO->skyGridMgrGetColor("GRID_ALTAZIMUTAL")));
	conf.setStr    ("color:equatorial_color", Utility::vec3fToStr(coreIO->skyGridMgrGetColor("GRID_EQUATORIAL")));
	conf.setStr    ("color:ecliptic_color", Utility::vec3fToStr(coreIO->skyGridMgrGetColor("GRID_ECLIPTIC")));
	conf.setStr    ("color:equator_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_EQUATOR")));
	conf.setStr    ("color:ecliptic_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ECLIPTIC")));
	conf.setStr    ("color:meridian_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_MERIDIAN")));
	conf.setStr    ("color:zenith_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ZENITH")));
	conf.setStr    ("color:polar_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_CIRCLE_POLAR")));
	conf.setStr    ("color:ecliptic_center_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ECLIPTIC_POLE")));
	conf.setStr    ("color:galactic_pole_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_GALACTIC_POLE")));
	conf.setStr    ("color:galactic_center_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_GALACTIC_CENTER")));
	conf.setStr    ("color:vernal_points_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_VERNAL")));
	conf.setStr    ("color:analemma_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ANALEMMA")));
	conf.setStr    ("color:analemma_line_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ANALEMMALINE")));
	conf.setStr    ("color:aries_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ARIES")));
	conf.setStr    ("color:zodiac_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_ZODIAC")));
	conf.setStr    ("color:personal_color", Utility::vec3fToStr(coreIO->personnalGetColor()));
	conf.setStr    ("color:personeq_color", Utility::vec3fToStr(coreIO->personeqGetColor()));
	conf.setStr    ("color:greenwich_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_GREENWICH")));
	conf.setStr    ("color:vertical_line", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_VERTICAL")));
	conf.setStr    ("color:const_lines_color", Utility::vec3fToStr(coreIO->constellationGetColorLine()));
	conf.setStr    ("color:const_names_color", Utility::vec3fToStr(coreIO->constellationGetColorNames()));
	conf.setStr    ("color:const_art_color", Utility::vec3fToStr(coreIO->constellationGetColorArt()));
	conf.setStr    ("color:const_boundary_color", Utility::vec3fToStr(coreIO->constellationGetColorBoundaries()));
	conf.setStr	("color:nebula_label_color", Utility::vec3fToStr(coreIO->nebulaGetColorLabels()));
	conf.setStr	("color:nebula_circle_color", Utility::vec3fToStr(coreIO->nebulaGetColorCircle()));
	conf.setStr	("color:precession_circle_color", Utility::vec3fToStr(coreIO->skyLineMgrGetColor("LINE_PRECESSION")));
	conf.setStr    ("color:cardinal_color", Utility::vec3fToStr(coreIO->cardinalsPointsGetColor()));

	// gui section
	conf.setDouble("gui:mouse_cursor_timeout",getMouseCursorTimeout());

	// Text ui section
	conf.setBoolean("tui:flag_show_gravity_ui", ui->getFlagShowGravityUi());
	conf.setBoolean("tui:flag_show_tui_datetime", ui->getFlagShowTuiDateTime());
	conf.setBoolean("tui:flag_show_tui_short_obj_info", ui->getFlagShowTuiShortObjInfo());

	// Navigation section
	conf.setBoolean("navigation:flag_manual_zoom", core->getFlagManualAutoZoom());
	conf.setDouble ("navigation:auto_move_duration", core->getAutoMoveDuration());
	conf.setDouble ("navigation:zoom_speed", core->getZoomSpeed());
	conf.setDouble ("navigation:preset_sky_time", PresetSkyTime);
	conf.setStr	("navigation:startup_time_mode", StartupTimeMode);
	conf.setStr	("navigation:day_key_mode", DayKeyMode);
	conf.setDouble ("navigation:heading", core->getHeading());

	// Astro section
	conf.setBoolean("astro:flag_object_trails", coreIO->planetsGetFlagTrails());
	conf.setBoolean("astro:flag_bright_nebulae", coreIO->nebulaGetFlagBright());
	conf.setBoolean("astro:flag_stars", coreIO->starGetFlag());
	conf.setBoolean("astro:flag_star_name", coreIO->starGetFlagName());
	conf.setBoolean("astro:flag_nebula", coreIO->nebulaGetFlag());
	conf.setBoolean("astro:flag_nebula_names", coreIO->nebulaGetFlagNames());
	conf.setBoolean("astro:flag_nebula_hints", coreIO->nebulaGetFlagHints());
	conf.setDouble("astro:max_mag_nebula_name", coreIO->nebulaGetMaxMagHints());
	conf.setBoolean("astro:flag_planets", coreIO->planetsGetFlag());
	conf.setBoolean("astro:flag_planets_hints", coreIO->planetsGetFlagHints());
	conf.setBoolean("astro:flag_planets_orbits", coreIO->planetsGetFlagOrbits());
	conf.setBoolean("astro:flag_light_travel_time", core->getFlagLightTravelTime());

	conf.setBoolean("astro:flag_milky_way", coreIO->milkyWayGetFlag());
	conf.setDouble("astro:milky_way_intensity", coreIO->milkyWayGetIntensity());

	conf.setDouble("astro:star_size_limit", coreIO->starGetSizeLimit());
	conf.setDouble("astro:planet_size_marginal_limit", core->getPlanetsSizeLimit());

	// Get landscape and other observatory info
	// TODO: shouldn't observer already know what section to save in?
	(core->getObservatory())->setConf(conf, "init_location");
	conf.save(confFile);

}

void App::recordCommand(string commandline)
{
	scriptMgr->recordCommand(commandline);
}

//! Masterput script launch
//! @todo rewriting with pipe no kluge!
void App::masterput()
{
	std::string action = settings->getFtpDir()+"pub/masterput.launch";
	FILE * tempFile = fopen(action.c_str(),"r");
	if (tempFile) {
		fclose(tempFile);
		Log.write("MASTERPUT is in action", cLog::LOG_TYPE::L_INFO);
		unlink(action.c_str());
		scriptMgr->playScript(settings->getFtpDir()+"pub/script.sts"); //, settings->getFtpDir()+"pub/");
	}
}

void App::tcpGetPosition()
{
	char tmp[1024];
	memset(tmp, '\0', 1024);
	sprintf(tmp,"%2.2f;%3.2f;%10.2f;%10.6f;%10.6f;", coreIO->observatoryGetLatitude(), coreIO->observatoryGetLongitude(), coreIO->observatoryGetAltitude(), coreIO->getJDay(),core->getHeading());
	Log.write(tmp);
	tcp->setOutput(tmp);
}

void App::start_main_loop()
{
	AppVisible = true;		// At The Beginning, Our App Is Visible
	isAlive = true; 		// au debut, on veut que l'application ne s'arrete pas :)

	//center mouse in middle screen
	mSdl->warpMouseInWindow(width /2, height /2);

	internalClock->init();
	internalClock->fixMaxFps();

	// Start the main loop
	while (isAlive) {
		//~ pd.startTimer("App::start_main_loop$"); //Debug
		if (videoStatus != media->playerGetAlive()) {
			if (media->playerGetAlive() == false) {
				Log.write("fin video on retourne a fps max");
				ui->setHandleKeyOnVideo(false);
				media->playerStop();
				internalClock->fixMaxFps();
			} else {
				ui->setHandleKeyOnVideo(true);
				internalClock->setVideoFps(media->playerGetFps());
				Log.write("changement de tempo, minfps = " + media->playerGetFps());
				internalClock->fixVideoFps();
			}

			videoStatus = !videoStatus;
		}

		//thi code is poor but it does not depend on my will
		if (flagMasterput==true) masterput();

		if (SDL_PollEvent(&E)) {	// Fetch The First Event Of The Queue
			ui->handleInputs(E);
		} else {
			//analyse le joystick au cas ou des events ont été accumulé pour le joystick
			ui->handleDeal();

			// If the application is not visible
			if (!AppVisible) {
				// Leave the CPU alone, don't waste time, simply wait for an event
				SDL_WaitEvent(NULL);
			} else {
				internalClock->setTickCount();
				// Wait a while if drawing a frame right now would exceed our preferred framerate.
				internalClock->wait(media->getDerive());
				internalClock->setTickCount();

				deltaTime = internalClock->getDeltaTime();

				if (deltaTime > internalClock->getFrameDuration() && m_OutputFrameNumber != 0 )
					deltaTime = internalClock->getFrameDuration();

				//~ this->selectStatePosition();
				this->update(deltaTime);		// And update the motions and data
				this->draw(deltaTime);			// Do the drawings!
				mSdl->glSwapWindow();  	// And swap the buffers

				internalClock->setLastCount();

				if (flagScreenshot) {
					writeScreenshot(getNextScreenshotFilename());
					flagScreenshot = false;
				}

				// write out video frame if recording video
				if(m_OutputFrameNumber) {
					std::ostringstream ss;
					ss << std::setw(6) << std::setfill('0') << m_OutputFrameNumber;
					writeScreenshot(settings->getVframeDirectory() + APP_LOWER_NAME + "-" + ss.str() + ".jpg");
					m_OutputFrameNumber++;
				}
			}
		}
		//~ pd.stopTimer("App::start_main_loop$"); //Debug
	}
}

//! Write current video frame to a specified file
void App::writeScreenshot(string filename)
{
	//cette fonction peut être bloquante
	saveScreen->getFreeIndex();
	glReadPixels( (width-height)/2, 0, height, height, GL_RGB, GL_UNSIGNED_BYTE, saveScreen->getFreeBuffer());
	saveScreen->saveScreenBuffer(filename);
}

//! Return the next sequential screenshot filename to use
string App::getNextScreenshotFilename()
{
	string tempName;
	string shotdir = settings->getScreenshotDirectory();

	time_t tTime = time ( NULL );
	tm * tmTime = localtime ( &tTime );
	char timestr[28];
	strftime( timestr, 24, "-%y.%m.%d-%H.%M.%S.jpg", tmTime );

	tempName = shotdir +APP_LOWER_NAME +timestr;
	return tempName;
}

void App::updateDeltaSpeed()
{
	printf("updateDeltaSpeed\n");
	if (deltaSpeed>0)
		commander->executeCommand( "timerate action sincrement");
	else
		commander->executeCommand( "timerate action sdecrement");
}
