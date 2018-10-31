/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2016 of the LSS Team & Association Sirius
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

#include <iostream>
#include <iterator>

#include "skyline_mgr.hpp"
#include "log.hpp"

using namespace std;

SkyLineMgr::SkyLineMgr()
{
	baseColor=Vec3f(0.f, 0.f, 0.f);
	SkyLine::createShader();
}

void SkyLineMgr::draw(const Projector* prj, const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->draw(prj, nav, timeMgr, observatory);
	}
}

SkyLineMgr::~SkyLineMgr()
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		//~ cout << "suppression de " << it->first << endl;
		Log.write("SkyLineMgr : delete " + it->first , cLog::LOG_TYPE::L_INFO);
		delete it->second;
	}

	SkyLine::deleteShader();
}

void SkyLineMgr::update(int delta_time)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->update(delta_time);
	}
}

void SkyLineMgr::translateLabels(Translator& trans)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->translateLabels(trans);
	}
}

void SkyLineMgr::setInternalNav(bool a)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setInternalNav(a);
	}
}


bool SkyLineMgr::isExist(std::string type_obj)
{
	std::map<std::string, SkyLine*>::iterator it=m_map.find(type_obj);
	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end())
		return true;
	else
		return false;
}

void SkyLineMgr::setFont(float font_size, const string& font_name)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setFont(font_size, font_name);
	}
}


void SkyLineMgr::flipFlagShow(std::string typeObj)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->flipFlagShow();
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::flipFlagShow : " << typeObj << " not found" << std::endl;
	Log.write("SkyLineMgr error : flipFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}


void SkyLineMgr::setFlagShow(std::string typeObj, bool a)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setFlagshow(a);
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::setFlagShow : " << typeObj << " not found" << std::endl;
	Log.write("SkyLineMgr error : setFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}


bool SkyLineMgr::getFlagShow(std::string typeObj)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getFlagshow();
		}
	}
	//~ std::cout << "error SkyGridMgr::getFlagShow : " << typeObj << " not found" << std::endl;
	Log.write("SkyLineMgr error : getFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
	return false;
}


void SkyLineMgr::setColor(std::string typeObj, const Vec3f& c)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setColor(c);
			return;
		}
	}
	//~ std::cout << "error SkyGridMgr::setColor : " << typeObj << " not found" << std::endl;
	Log.write("SkyLineMgr error : setColor not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}

const Vec3f& SkyLineMgr::getColor(std::string typeObj)
{
	for (std::map<std::string, SkyLine*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getColor();
		}
	}
	//~ std::cout << "error SkyGridMgr::getColor : " << typeObj << " not found" << std::endl;
	Log.write("SkyLineMgr error : getColor not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
	//~ Vec3f tmp(0.f, 0.f, 0.f);
	return baseColor;
}


SkyLineMgr::LINE_TYPE SkyLineMgr::stringToType(std::string typeObj)
{
	if (typeObj == "LINE_CIRCLE_POLAR")
		return SkyLineMgr::LINE_CIRCLE_POLAR;

	if (typeObj == "LINE_ECLIPTIC_POLE")
		return SkyLineMgr::LINE_ECLIPTIC_POLE;

	if (typeObj == "LINE_GALACTIC_POLE")
		return SkyLineMgr::LINE_GALACTIC_POLE;

	if (typeObj == "LINE_ANALEMMA")
		return SkyLineMgr::LINE_ANALEMMA;

	if (typeObj == "LINE_ANALEMMALINE")
		return SkyLineMgr::LINE_ANALEMMALINE;

	if (typeObj == "LINE_CIRCUMPOLAR")
		return SkyLineMgr::LINE_CIRCUMPOLAR;

	if (typeObj == "LINE_GALACTIC_CENTER")
		return SkyLineMgr::LINE_GALACTIC_CENTER;

	if (typeObj == "LINE_GALACTIC_EQUATOR")
		return SkyLineMgr::LINE_GALACTIC_EQUATOR;

	if (typeObj == "LINE_VERNAL")
		return SkyLineMgr::LINE_VERNAL;

	if (typeObj == "LINE_ANALEMMA")
		return SkyLineMgr::LINE_ANALEMMA;

	if (typeObj == "LINE_ANALEMMALINE")
		return SkyLineMgr::LINE_ANALEMMALINE;

	if (typeObj == "LINE_GREENWICH")
		return SkyLineMgr::LINE_GREENWICH;

	if (typeObj == "LINE_ARIES")
		return SkyLineMgr::LINE_ARIES;

	if (typeObj == "LINE_MERIDIAN")
		return SkyLineMgr::LINE_MERIDIAN;

	if (typeObj == "LINE_TROPIC")
		return SkyLineMgr::LINE_TROPIC;

	if (typeObj == "LINE_EQUATOR")
		return SkyLineMgr::LINE_EQUATOR;

	if (typeObj == "LINE_ECLIPTIC")
		return SkyLineMgr::LINE_ECLIPTIC;

	if (typeObj == "LINE_PRECESSION")
		return SkyLineMgr::LINE_PRECESSION;

	if (typeObj == "LINE_VERTICAL")
		return SkyLineMgr::LINE_VERTICAL;

	if (typeObj == "LINE_ZENITH")
		return SkyLineMgr::LINE_ZENITH;

	if (typeObj == "LINE_ZODIAC")
		return SkyLineMgr::LINE_ZODIAC;

	return SkyLineMgr::LINE_UNKNOWN;
}


void SkyLineMgr::Create(std::string type_obj)
{
	SkyLine* tmp=nullptr;
	std::map<std::string, SkyLine*>::iterator it=m_map.find(type_obj);

	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end()) {
		Log.write("SkyLineMgr already create " + type_obj , cLog::LOG_TYPE::L_ERROR);
		return;
	}

	LINE_TYPE typeObj=stringToType(type_obj);

	switch (typeObj) {
		case LINE_CIRCLE_POLAR :
			Log.write("SkyLineMgr creating LINE_CIRCLE_POLAR" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::POLE,1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ECLIPTIC_POLE:
			Log.write("SkyLineMgr creating LINE LINE_ECLIPTIC_POLE" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::ECLIPTIC_POLE,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_GALACTIC_POLE :
			Log.write("SkyLineMgr creating LINE LINE_GALACTIC_POLE" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Pole(SkyLine_Pole::GALACTIC_POLE,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ANALEMMA :
			Log.write("SkyLineMgr creating LINE_ANALEMMA" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Analemme(SkyLine_Analemme::ANALEMMA,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ANALEMMALINE :
			Log.write("SkyLineMgr creating LINE_ANALEMMALINE" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Analemme(SkyLine_Analemme::ANALEMMALINE,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_CIRCUMPOLAR :
			Log.write("SkyLineMgr creating LINE_CIRCUMPOLAR" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_CircumPolar(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_GALACTIC_CENTER :
			Log.write("SkyLineMgr creating LINE LINE_GALACTIC_CENTER" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Galactic_Center(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_VERNAL :
			Log.write("SkyLineMgr creating LINE_VERNAL" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Vernal(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_GREENWICH :
			Log.write("SkyLineMgr creating LINE_GREENWICH" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Greenwich(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ARIES :
			Log.write("SkyLineMgr creating LINE_ARIES" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Aries(1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_EQUATOR :
			Log.write("SkyLineMgr creating LINE_EQUATOR" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Equator(SkyLine_Equator::EQUATOR,1,192);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_GALACTIC_EQUATOR :
			Log.write("SkyLineMgr creating LINE_GALACTIC_EQUATOR" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Equator(SkyLine_Equator::GALACTIC_EQUATOR,1,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_MERIDIAN :
			Log.write("SkyLineMgr creating LINE_MERIDIAN" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Meridian(1, 360);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_TROPIC :
			Log.write("SkyLineMgr creating LINE_TROPIC" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Tropic(1.0,96);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_PRECESSION :
			Log.write("SkyLineMgr creating LINE_PRECESSION" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Precession(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_VERTICAL :
			Log.write("SkyLineMgr creating LINE_VERTICAL" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Vertical(1,180); //always multiples of 18
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ZENITH :
			Log.write("SkyLineMgr creating LINE_ZENITH" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Zenith(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ZODIAC :
			Log.write("SkyLineMgr creating LINE_ZODIAC" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Zodiac(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		case LINE_ECLIPTIC :
			Log.write("SkyLineMgr creating LINE_MERIDIAN" , cLog::LOG_TYPE::L_INFO);
			tmp=new SkyLine_Ecliptic(1.0,72);
			m_map[type_obj]= tmp;
			return;
			break;

		default:
			Log.write("SkyLineMgr SkyGrid unknown " + type_obj , cLog::LOG_TYPE::L_ERROR);
			break;
	}
}
