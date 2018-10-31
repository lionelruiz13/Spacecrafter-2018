/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _CONSTELLATION_H_
#define _CONSTELLATION_H_

#include "object_base.hpp"
#include "object_type.hpp"
#include "object.hpp"
#include "utility.hpp"
#include "fader.hpp"
#include <vector>

class HipStarMgr;
class s_font;

class Constellation : public ObjectBase {
	friend class ConstellationMgr;

public:
	Constellation();
	~Constellation();

	//! Write I18n information about the object.
	std::string getInfoString(const Navigator * nav) const {
		return getNameI18n() + "(" + getShortName() + ")";
	}

	void getRaDeValue(const Navigator *nav,double *ra, double *de) const {
	}

	//! The returned string can typically be used for object labeling in the sky
	std::string getShortInfoString(const Navigator * nav) const {
		return getNameI18n();
	}

	//! Nav string
	std::string getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const {
		return " ";
	}

	//! Return object's type
	OBJECT_TYPE getType(void) const {
		return OBJECT_CONSTELLATION;
	}
	//! Get position in earth equatorial frame
	Vec3d getEarthEquPos(const Navigator *nav) const {
		return XYZname;
	}
	//! observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const {
		return XYZname;
	}
	//! Return object's magnitude
	float getMag(const Navigator * nav) const {
		return 0.;
	}

	bool read(const std::string& record, HipStarMgr * _VouteCeleste);

	const Constellation* isStarIn(const Object&) const;

	ObjectBaseP getBrightestStarInConstellation(void) const;

	//! Return translated name in UTF8 string
	std::string getNameI18n(void) const {
		return nameI18;
	}
	std::string getEnglishName(void) const {
		return abbreviation;
	}
	std::string getShortName(void) const {
		return abbreviation;
	}

	void drawName(s_font * constfont,const  Projector* prj) const;
	void drawBoundary(const Projector* prj, std::vector<float> &vBoundariesPos, std::vector<float> &vBoundariesIntensity);
	void drawLines(const Projector* prj, std::vector<float> &vLinesPos, std::vector<float> &vLinesColor);
	void drawArt(const Projector* prj, const Navigator* nav, shaderProgram* &shaderArt, const DataGL &constellation);

	void update(int delta_time);

	void setFlagLines(bool b) {
		line_fader=b;
	}
	void setFlagBoundaries(bool b) {
		boundary_fader=b;
	}
	void setFlagName(bool b) {
		name_fader=b;
	}
	void setFlagArt(bool b) {
		art_fader=b;
	}
	bool getFlagLines(void) const {
		return line_fader;
	}
	bool getFlagBoundaries(void) const {
		return boundary_fader;
	}
	bool getFlagName(void) const {
		return name_fader;
	}
	bool getFlagArt(void) const {
		return art_fader;
	}

	void setLineColor(const Vec3f& c) {
		lineColor = c;
	}

	void setLabelColor(const Vec3f& c) {
		labelColor = c;
	}

private:
	//! Translated name in UTF8 format (translated using gettext)
	std::string nameI18;

	/** Name in english */
	std::string englishName;

	/** Abbreviation (of the latin name for western constellations) */
	std::string abbreviation;

	/** Direction vector pointing on constellation name drawing position */
	Vec3f XYZname;
	Vec3d XYname;

	/** Number of segments in the lines */
	unsigned int nb_segments;

	/** List of stars forming the segments */
	ObjectBaseP* asterism;

	s_texture* art_tex;
	Vec3d art_vertex[9];

	/** Define whether art, lines, names and boundary must be drawn */
	LinearFader art_fader, line_fader, name_fader, boundary_fader;

	std::vector<std::vector<Vec3f> *> isolatedBoundarySegments;
	std::vector<std::vector<Vec3f> *> sharedBoundarySegments;

	Vec3f lineColor;
	Vec3f labelColor;
	static Vec3f artColor;
	static bool singleSelected;

	std::vector<float> vecPos;
	std::vector<float> vecTex;
};

#endif // _CONSTELLATION_H_
