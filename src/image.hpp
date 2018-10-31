/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2018 LSS Team & Immersive Adventure
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

// manage an image for display from scripts

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include <vector>
#include "shader.hpp"
#include "stateGL.hpp"

class s_texture;
class Navigator;
class Projector;

class Image {

public:
	// is the image flat on the viewport or positioned with alt-azimuthal or earth equatorial coordinates?
	enum class IMAGE_POSITIONING : char {
		POS_VIEWPORT,
		POS_HORIZONTAL,
		POS_EQUATORIAL,
		POS_J2000,
		POS_DOME
	};

	Image(Image const *n, int i);
	Image& operator = (Image const &) = delete;

	Image() = delete;
	Image(const std::string& filename, const std::string& name, IMAGE_POSITIONING pos_type, bool mipmap);
	Image(s_texture *_imgTex, const std::string& name, IMAGE_POSITIONING pos_type);
	virtual ~Image();

	void setAlpha(float alpha, float duration);
	void setScale(float scale, float duration);
	void setRotation(float rotation, float duration);
	void setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration);
	void setPersistent(bool value) {
		isPersistent = value;
	}

	//! indique si on active la transparence sur la KeyColor
	void setTransparency(bool v) {
		printf("définition de setTransparency\n");
		transparency = v;
	}

	//! KeyColor a utiliser pour la transparence
	void setKeyColor(const Vec3f&color, float intensity) {
		printf("définition de setKeyColor\n");
		noColor = Vec4f(color[0], color[1], color[2],intensity);
	}

	bool update(int delta_time);  // update properties
	void draw(const Navigator * nav, Projector * prj);

	const std::string getName() const {
		return image_name;
	};

	bool imageLoaded() {
		return (image_ratio != -1); // was texture loaded from disk?
	}

	bool imageIsPersistent() {
		return isPersistent;
	}

	static void createShaderUnified();
	static void createShaderImageViewport();
	static void deleteShaderUnified();
	static void deleteShaderImageViewport();
	
	static shaderProgram *shaderImageViewport;
	static shaderProgram *shaderUnified;

private:
	void drawViewport(const Navigator * nav, Projector * prj);
	void drawUnified(bool drawUp, const Navigator * nav, Projector * prj);

	void initialise(const std::string& name, IMAGE_POSITIONING pos_type, bool mipmap = false);
	void initCache(Projector * prj); 

	s_texture* image_tex = nullptr;
	std::string image_name;
	IMAGE_POSITIONING image_pos_type;
	bool isPersistent= false;

	float image_scale, image_alpha, image_rotation;
	float image_ratio, image_xpos, image_ypos;

	bool flag_alpha, flag_scale, flag_rotation, flag_location;
	float coef_alpha, coef_scale, coef_rotation;
	float mult_alpha, mult_scale, mult_rotation;
	float start_alpha, start_scale, start_rotation;
	float end_alpha, end_scale, end_rotation;

	float coef_location, mult_location;
	float start_xpos, start_ypos, end_xpos, end_ypos;

	//OpenGL vars
	std::vector<float> vecImgTex, vecImgPos;
	static DataGL sImage;

	//active la transparence
	bool transparency;
	// indique quelle couleur est à effacer de l'image nocolor[3] indique le delta de couleur
	Vec4f noColor;

	//données utiles au cache
	bool initialised = false;
	int vieww, viewh;

	float cx, cy, radius, prj_ratio;
	float xbase, ybase;

	Mat4d mat;
	Vec3d gridpt, onscreen;
	Vec3d imagev, ortho1 , ortho2;
	int grid_size;
	bool needFlip = false;
};


#endif // _IMAGE_H
