/*
 * Spacecrafter astronomy simulation and visualization
 *
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which handles  Text for User script
#include <string>
#include <fstream>
#include <iostream>

#include "log.hpp"
#include "text_mgr.hpp"
#include "text.hpp"

//~ using namespace std;


TextMgr::TextMgr()
{
	for(int i=0; i<7; i++)
		textFont[i]=nullptr;
	textToDestroy="";
}

TextMgr::~TextMgr()
{
	std::vector<Text *>::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); iter++)
		delete(*iter);

	for(int i=0; i<7; i++) {
		if (textFont[i]) delete textFont[i];
		textFont[i]=nullptr;
	}
}

void TextMgr::update(int delta_time)
{
	std::vector < Text * >::const_iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		(*iter)->update(delta_time);
		if ((*iter)->isDead()) {
			textToDestroy = (*iter)->getName();
		}
	}
	if (!textToDestroy.empty() ) {
		del(textToDestroy);
		textToDestroy.clear();
	}
}


bool TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size, const Vec3f &color, int timeout)
{
	Text *token =nullptr;
	token = new Text(name, text, altitude, azimuth, size, color, timeout);
	if (token != nullptr) {
		textUsr.push_back(token);
		return true;
	} else {
		Log.write("TEXT: Bad text syntax", cLog::LOG_TYPE::L_ERROR);
		return false;
	}
}

bool TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size, int timeout)
{
	return add(name, text, altitude, azimuth, size, defaultTextColor, timeout);
}


void TextMgr::setColor(const Vec3f& c)
{
	defaultTextColor = c;
}

void TextMgr::clear()
{
	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		delete(*iter);
	}
	textUsr.clear();
}

bool TextMgr::del(const std::string &name)
{
	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		if ((*iter)->getName() == name) {
			delete(*iter);
			textUsr.erase(iter);
			return true;
		}
	}
	return false;
}

void TextMgr::nameUpdate(const std::string &name, const std::string &text)
{
	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		if ((*iter)->getName() == name) {
			(*iter)->textUpdate(text);
			return;
		}
	}
}

void TextMgr::textDisplay(const std::string &name , bool displ)
{
	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		if ((*iter)->getName() == name) {
			(*iter)->setDisplay(displ);
			return;
		}
	}
}

void TextMgr::setFadingDuration(int t)
{
	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		(*iter)->setFadingDuration(t);
	}
}


void TextMgr::setFont(float font_size, const std::string& font_name)
{
	if (font_size<10) {
		font_size=10;
		Log.write("text size to small fixed to 10", cLog::LOG_TYPE::L_WARNING);
	}

	for(int i=0; i<7; i++) {
		if (textFont[i]) delete textFont[i];
		textFont[i]=nullptr;
	}
	for(int i=0; i<7; i++) {
		textFont[i] = new s_font(font_size+2*(i-3), font_name);
		if (textFont[i]==nullptr) {
			Log.write("TEXT: can't create text usr font", cLog::LOG_TYPE::L_ERROR);
			isUsable = false;
		}
	}
	isUsable= true;

	if (!isUsable)
		Log.write("TEXT: module disable", cLog::LOG_TYPE::L_WARNING);
}


void TextMgr::draw(const Projector* prj)
{
	if (!isUsable)
		return;

	std::vector < Text * >::iterator iter;
	for (iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		(*iter)->draw(prj,textFont);
	}
}
