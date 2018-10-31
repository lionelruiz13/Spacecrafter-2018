/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014_2015 Association Sirius & LSS team
 * Spacecrafter astronomy simulation and visualization
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


#define __main__
#include "spacecrafter.hpp"
#include "log.hpp"
#include "perf_debug.hpp"

#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <SDL2/SDL.h>


#include "app.hpp"
#include "utility.hpp"
#include "app_settings.hpp"
#include "sdl_facade.hpp"
#include "signals.hpp"
#include <dirent.h>
#include "call_system.hpp"
#include <sys/stat.h>
#include "checkkeys.hpp"
#include "translator.hpp"


#ifdef LINUX
#include "CPUInfo.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#endif


using namespace std;

static void drawIntro(void)
{
	Log.mark();
	Log.write("Welcome to spacecrafter",  cLog::LOG_TYPE::L_INFO);
	string strMsg="Version : " + string(APP_NAME) + " - " + string(EDITION) + " Edition";
	Log.write(strMsg, cLog::LOG_TYPE::L_INFO);
	strMsg.clear();
	strMsg +=" Copyright (c) 2015 Association Sirius, LSS team et al.\n";
	strMsg +=" Copyright (c) 2012-2014 LSS team et al.\n";
	strMsg +=" Copyright (c) 2003-2011 Digitalis Education Solutions, Inc. et al.\n";
	strMsg +=" Copyright (c) 2000-2008 Fabien Chereau et al.\n";
	strMsg +=" http://lss-planetariums.info";
	Log.write(strMsg, cLog::LOG_TYPE::L_INFO);
	Log.mark();
}

static void getUnameInfo()
{
	#ifdef LINUX
	//get uname's info
	struct utsname buffer;
	errno = 0;
	if (uname(&buffer) != 0) {
		switch (errno) {
			case EFAULT:
				Log.write("call to uname() with an invalid pointer", cLog::LOG_TYPE::L_ERROR);
				break;
			default:
				Log.write("error calling uname()", cLog::LOG_TYPE::L_ERROR);
		}
		exit(EXIT_FAILURE);
	}
	string strMsg="system name = "+string(buffer.sysname);
	strMsg+="\nnode name   = "+string(buffer.nodename);
	strMsg+="\nrelease     = "+string(buffer.release);
	strMsg+="\nversion     = "+string(buffer.version);
	strMsg+="\nmachine     = "+string(buffer.machine);
	Log.write(strMsg, cLog::LOG_TYPE::L_INFO);
	Log.mark();
	#endif
}

// Display usage in the console
static void usage(char **argv)
{
	cout << APP_NAME << endl;
	cout << _("Usage: %s [OPTION] ...\n -v, --version          Output version information and exit.\n -h, --help             Display this help and exit.\n");
}

// Check command line arguments
static void check_command_line(int argc, char **argv)
{
	if (argc == 2) {
		if (!(strcmp(argv[1],"--version") && strcmp(argv[1],"-v"))) {
			cout << APP_NAME << endl;
			exit(0);
		}
		if (!(strcmp(argv[1],"--help") && strcmp(argv[1],"-h"))) {
			usage(argv);
			exit(0);
		}
	}

	if (argc > 1) {
		cout << APP_NAME << endl;
		cout << _("%s: Bad command line argument(s)\n")<< endl;
		cout << _("Try `%s --help' for more information.\n");
		exit(1);
	}
}

#ifdef LINUX
static void create_lock_file(string lock_file)
{
	ofstream file(lock_file.c_str(), ios::out);
	file << getpid() << endl;
	file.close();
}

static bool is_lock_file(string lock_file)
{
	ifstream file(lock_file.c_str(), ios::in);
	if (!file ) {
		return false;
	} else {
		string ch;
		file >> ch;
		file.close();
		Log.write("Saved getpid is : "+ ch, cLog::LOG_TYPE::L_INFO);

		string test="kill -0 "+ch +"\n";

		return CallSystem::useSystemCommand(test);
		// 	return false; //lock_file exist & it's an old session
		// else
		// 	return true;  //lock_file exist & pid is the same
	}
	return false; //for compiler
}
#endif


// Main procedure
int main(int argc, char **argv)
{
	#if LINUX
	string homeDir = getenv("HOME");
	string CDIR = homeDir + "/." + APP_LOWER_NAME+"/";
	string DATA_ROOT = string(CONFIG_DATA_DIR);
	#else //win32
	string homeDir = "C:\\spacecrafter\\";
	string CDIR = "C:\\spacecrafter\\";
	string DATA_ROOT="C:\\spacecrafter\\";
	string CONFIG_DATA_DIR="C:\\spacecrafter\\";
	string LOCALEDIR="C:\\spacecrafter\\";
	#endif
	string dirResult;
	// Check the command line
	check_command_line(argc, argv);

	//check if home Directory exist and if not try to create it.
	CallSystem::checkUserDirectory(CDIR, dirResult);
	CallSystem::checkUserSubDirectory(CDIR, dirResult);

	//Open log files
	#ifdef LINUX
	std::string logpath(getenv("HOME") + std::string("/.") + APP_LOWER_NAME +"/log/");
	Log.open(logpath + "spacecrafter.log", logpath + "script.log", logpath + "openGL.log", logpath + "tcp.log");
	#else // on windows
	std::string logpath("c:\\spacecrafter\\log\\");
	Log.open(logpath + "spacecrafter.log", logpath + "script.log", logpath + "openGL.log", logpath + "tcp.log");
	#endif

	// Write the console logo & Uname Information...
	drawIntro();
	getUnameInfo();
	//Information about memory capacity
	Log.write(CallSystem::getRamInfo());
	Log.mark();

	// write in logfile the result of repertory checking
	Log.write(dirResult, cLog::LOG_TYPE::L_INFO);
	dirResult.clear();
	CallSystem::checkIniFiles(CDIR, DATA_ROOT);

	#ifdef LINUX
	string lock_file = PATH_FILE_LOCK;
	Log.write("Lock file is "+ lock_file,cLog::LOG_TYPE::L_INFO);
	Log.write("My getpid() is "+ Utility::doubleToString(getpid()), cLog::LOG_TYPE::L_INFO);

	if (is_lock_file(lock_file)) {
		Log.write("There already has an instance of the running program. New instance aborded", cLog::LOG_TYPE::L_WARNING);
		return 0;
	}
	create_lock_file(lock_file);
	#endif

	// Used for getting system date formatting
	setlocale(LC_TIME, "");

	//save the environnement variable
	Log.write("CONFIG DIR: " + string(CDIR), cLog::LOG_TYPE::L_INFO);
	Log.write("ROOT   DIR: " + string(DATA_ROOT), cLog::LOG_TYPE::L_INFO);
	Log.write("LOCALE DIR: " + string(LOCALEDIR), cLog::LOG_TYPE::L_INFO);


	//test if config.ini is not to old.
	checkConfigIni(CDIR+"config.ini", string(VERSION));

	AppSettings::Init(CDIR, DATA_ROOT, LOCALEDIR);
	InitParser conf;
	AppSettings* ini = AppSettings::Instance();


	#ifdef LINUX
	CPUInfo *cpuInfo =  nullptr;
    #endif // LINUX

	ini->loadAppSettings( &conf );

	Log.setDebug(conf.getBoolean("main:debug"));


	#ifdef LINUX
	if (conf.getBoolean("main:CPU_info")) {
		cpuInfo = new CPUInfo();
		cpuInfo -> init(ini->getLogDir()+"CPUlog.csv",ini->getLogDir()+"GPUlog.csv");
		cpuInfo -> start();
		Log.write("CPUInfo actived",cLog::LOG_TYPE::L_DEBUG);
	}
	#endif

	SDLFacade* sdl = new SDLFacade();
	sdl->initSDL();

	// détermination de la résolution initiale
	bool autoscreen = conf.getBoolean("video:autoscreen");
	Uint16 curW, curH;
	bool fullscreen;
	int antialiasing;

	if (autoscreen) {
		SDL_DisplayMode dm;
		if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
			SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		curW = dm.w;
		curH = dm.h;
		fullscreen = true;
	} else {
		curW = conf.getInt("video:screen_w");
		curH = conf.getInt("video:screen_h");
		fullscreen = conf.getBoolean("video:fullscreen");
	}

	antialiasing = conf.getInt("rendering:antialiasing");
	sdl->createWindow(curW, curH, conf.getInt("video:bbp_mode"), antialiasing, fullscreen, DATA_ROOT + "data/icon.bmp"); //, conf.getBoolean("main:debug_opengl"));
	App* app = new App( sdl );

	// Register custom suspend and term signal handers
	ISignals* signalObj = ISignals::Create(app);
	signalObj->Register( SIGTSTP, ISignals::NSSigTSTP );
	signalObj->Register( SIGTERM, ISignals::NSSigTERM );
	signalObj->Register( SIGINT, ISignals::NSSigTERM );
	signalObj->Register( SIGQUIT, ISignals::NSSigTERM );

	app->initDebugGL(conf.getBoolean("main:debug_opengl"));
	//~ app->init_debug(conf.getBoolean("main:debug"));
	app->initSplash();
	app->init();

	app->startMainLoop();

	#ifdef LINUX
	if( unlink(lock_file.c_str()) != 0 )
		Log.write("Error deleting file.lock",  cLog::LOG_TYPE::L_ERROR);
	else
		Log.write("File file.lock successfully deleted",  cLog::LOG_TYPE::L_INFO);
	#endif

	delete app;
	delete sdl;
	delete signalObj;

	Log.write("EOF", cLog::LOG_TYPE::L_INFO);
	Log.write("EOF", cLog::LOG_TYPE::L_INFO, cLog::LOG_FILE::SCRIPT);

	#ifdef LINUX
	if (cpuInfo != nullptr) {
		cpuInfo->stop();
	}
	#endif

	AppSettings::close();
	// pd.exportData(CDIR+"spacecrafter-perf.csv", false);

	return 0;
}

