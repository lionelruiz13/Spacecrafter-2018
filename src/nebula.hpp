/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
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

#ifndef _NEBULA_H_
#define _NEBULA_H_

#include "object_base.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "s_texture.hpp"
#include "s_font.hpp"
#include "tone_reproductor.hpp"
#include "translator.hpp"
#include <vector>

/**
 * \brief     Type of deepSkyObject
 * \details   list all deepSkyObject supported.
 */
typedef enum nebula_type {

	GALXY,	//!< galaxy
	OPNCL,	//!< open cluster
	GLOCL,	//!< globular cluster
	BRTNB,	//!< bright nebula
	PLNNB,	//!< planetary nebula
	DRKNB,	//!< dark nebula
	CLNEB,	//!< cluster and nebula
	STARS,	//!< particular star
	GALCL,	//!< galaxies cluster
	QUASR,	//!< quazar
	SNREM,	//!< supernova remnant
	GENRC 	//!< Generic
} nebula_type;

class Nebula : public ObjectBase {
	friend class NebulaMgr;
public:
	Nebula(std::string _englishName, std::string _DSOType, std::string _constellation, float _ra, float _de, float _mag, float _size, std::string _classe,
	       float _distance, std::string tex_name, bool path, float tex_angular_size, float _rotation, std::string _credit, float _luminance, bool _deleteable, bool _hidden);
	~Nebula();

	//! display pretty information from the deepskyObject
	std::string getInfoString(const Navigator * nav) const;

	//! return the position ra, de from the deepskyObject
	void getRaDeValue(const Navigator *nav ,double *ra, double *de) const {
	}

	std::string getShortInfoString(const Navigator * nav = nullptr) const;

	std::string getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const;

	OBJECT_TYPE getType(void) const {
		return OBJECT_NEBULA;
	}

	Vec3d getEarthEquPos(const Navigator *nav) const {
		return nav->j2000ToEarthEqu(XYZ);
	}

	//! return the observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const {
		return XYZ;
	}
	double getCloseFov(const Navigator * nav = nullptr) const;

	//! Return the apparent magnitude for DSO object
	float getMag(const Navigator * nav = nullptr) const {
		return mag;
	}

	void setLabelColor(const Vec3f& v) const {
		labelColor = v;
	}

	//! Set the commun picto color for DSO
	void setCircleColor(const Vec3f& v) const {
		circleColor = v;
	}
	//! Return commun picto color for DSO
	const Vec3f &getCircleColor() const {
		return Nebula::circleColor;
	}

	//! return the translated name of DSO
	std::string getNameI18n(void) const {
		return nameI18;
	}

	//! return the Name of the DSO
	std::string getEnglishName(void) const {
		return englishName;
	}

	//! @brief Get the printable nebula Type
	//! @return an string of the nebula type code.
	std::string getTypeToString(void) const;

	void translateName(Translator&);

	//! return if the DSO can be deleted or not:
	//! return if allowed to delete from script
	bool isDeletable() const {
		return m_deletable;
	}

	//! return the constellation where the DSo is located.
	std::string getConstellation(void) const {
		return constellation;
	}

	//! return the type of DSO in string format
	std::string getStringType(void) const {
		return DSOstringType;
	}

protected:
	//! Return the radius of a circle containing the object on screen
	float getOnScreenSize(const Projector* prj, const Navigator * nav = nullptr, bool orb_only = false) {
		return m_angular_size * (180./C_PI) * (prj->getViewportHeight()/prj->getFov());
	}

	//! hide a deepskyObject
	void hide() {
		m_hidden = true;
	}

	//! unhide a deepskyObject
	void show() {
		m_hidden = false;
	}

	//! select a deepskyObject for display
	void select() {
		m_selected = true;
	}

	//! unselect a deepskyObject for display
	void unselect() {
		m_selected = false;
	}

private:
	void drawTex(const Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness);
	void drawName(const Projector* prj);
	void drawHint(const Projector* prj, const Navigator * nav, std::vector<float> &vecHintPos, std::vector<float> &vecHintTex, std::vector<float> &vecHintColor);
	nebula_type getDsoType( std::string type);

	std::string englishName;		// English name
	std::string nameI18;			// translated englishName
	std::string constellation;		// who is it ?
	std::string credit;				// Nebula image credit
	std::string DSOclass;			// string object class
	std::string DSOstringType;		// DSOType in string format
	float mag;						// Apparent magnitude for object


	float texLuminanceAdjust;		// draw texture luminance adjustment for overexposed images
	float m_angular_size;			// Angular size in radians
	Vec3f XYZ;						// Cartesian equatorial position
	Vec3d XY;						// Store temporary 2D position
	nebula_type DSOType;			// say what type of nebula it is

	s_texture * neb_tex = nullptr;	// Texture
	float sDataTex[8];				// The 8 indices for the 4 vertex
	std::vector<float> sDataPos;	//all coordonates points for the 4 vertex
	float luminance;				// Object luminance to use (value computed to compensate the texture avergae luminosity)
	float tex_avg_luminance;        // avg luminance of the texture (saved here for performance)

	float myRA, myDe; 				// in radians
	float texAngularSize; 			// angular texture size in radians
	float myRotation; 				// rotation in radians
	double myDistance; 				// in light years

	bool m_deletable;				// whether a script added this nebula
	bool m_hidden;  				// whether hidden from being visible
	bool m_selected;				// indique si la nébuleuse est selectionnée pour affichage ou pas.

	Vec3f neb_color;				// particular color for the DSO

	Vec2f posTex;

	s_texture * tex_circle;			// The symbolic circle texture

	static s_texture * tex_NEBULA;

	static shaderProgram * shaderNebulaTex;
	static DataGL nebulaTex;

	static s_font* nebulaFont;			// Font used for names printing
	static float hintsBrightness;
	static float nebulaBrightness;
	static float textBrightness;
	static Vec3f labelColor;
	static Vec3f circleColor;
	static float circleScale;			// Define the sclaing of the hints circle
	static bool flagBright;				// Define if nebulae must be drawn in bright mode
	static bool displaySpecificHint;	// Define if specific or generic Hints are to be displayed
	static float dsoRadius;				// Define commun scale radius for DSO
	static int dsoPictoSize;			// Define the size/2 from picto tex
};

#endif // _NEBULA_H_
