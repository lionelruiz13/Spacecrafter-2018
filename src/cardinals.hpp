/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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

#ifndef __CARDINALS_H__
#define __CARDINALS_H__

#include <string>
#include <fstream>
#include "fader.hpp"
#include "vecmath.hpp"

class Projector;
class s_font;
class Translator;

//! Class which manages the cardinal points displaying
class Cardinals {
public:
	Cardinals(float _radius = 1.);
	virtual ~Cardinals();
	Cardinals(Cardinals const &) = delete;
	Cardinals& operator = (Cardinals const &) = delete;

	void draw(const Projector* prj, double latitude, bool gravityON = false) const;
	void setColor(const Vec3f& c) {
		color = c;
	}
	Vec3f getColor() {
		return color;
	}
	void setFont(float font_size, const std::string& font_name);
	void translateLabels(Translator& trans);  // for i18n
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	void setFlagShow(bool b) {
		fader = b;
	}
	bool getFlagShow(void) const {
		return fader;
	}

	void flipFlagShow() {
		fader = !fader;
	}

	void setInternalNav (bool a) {
		internalNav=a;
	}

private:
	float radius;
	s_font* font;
	Vec3f color;
	std::string sNorth, sSouth, sEast, sWest;
	LinearFader fader;
	bool internalNav;
};


#endif // __CARDINALS_H__
