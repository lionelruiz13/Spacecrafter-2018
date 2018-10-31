/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _LANDSCAPE_H_
#define _LANDSCAPE_H_

#include <string>
#include <vector>
#include "vecmath.hpp"
#include "fader.hpp"
#include "utility.hpp"
#include "shader.hpp"
#include "stateGL.hpp"

class s_texture;
class ToneReproductor;
class Navigator;
class Projector;

// Class which manages the displaying of the Landscape
class Landscape {

public:
	enum LANDSCAPE_TYPE {
		LANDSCAPE,
		FISHEYE,
		SPHERICAL
	};

	Landscape(float _radius = 2.);
	virtual ~Landscape();
	Landscape(Landscape const &) = delete;
	Landscape& operator = (Landscape const &) = delete;

	virtual void load(const std::string& file_name, const std::string& section_name);

	void setSkyBrightness(float b) {
		sky_brightness = b;
	}


	//! Set the number of slices pour la construction des panoramas
	void setSlices(int a) {
		a=a-a%5;  //on veut un nombre multiple de 5
		if (a>0)
			slices = a;
	}

	//! Set the number of stacks pour la construction des panoramas
	void setStacks(int a) {
		a=a-a%5;  // on veut un nombre multiple de 5
		if (a>0)
			stacks = a;
	}

	//! Set whether landscape is displayed (does not concern fog)
	void setFlagShow(bool b);

	//! Get whether landscape is displayed (does not concern fog)
	bool getFlagShow() const {
		return (bool)land_fader;
	}
	//! Set whether fog is displayed
	void setFlagShowFog(bool b) {
		fog_fader=b;
	}

	void flipFlagShow() {
		land_fader = !land_fader;
	}

	//! Get whether fog is displayed
	bool getFlagShowFog() const {
		return (bool)fog_fader;
	}
	//! Get landscape name
	std::string getName() const {
		return name;
	}
	//! Get landscape author name
	std::string getAuthorName() const {
		return author;
	}
	//! Get landscape description
	std::string getDescription() const {
		return description;
	}

	void update(int delta_time) {
		land_fader.update(delta_time);
		fog_fader.update(delta_time);
	}
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);

	static Landscape* createFromFile(const std::string& landscape_file, const std::string& section_name);
	static Landscape* createFromHash(stringHash_t & param);
	static std::string getFileContent(const std::string& landscape_file);
	static std::string getLandscapeNames(const std::string& landscape_file);
	static std::string nameToKey(const std::string& landscape_file, const std::string & name);
protected:
	void getFogDraw(GLdouble radius, GLdouble height, GLint slices, GLint stacks, std::vector<float>* dataTex, std::vector<float>* dataPos);
	void initShaderFog();
	void drawFog(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const;
	//! Load attributes common to all landscapes
	void loadCommon(const std::string& landscape_file, const std::string& section_name);
	void initShaderParams();
	void deleteVboVoa();
	void deleteMapTex();
	float radius;
	std::string name;
	float sky_brightness;
	bool valid_landscape;   // was a landscape loaded properly?
	LinearFader land_fader;
	LinearFader fog_fader;
	std::string author;
	std::string description;
	s_texture* fog_tex;
	s_texture* map_tex;
	s_texture* map_tex_night;
	bool haveNightTex;
	float fog_alt_angle;
	float fog_angle_shift;

	unsigned int nbVertex;				// nombre de vertex des landscapes
	unsigned int nbFogVertex;			//nombre de vertex pour le fog
	static int slices;
	static int stacks;
	shaderProgram* shaderLandscape;
	shaderProgram* shaderFog;
	DataGL landscape;
	DataGL fog;
};

class LandscapeFisheye : public Landscape {
public:
	LandscapeFisheye(float _radius = 1.);
	virtual ~LandscapeFisheye();
	virtual void load(const std::string& fileName, const std::string& section_name);
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);
	void create(const std::string _name, bool _fullpath, const std::string _maptex, double _texturefov,
	            const float _rotate_z, const std::string _maptex_night, const bool _mipmap);
private:
	void getLandscapeFisheye(double radius, int slices, int stacks, double texture_fov,  GLfloat * datatex, GLfloat * datapos);
	void initShader();
	float tex_fov;
	float rotate_z; // rotation around the z axis
};


class LandscapeSpherical : public Landscape {
public:
	LandscapeSpherical(float _radius = 1.);
	virtual ~LandscapeSpherical();
	virtual void load(const std::string& fileName, const std::string& section_name);
	virtual void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav);
	void create(const std::string _name, bool _fullpath, const std::string _maptex, const float _base_altitude,
	            const float _top_altitude, const float _rotate_z, const std::string _maptex_night, const bool _mipmap);
private:
	void getLandscapeSpherical(double radius, double one_minus_oblateness, int slices, int stacks,
	                           double bottom_altitude, double top_altitude , GLfloat * datatex, GLfloat * datapos);
	void initShader();
	float base_altitude, top_altitude;  // for partial sphere coverage
	float rotate_z; // rotation around the z axis
};

#endif // _LANDSCAPE_H_
