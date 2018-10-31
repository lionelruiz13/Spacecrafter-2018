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

#include "skygrid_mgr.hpp"
#include "log.hpp"

SkyGridMgr::SkyGridMgr()
{
	baseColor=Vec3f(0.f, 0.f, 0.f);
}

void SkyGridMgr::draw(const Projector* prj)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->draw(prj);
	}
}

SkyGridMgr::~SkyGridMgr()
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		Log.write("SkyGridMgr : delete " + it->first , cLog::LOG_TYPE::L_INFO);
		delete it->second;
	}
}

void SkyGridMgr::update(int delta_time)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->update(delta_time);
	}
}


void SkyGridMgr::setInternalNav(bool a)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setInternalNav(a);
	}
}


void SkyGridMgr::setFont(float font_size, const std::string& font_name)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		it->second->setFont(font_size, font_name);
	}
}


void SkyGridMgr::flipFlagShow(std::string typeObj)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->flipFlagShow();
			return;
		}
	}
	Log.write("SkyGridMgr error : flipFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}


void SkyGridMgr::setFlagShow(std::string typeObj, bool a)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setFlagShow(a);
			return;
		}
	}
	Log.write("SkyGridMgr error : setFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}


bool SkyGridMgr::getFlagShow(std::string typeObj)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getFlagShow();
		}
	}
	Log.write("SkyGridMgr error : getFlagShow not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
	return false;
}


void SkyGridMgr::setColor(std::string typeObj, const Vec3f& c)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			it->second->setColor(c);
			return;
		}
	}
	Log.write("SkyGridMgr error : setColor not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
}

const Vec3f& SkyGridMgr::getColor(std::string typeObj)
{
	for (std::map<std::string, SkyGrid*>::iterator it=m_map.begin(); it!=m_map.end(); ++it) {
		if (it->first==typeObj) {
			return it->second->getColor();
		}
	}
	Log.write("SkyGridMgr error : getColor not found " + typeObj , cLog::LOG_TYPE::L_WARNING);
	return baseColor;
}


SkyGridMgr::GRID_TYPE SkyGridMgr::stringToType(std::string typeObj)
{
	if (typeObj == "GRID_EQUATORIAL")
		return SkyGridMgr::GRID_EQUATORIAL;

	if (typeObj == "GRID_ALTAZIMUTAL")
		return SkyGridMgr::GRID_ALTAZIMUTAL;

	if (typeObj == "GRID_ECLIPTIC")
		return SkyGridMgr::GRID_ECLIPTIC;

	if (typeObj == "GRID_GALACTIC")
		return SkyGridMgr::GRID_GALACTIC;

	return SkyGridMgr::GRID_UNKNOWN;
}

void SkyGridMgr::Create(std::string type_obj)
{
	SkyGrid* tmp=nullptr;
	std::map<std::string, SkyGrid*>::iterator it=m_map.find(type_obj);

	//si l'itérateur ne vaut pas map.end(), cela signifie que que la clé à été trouvée
	if(it!=m_map.end()) {
		Log.write("SkyGridMgr SkyGrid already create " + type_obj , cLog::LOG_TYPE::L_ERROR);
		return;
	}

	GRID_TYPE typeObj=stringToType(type_obj);

	switch (typeObj) {
		case GRID_EQUATORIAL :
			Log.write("SkyGridMgr creating GRID EQUATORIAL" , cLog::LOG_TYPE::L_INFO);
			tmp=new GridEquatorial();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_ALTAZIMUTAL:
			Log.write("SkyGridMgr creating GRID ALTAZIMUTAL" , cLog::LOG_TYPE::L_INFO);
			tmp=new GridAltAzimutal();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_ECLIPTIC :
			Log.write("SkyGridMgr creating GRID ECLIPTIC" , cLog::LOG_TYPE::L_INFO);
			tmp=new GridEcliptic();
			m_map[type_obj]= tmp;
			return;
			break;

		case GRID_GALACTIC :
			Log.write("SkyGridMgr creating GRID GALACTIC" , cLog::LOG_TYPE::L_INFO);
			tmp=new GridGalactic();
			m_map[type_obj]= tmp;
			return;
			break;

		default: // grille inconnue
			Log.write("SkyGridMgr SkyGrid unknown " + type_obj , cLog::LOG_TYPE::L_ERROR);
			break;
	}
}
