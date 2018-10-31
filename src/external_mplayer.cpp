/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2007 Digitalis Education Solutions, Inc.
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

// manage an external viewer

#include <iostream>
#include "external_mplayer.hpp"
#include "app_settings.hpp"
#include "call_system.hpp"
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "log.hpp"
#include "spacecrafter.hpp"
#include "utility.hpp"

//pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <signal.h> //kill

#define LEN 25

using namespace std;


ExternalMplayer::ExternalMplayer(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}

void ExternalMplayer::init(const std::string &mplayerFileName, const std::string &mplayerMkfifoName, bool enable_mplayer)
{
	Log.write("service ExternalMplayer est initialisé comme inactif", cLog::LOG_TYPE::L_DEBUG);
}

void ExternalMplayer::update(int delta_time)
{
}

void ExternalMplayer::initMkfifo(bool enable)
{
	serviceAvariable = false;
}


void ExternalMplayer::launchMplayer()
{
}


int ExternalMplayer::play(const std::string &filename)
{
}

ExternalMplayer::~ExternalMplayer()
{
}

void ExternalMplayer::jumpAbsolute(int secondes)
{
}

void ExternalMplayer::jumpRelative(int secondes)
{
}

void ExternalMplayer::volume(int sound)
{
}


void ExternalMplayer::execute(const std::string &msg)
{
}

void ExternalMplayer::speed(int lvl)
{
}

void ExternalMplayer::pause()
{
}


void ExternalMplayer::stop()
{
}

bool ExternalMplayer::writeToMplayer(const std::string &msg)
{
}
