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

#ifndef __SKYLINE_H__
#define __SKYLINE_H__

#include <string>
#include <fstream>
#include "s_font.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "tone_reproductor.hpp"
#include "fader.hpp"
#include "translator.hpp"
#include "time_mgr.hpp"

//! Class which manages a line to display around the sky like the ecliptic line
class SkyLine {
	friend class SkyLineMgr;


public:
	//! Create and precompute positions of a SkyGrid
	SkyLine( double _radius = 1., unsigned int _nb_segment = 48);
	virtual ~SkyLine();
	//!	void draw(const Projector* prj) const; 20060825 patch
	virtual void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory) = 0;
	void setColor(const Vec3f& c) {
		color = c;

	}
	const Vec3f& getColor() {
		return color;
	}
	void translateLabels(Translator& trans);  // for i18n
	void update(int delta_time) {
		fader.update(delta_time);
	}
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	void setFlagshow(bool b) {
		fader = b;
	}
	bool getFlagshow(void) const {
		return fader;
	}
	void setFont(float font_size, const std::string& font_name);

	void setInternalNav (bool a) {
		internalNav=a;
	}

	void flipFlagShow() {
		fader = ! fader;
	}


protected:
	static void createShader();
	static void deleteShader();

	double radius;
	unsigned int nb_segment;
	Vec3f color;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	LinearFader fader;
	s_font * font;
	bool internalNav;
	mutable Vec3d pt1;
	mutable Vec3d pt2;
	std::string month[13]; // labels for translating on ecliptic

	//variables utilitaires
	Vec4f tmp;
	Mat4f TRANSFO; //a renommer

	//Opengl
	static shaderProgram* shaderSkylineDraw;
	static shaderProgram* shaderTropicDrawTick;
	static shaderProgram* shaderSkylineMVPDraw;

	static DataGL skylineDraw;

	std::vector<float> vecDrawPos;
	std::vector<float> vecDrawMVPPos;
};

//--------------------------------------------------------------------------

class SkyLine_Pole : public SkyLine {
public:
	enum SKY_LINE_POLE_TYPE {
		POLE,
		ECLIPTIC_POLE,
		GALACTIC_POLE
	};
	//! Create and precompute positions of a SkyGrid
	SkyLine_Pole(SKY_LINE_POLE_TYPE _line_pole_type, double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Pole();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	SKY_LINE_POLE_TYPE line_pole_type;
	mutable Vec3f circlep[51];
};

//--------------------------------------------------------------------------

class SkyLine_Zodiac : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Zodiac(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Zodiac();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f punts[49];
	std::string zod[13];
	mutable double inclination;
	mutable double derivation;
	mutable double inclinaison;
	mutable float alpha,delta;
};

//--------------------------------------------------------------------------

class SkyLine_CircumPolar : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_CircumPolar(double _radius, unsigned int _nb_segment);
	virtual ~SkyLine_CircumPolar();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable double inclination;
	mutable Vec3f* points;
	mutable Vec3f* punts;
	mutable double angle;
};

//--------------------------------------------------------------------------

class SkyLine_Analemme : public SkyLine {
public:
	enum SKY_LINE_ANALEMME_TYPE {
		ANALEMMA,
		ANALEMMALINE
	};
	//! Create and precompute positions of a SkyGrid
	SkyLine_Analemme(SKY_LINE_ANALEMME_TYPE _line_analemme_type, double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Analemme();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	SKY_LINE_ANALEMME_TYPE line_analemme_type;
	mutable Vec3f analemma[93];
	float ana_ad[93];
	float ana_de[93];
	mutable double jd;
	mutable double lati;
	mutable double T;
};

//--------------------------------------------------------------------------

class SkyLine_Galactic_Center : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Galactic_Center( double _radius , unsigned int _nb_segment);
	virtual ~SkyLine_Galactic_Center();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable float alpha,delta;
	mutable double derivation;
	mutable double inclination;
	mutable Vec3f punts[51];
};

//--------------------------------------------------------------------------

class SkyLine_Vernal : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Vernal(double _radius , unsigned int _nb_segment);
	virtual ~SkyLine_Vernal();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable float alpha,delta;
	mutable double derivation;
	mutable double inclination;
	mutable Vec3f punts[51];
};

//--------------------------------------------------------------------------

class SkyLine_Greenwich : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Greenwich(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Greenwich();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f punts[60];
	mutable Vec3f punt[2];
	mutable double latitude;
	mutable double inclination;
};

//--------------------------------------------------------------------------

class SkyLine_Aries : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Aries(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Aries();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f punts[60];
	mutable Vec3f punt[2];
	mutable double latitude;
	mutable double inclination;
};

//--------------------------------------------------------------------------

class SkyLine_Equator : public SkyLine {
public:
	enum SKY_LINE_EQUATOR_LINE {
		EQUATOR,
		GALACTIC_EQUATOR
	};
	//! Create and precompute positions of a SkyGrid
	SkyLine_Equator(SKY_LINE_EQUATOR_LINE _line_equator_type, double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Equator();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	SKY_LINE_EQUATOR_LINE line_equator_type;
	mutable Vec3f* points;
	mutable double inclination;
};

//--------------------------------------------------------------------------

class SkyLine_Meridian : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Meridian(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Meridian();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f* points;
	mutable double inclination;
};
//--------------------------------------------------------------------------

class SkyLine_Tropic : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Tropic(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Tropic();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f* points;
	mutable double inclination;
};


//--------------------------------------------------------------------------

class SkyLine_Ecliptic : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Ecliptic(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Ecliptic();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Mat4d m;
	mutable bool draw_labels;
	mutable double inclination;
};


//--------------------------------------------------------------------------

class SkyLine_Precession : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Precession(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Precession();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Mat4d m;
	mutable bool draw_labels;

};


//--------------------------------------------------------------------------

class SkyLine_Vertical : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Vertical(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Vertical();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f* circlep;

};


//--------------------------------------------------------------------------

class SkyLine_Zenith : public SkyLine {
public:
	//! Create and precompute positions of a SkyGrid
	SkyLine_Zenith(double _radius , unsigned int _nb_segment );
	virtual ~SkyLine_Zenith();
	void draw(const Projector *prj,const Navigator *nav, const TimeMgr* timeMgr, const Observer* observatory);

private:
	mutable Vec3f circlep[51];
	mutable Vec3f circlen[51];
	mutable Vec3f punts[3];
};

#endif // __SKYLINE_H__
