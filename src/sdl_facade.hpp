/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

/*
 * A wrapper around basic SDL functionality such as initialization, surface creation,
 * and available video mode queries.
 */

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>

class SDLFacade {

public:
	static SDL_Cursor *create_cursor(const char *image[]);
	SDLFacade();
	virtual ~SDLFacade();
	SDLFacade(SDLFacade const &) = delete;
	SDLFacade& operator = (SDLFacade const &) = delete;

	// Must be called prior to any other SDL methods
	void initSDL( void );

	// Creates the rendering target. //Must be called prior to any OpenGL functions
	void createWindow(Uint16 w, Uint16 h, int bbpMode, int antialiasing,  bool fullScreen, std::string iconFile); //, bool _debug);

	// Video mode queries
	void getResolution( Uint16* const w, Uint16* const h ) const;
	void getCurrentRes( Uint16* const w, Uint16* const h ) const;
	Uint16 getCurrentResW() const {
		return windowW;
	}
	Uint16 getCurrentResH() const {
		return windowH;
	}

	void warpMouseInWindow(float x, float y) const {
		SDL_WarpMouseInWindow( window, x , y);
	}

	void glSwapWindow() const{
		SDL_GL_SwapWindow(window);
	}

	void getGLInfos();

	void getWorkGroupsCapabilities();

private:
	void getLogInfos(int w, int h);
	SDL_Window *window = nullptr;
	SDL_GLContext context;
	SDL_Cursor *Cursor = nullptr;
	Uint16 windowW;
	Uint16 windowH;
};
