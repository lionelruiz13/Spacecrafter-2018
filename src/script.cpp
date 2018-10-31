/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014-2017 Association Sirius
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

#include <iostream>
#include <fstream>
#include "script.hpp"
#include <cstddef>
#include "log.hpp"

using namespace std;

//~ void print( vector <string> & v )
//~ {
	//~ for (size_t n = 0; n < v.size(); n++)
		//~ Log.write(v[n],  cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT);
//~ }

Token::Token(const std::string &s, const std::string &p)
{
	elmt.clear();
	path.clear();
	pNext=nullptr;
	elmt=s;
	path=p;
}

Token::~Token()
{
	if (pNext != nullptr) delete pNext;
}

void Token::printToken()
{
	Log.write("Token script : " + path + " : " + elmt,  cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT);
}

Script::Script()
{
	//cout << "Script->init()" << endl;
	pFirst=nullptr;
	pLast=nullptr;
	p2First=nullptr;
	p2Last=nullptr;
}

Script::~Script()
{
	if (pFirst != nullptr) delete pFirst;
}

void Script::printScript()
{
	Token *pMove=pFirst;
	Log.write("Printing script: --BEGIN",  cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT);
	while (pMove !=nullptr) {
		Log.write("   " + pMove->getToken(),  cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT);
		pMove=pMove->pNext;
	}
	Log.write("Printing script: --END",  cLog::LOG_TYPE::L_DEBUG, cLog::LOG_FILE::SCRIPT);
}

void Script::clean()
{
	if (pFirst != nullptr) delete pFirst;
	pFirst = nullptr;
	pLast = nullptr;
	p2First = nullptr;
	p2Last = nullptr;
}

int Script::load(const std::string &script_file, const std::string &script_path )
{
	if (pFirst == nullptr) {
		//cout << "loading script "<< script_file << " when no script is playing" << endl;
		return loadInternal(script_file, script_path ,ListPosition::first);
	} else {
		//cout << "loading script " << script_file << " when script is playing" << endl;
		return loadInternal(script_file, script_path ,ListPosition::second);
	}
}

int Script::loadInternal(const std::string &script_file,const std::string & script_path , ListPosition wp)
{
	//~ printf("s : %s p : %s\n", script_file.c_str() , script_path.c_str() );
	input_file = new ifstream(script_file.c_str());

	if (! input_file->is_open()) {
		Log.write("Unable to open script: " + script_file,  cLog::LOG_TYPE::L_ERROR, cLog::LOG_FILE::SCRIPT);
		return 0;
	}
	vector <string> fields;

	bool is_script_empty=true;
	string line;
	Token *token=nullptr;
	while (! input_file->eof() ) {
		getline(*input_file,line);

		if ( line[0] != '#' && line[0] != 0 && line[0] != '\r' && line[0] != '\n') {
			//cout << "[script.cpp => Line is: " << line << "]"<< endl;
			token=new Token(line, script_path);
			is_script_empty=false;
			if (wp==ListPosition::first)
				addFirst(token);
			else
				addSecond(token);
		}
	}
	input_file->close();
	if ((! is_script_empty) && (wp==ListPosition::second)) {
		p2Last->pNext=pFirst;
		pFirst=p2First;
		p2Last=nullptr;
		p2First=nullptr;
	}
	//printScript();
	return 1;
}

void Script::addFirstInQueue(Token * token){
	
	if (pFirst == nullptr) {
		pFirst=token;
		pLast=token;
	}else {
		
		Token * temp = pFirst;
		pFirst = token;
		token->pNext = temp;
	}
}

void Script::addFirst(Token *token)
{
	if (pFirst == nullptr) {
		pFirst=token;
		pLast=token;
	} else {
		pLast->pNext=token;
		pLast=pLast->pNext;
	}
}

void Script::addSecond(Token *token)
{
	if (p2First == nullptr) {
		p2First=token;
		p2Last=token;
	} else {
		p2Last->pNext=token;
		p2Last=p2Last->pNext;
	}
}

int Script::getFirst(string &command, string &dataDir)
{
	Token *pMove;
	if (pFirst == nullptr) {
		Log.write("End of script",  cLog::LOG_TYPE::L_INFO, cLog::LOG_FILE::SCRIPT);
		command="script action end";
		dataDir="";
		return 0;
	} else {
		pMove=pFirst;
		//pFirst->printToken();
		dataDir=pFirst->getTokenPath();
		command=pFirst->getToken();
		if (command=="script action end" && (pFirst->pNext != nullptr)) {
			Log.write("End of script  detected but not at end of the execution stack", cLog::LOG_TYPE::L_WARNING, cLog::LOG_FILE::SCRIPT);
		}
		//cout << " script.cpp : " << command << endl;
		pFirst=pFirst->pNext;
		pMove->pNext=nullptr;
		delete pMove;
		return 1;
	}
}
