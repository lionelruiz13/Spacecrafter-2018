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


#ifndef SKYLINE_MGR_HPP
#define SKYLINE_MGR_HPP

#include <map>
#include <string>

#include "skyline.hpp"
#include "s_font.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "fader.hpp"
#include "time_mgr.hpp"

class SkyLineMgr {
public:
	SkyLineMgr();
	~SkyLineMgr();
	SkyLineMgr(SkyLineMgr const &) = delete;
	SkyLineMgr& operator = (SkyLineMgr const &) = delete;

	int size() {
		return m_map.size();
	};
	//Celle qui va créer les objets
	void Create(std::string type_obj);
	void draw(const Projector* prj, const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);
	void update(int delta_time);

	void setFont(float font_size, const std::string& font_name);

	void setInternalNav(bool a);

	void setColor(std::string typeObj, const Vec3f& c);

	const Vec3f& getColor(std::string typeObj);

	//! change FlagShow: inverse la valeur du flag
	void flipFlagShow(std::string typeObj);

	void setFlagShow(std::string typeObj, bool b);

	bool getFlagShow(std::string typeObj);

	void translateLabels(Translator& trans);

	bool isExist(std::string typeObj);

private:
	enum LINE_TYPE {
		LINE_CIRCLE_POLAR,
		LINE_ECLIPTIC_POLE,
		LINE_GALACTIC_POLE,
		LINE_ANALEMMA,
		LINE_ANALEMMALINE,
		LINE_CIRCUMPOLAR,
		LINE_GALACTIC_CENTER,
		LINE_VERNAL,
		LINE_GREENWICH,
		LINE_ARIES,
		LINE_EQUATOR,
		LINE_GALACTIC_EQUATOR,
		LINE_MERIDIAN,
		LINE_TROPIC,
		LINE_ECLIPTIC,
		LINE_PRECESSION,
		LINE_VERTICAL,
		LINE_ZODIAC,
		LINE_ZENITH,
		LINE_UNKNOWN
	};

	SkyLineMgr::LINE_TYPE stringToType(std::string typeObj);
	std::map<std::string ,SkyLine*> m_map;

	Vec3f baseColor;
};
#endif
