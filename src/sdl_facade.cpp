/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
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

//~ #include "app.hpp"
#include "spacecrafter.hpp"
#include "sdl_facade.hpp"
#include "utility.hpp"
#include "log.hpp"
#include <string>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

using namespace std;

SDLFacade::SDLFacade()
{
}

SDLFacade::~SDLFacade()
{
	SDL_FreeCursor(Cursor);
	SDL_DestroyWindow(window);
}

void SDLFacade::getResolution( Uint16* const w, Uint16* const h ) const
{
	*w = windowW;
	*h = windowH;
}

void SDLFacade::getCurrentRes( Uint16* const w, Uint16* const h ) const
{
	*w = 0;
	*h = 0;
	SDL_DisplayMode crtdmode;
	int result= SDL_GetCurrentDisplayMode(0,&crtdmode);


	if( !result ) {
		*w = crtdmode.w;
		*h = crtdmode.h;
	}
}

void SDLFacade::createWindow( Uint16 w, Uint16 h, int bppMode, int antialiasing, bool fullScreen, string iconFile) // , bool _debugGL)
{

	Uint32	Vflags;		// Our Video Flags
	//~ window=NULL;
	windowW = w;
	windowH = h;

	// We want a hardware surface
	Vflags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//~ if (_debugGL)
	//~ SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	// If fullscreen, set the Flag
	if (fullScreen) Vflags|=SDL_WINDOW_FULLSCREEN;

	if (antialiasing>1) {
		//~ if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) == -1)
		//~ fprintf(stderr, "Unable to initialise SDL_GL_MULTISAMPLEBUFFERS à 1\n");

		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasing) == -1) {
			fprintf(stderr, "Unable to initialise SDL_GL_MULTISAMPLESAMPLES\n");
			Log.write("Antialiasing opérationnel, valeur "+Utility::intToString(antialiasing),cLog::LOG_TYPE::L_INFO);
			glEnable(GL_MULTISAMPLE);
		}
	} else
		Log.write("no antialiasing.",cLog::LOG_TYPE::L_WARNING);

	// Create the SDL screen surface
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0"); //lost screen after mplayer
	window  = SDL_CreateWindow(APP_NAME,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,windowW,windowH,Vflags);
	if (window == nullptr) {
		Log.write("SDL Could not create window: "+ string(SDL_GetError()) +" Retrying with stencil size 0", cLog::LOG_TYPE::L_ERROR);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
		window  = SDL_CreateWindow(APP_NAME,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,windowW,windowH,Vflags);
		if (window == nullptr) {
			Log.write("SDL Could not create window even with stencil 0: " + string(SDL_GetError()), cLog::LOG_TYPE::L_ERROR);
			exit(1);
		}
	} else
		getLogInfos( w,  h);
	SDL_DisableScreenSaver();
	context = SDL_GL_CreateContext(window);

	GLenum code;

	/* initialisation de GLEW */
	code = glewInit();
	if(code != GLEW_OK) {
		Log.write("SDL Unable to init GLEW : error " + Utility::intToString(code), cLog::LOG_TYPE::L_ERROR);
	}

	/*get GL infos */
	getGLInfos();

	getWorkGroupsCapabilities();

	/* test opengl version 4.1 */
	if ( ! glewIsSupported("GL_VERSION_4_3") ) {
		Log.write("GLEW no openGL 4_3 support" , cLog::LOG_TYPE::L_ERROR);
		exit(2);
	} else
		Log.write("GLEW openGL 4_3 enable" , cLog::LOG_TYPE::L_INFO);

	// set mouse cursor
	static const char *arrow[] = {
		/* width height num_colors chars_per_pixel */
		"    32    32        3            1",
		/* colors */
		"X c #000000",
		". c #ffffff",
		"  c None",
		/* pixels */
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"                                ",
		"   XXXXXXXX         XXXXXXXX    ",
		"   X......X         X......X    ",
		"   XXXXXXXX         XXXXXXXX    ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"15,17"
	};

	Cursor = create_cursor(arrow);
	SDL_SetCursor(Cursor);

	// Set the window icon
	SDL_Surface *icon = SDL_LoadBMP((iconFile).c_str());
	SDL_SetWindowIcon(window,icon);
	SDL_FreeSurface(icon);

	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
	glClear(GL_COLOR_BUFFER_BIT);
	Log.mark();
}

void SDLFacade::initSDL()
{
	Log.mark();

	// Init the SDL library, the VIDEO subsystem
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER| SDL_INIT_JOYSTICK)<0) {
		Log.write("Error: unable to open SDL : "+ string(SDL_GetError()), cLog::LOG_TYPE::L_ERROR );
		exit(-1);
	} else
		Log.write("SDL init oki !", cLog::LOG_TYPE::L_INFO );


	//Init the SDL_TTF library
	if(TTF_Init()==-1) {
		Log.write("Error: unable to open SDL_TTF : " + string(TTF_GetError()), cLog::LOG_TYPE::L_ERROR );
		exit(-1);
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
		Log.write("Error Mix_OpenAudio: "+ string(Mix_GetError()), cLog::LOG_TYPE::L_ERROR );
		exit(-1);
	} else
		Log.write("SDL Sound loaded", cLog::LOG_TYPE::L_INFO);

	// Make sure that SDL_Quit will be called in case of exit()
	atexit(TTF_Quit);
	atexit(SDL_Quit);
	atexit(Mix_CloseAudio);
}

// from an sdl wiki
SDL_Cursor* SDLFacade::create_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;

	i = -1;
	for ( row=0; row<32; ++row ) {
		for ( col=0; col<32; ++col ) {
			if ( col % 8 ) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4+row][col]) {
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				case '.':
					mask[i] |= 0x01;
					break;
				case ' ':
					break;
			}
		}
	}
	sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void SDLFacade::getLogInfos(int w, int h)
{
	stringstream out;
	out << w << "x" << h << "px";
	Log.write("Windows size is "+out.str(), cLog::LOG_TYPE::L_INFO );
	Log.write("Video driver is  " + string(SDL_GetCurrentVideoDriver()) , cLog::LOG_TYPE::L_INFO);
	out.clear();
	static int display_in_use = 0; /* Only using first display */
	int i, display_mode_count;
	SDL_DisplayMode mode;
	Uint32 f;
	Log.write("SDL_GetNumVideoDisplays(): "+ Utility::doubleToString(SDL_GetNumVideoDisplays()), cLog::LOG_TYPE::L_INFO);

	display_mode_count = SDL_GetNumDisplayModes(display_in_use);
	if (display_mode_count < 1) {
		Log.write("SDL_GetNumDisplayModes failed: "+string(SDL_GetError()), cLog::LOG_TYPE::L_ERROR);
	}
	Log.write("SDL_GetNumDisplayModes:  "+ Utility::doubleToString(display_mode_count), cLog::LOG_TYPE::L_INFO);

	for (i = 0; i < display_mode_count; ++i) {
		stringstream output;
		if (SDL_GetDisplayMode(display_in_use, i, &mode) != 0) {
			Log.write("SDL_GetDisplayMode failed: "+ string(SDL_GetError()),cLog::LOG_TYPE::L_ERROR);
		}
		f = mode.format;
		output << "Mode " << i << " bpp " << SDL_BITSPERPIXEL(f) << " " << SDL_GetPixelFormatName(f) << " " << mode.w << " " << mode.h;
		Log.write(output.str(), cLog::LOG_TYPE::L_INFO);
		output.clear();
	}
}

void SDLFacade::getGLInfos()
{
	Log.mark();
	stringstream oss;
	oss << "GL info" << endl << "GL_VENDOR: " << glGetString(GL_VENDOR) << endl;
	oss << "GL_RENDERER: " << glGetString(GL_RENDERER) << endl << "GL_VERSION: " << glGetString(GL_VERSION) << endl;
	oss << "SHADER_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
	//cout << oss.str() << endl;
	Log.write(oss.str(),  cLog::LOG_TYPE::L_INFO);
}


void SDLFacade::getWorkGroupsCapabilities()
{
	//~ Log.mark();
	stringstream oss;

	int workgroup_count[3];
	int workgroup_size[3];
	int workgroup_invocations;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroup_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroup_count[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroup_count[2]);
	oss << "Taille maximale des workgroups: " << workgroup_count[0] << " " << workgroup_count[1] << " " << workgroup_count[2]<< endl;

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroup_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroup_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroup_size[2]);
	oss << "Nombre maximal d'invocation locale: " << workgroup_size[0] << " " << workgroup_size[1] << " " << workgroup_size[2] << endl;

	glGetIntegerv (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);
	oss << "Nombre maximum d'invocation de workgroups: " << workgroup_invocations << endl;

	Log.write(oss.str(),  cLog::LOG_TYPE::L_INFO);
}
