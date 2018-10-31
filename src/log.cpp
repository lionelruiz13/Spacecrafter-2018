/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
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

#include "log.hpp"
#include <exception>
#include <string>
#include <time.h>

// thanks to internet for color !!
// http://stackoverflow.com/questions/1961209/making-some-text-in-printf-appear-in-green-and-red
#define LOG_RESET   "\033[0m"
#define LOG_BLACK   "\033[30m"      /* Black */
#define LOG_RED     "\033[31m"      /* Red */
#define LOG_GREEN   "\033[32m"      /* Green */
#define LOG_YELLOW  "\033[33m"      /* Yellow */
#define LOG_BLUE    "\033[34m"      /* Blue */
#define LOG_MAGENTA "\033[35m"      /* Magenta */
#define LOG_CYAN    "\033[36m"      /* Cyan */
#define LOG_WHITE   "\033[37m"      /* White */
#define LOG_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define LOG_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define LOG_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define LOG_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define LOG_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define LOG_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define LOG_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define LOG_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

cLog::cLog()
{

}

void cLog::open(const std::string& LogfilePath, const std::string& scriptLogfilePath, const std::string& openglLogfilePath, const std::string& tcpLogfilePath)
{
	Logfile.open(LogfilePath, std::ofstream::out | std::ofstream::trunc);

	ScriptLogfile.open(
	    scriptLogfilePath.substr(0, scriptLogfilePath.find_last_of("."))
	    + std::string ("-")
	    + getDate()
	    + scriptLogfilePath.substr(scriptLogfilePath.find_last_of("."), std::string::npos)
	    , std::ofstream::out | std::ofstream::app);

	OpenglLogfile.open(openglLogfilePath, std::ofstream::out | std::ofstream::trunc);
	TcpLogfile.open(tcpLogfilePath, std::ofstream::out | std::ofstream::trunc);

	if (!(Logfile.is_open() && ScriptLogfile.is_open() && OpenglLogfile.is_open() && TcpLogfile.is_open())) {
		std::cerr << "(EE): Couldn't open file log!\n Please check file/directory permissions" << std::endl;
		throw;
	}

	unsigned char bom[] = { 0xEF,0xBB,0xBF };
	Logfile.write((char*)bom, sizeof(bom));
	ScriptLogfile.write((char*)bom, sizeof(bom));
	OpenglLogfile.write((char*)bom, sizeof(bom));
	TcpLogfile.write((char*)bom, sizeof(bom));
}

cLog::~cLog()
{
	Logfile.close();
	ScriptLogfile.close();
	OpenglLogfile.close();
	TcpLogfile.close();
}


void cLog::write(const std::string& texte, const LOG_TYPE& type, const LOG_FILE& fichier)
{
	writeMutex.lock();

	if (isDebug)
		writeConsole(texte, type);

	std::string ligne;

	switch(type) {
		case LOG_TYPE::L_WARNING :
			ligne.append(LOG_WW);
			break;
		case LOG_TYPE::L_ERROR :
			ligne.append(LOG_EE);
			break;
		case LOG_TYPE::L_DEBUG :
			ligne.append(LOG_DD);
			break;
		case LOG_TYPE::L_INFO :
			ligne.append(LOG_II);
			break;
		default :
			;
	}
	ligne.append(texte);
	ligne.append("\r\n");

	switch(fichier) {
		case LOG_FILE::INTERNAL :
			Logfile << ligne;
			Logfile.flush();
			break;
		case LOG_FILE::SCRIPT :
			ScriptLogfile << ligne;
			ScriptLogfile.flush();
			break;
		case LOG_FILE::OPENGL :
			OpenglLogfile << ligne;
			OpenglLogfile.flush();
			break;
		case LOG_FILE::TCP :
			TcpLogfile << ligne;
			TcpLogfile.flush();
			break;
		default : {
			Logfile << ligne;
			Logfile.flush();
		}
	}
	writeMutex.unlock();
}

void cLog::mark(const LOG_FILE& fichier)
{
	write(std::string("================================================================="), LOG_TYPE::L_OTHER, fichier);
}

void cLog::writeConsole(const std::string& texte, const LOG_TYPE& type)
{
	std::string ligne;
	switch(type) {
		case LOG_TYPE::L_WARNING :
			ligne.append(LOG_BOLDYELLOW);
			ligne.append(LOG_WW);
			ligne.append(LOG_RESET);
			ligne.append(LOG_BOLDWHITE);
			break;

		case LOG_TYPE::L_ERROR :
			ligne.append(LOG_BOLDRED);
			ligne.append(LOG_EE);
			ligne.append(LOG_RESET);
			ligne.append(LOG_BOLDWHITE);
			break;

		case LOG_TYPE::L_DEBUG :
			ligne.append(LOG_BOLDCYAN);
			ligne.append(LOG_DD);
			ligne.append(LOG_RESET);
			ligne.append(LOG_WHITE);
			break;

		case LOG_TYPE::L_INFO :
			ligne.append(LOG_BOLDGREEN);
			ligne.append(LOG_II);
			ligne.append(LOG_RESET);
			ligne.append(LOG_WHITE);
			break;

		default :
			ligne.append(LOG_WHITE);
	}

	ligne.append(texte);
	ligne.append(LOG_RESET);
	ligne.append("\r\n");

	if(type == LOG_TYPE::L_ERROR || type == LOG_TYPE::L_WARNING) {
		std::cerr << ligne;
	} else {
		std::cout << ligne;
	}

}

std::string cLog::getDate()
{
	time_t tTime = time(NULL);
	tm * tmTime = localtime (&tTime);
	char timestr[8];
	strftime(timestr, 21, "%y.%m.%d", tmTime);
	return std::string(timestr);
}
