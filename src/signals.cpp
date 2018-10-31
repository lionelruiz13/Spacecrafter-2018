/*
 * Spacecrafter astronomy simulation and visualization
 *
 * File Author: Trystan A. Larey-Williams
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#include "spacecrafter.hpp"
#include "app.hpp"
#include "app_settings.hpp"
#include "signals.hpp"
#include <sys/time.h>

App* ISignals::m_app = NULL;

using namespace std;

#ifdef WIN32
int setitimer( int, struct itimerval*, struct itimerval* )
{
	return 0;
}
int sigaction(int, const struct sigaction*, struct sigaction* )
{
	return 0;
};
#endif

ISignals* ISignals::Create( App* const app )
{
	m_app = app;

	if( AppSettings::Instance()->Windows() )
		return new Win32Signals(); 	// Limited signal support, not posix compliant
	else
		return new PosixSignals();
}

ISignals::~ISignals()
{}

void ISignals::NSSigTERM( int )
{
	// Tell app to stop gracefully
	cout << "Caught SIGTERM, shutting down." << endl;
	if( m_app )
		m_app->quit();
}

void ISignals::NSSigTSTP( int )
{
	struct sigaction restore;
	restore.sa_handler = SIG_DFL;
	restore.sa_flags = SA_RESTART;
	sigaction( SIGTSTP, &restore, NULL );
	raise( SIGTSTP );
}


void ISignals::NSSigCONT( int )
{
	// Re-register SIGTSTP handler then raise default SIGCONT
	struct sigaction act;
	act.sa_handler = ISignals::NSSigTSTP;
	act.sa_flags = SA_RESTART;
	sigaction( SIGTSTP, &act, NULL );

	// Restore default continue handler
	struct sigaction restore;
	restore.sa_handler = SIG_DFL;
	restore.sa_flags = SA_RESTART;
	sigaction( SIGCONT, &restore, NULL );

	// Raise default continue handler
	raise( SIGCONT );
}

// Posix Signal Implementation ////////////////////////////////////////////////
void PosixSignals::Register( int sigid, void(*handler)(int) )
{
	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = SA_RESTART;
	sigaction ( sigid, &act, NULL);
}

void PosixSignals::UnRegister( int sigid )
{
	struct sigaction act;
	act.sa_handler = SIG_DFL;
	act.sa_flags = SA_RESTART;
	sigaction ( sigid, &act, NULL);
}

// Win32 Signal Implementation ////////////////////////////////////////////////
void Win32Signals::Register( int sigid, void(*handler)(int) )
{
	signal( sigid, handler );
}

void Win32Signals::UnRegister( int sigid )
{
	signal( sigid, SIG_DFL );
}
