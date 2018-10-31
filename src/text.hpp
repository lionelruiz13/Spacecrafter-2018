/*
 * Spacecrafter astronomy simulation and visualization
 *
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

#ifndef _TEXT_HPP
#define _TEXT_HPP

#include "s_font.hpp"
#include "fader.hpp"
#include "projector.hpp"
#include <vector>
#include "stateGL.hpp"
#include "vecmath.hpp"


/**
 * @class Text
 * @brief Cette classe gère un unique texte à afficher à l'écran.
 * Un texte est représenté par sa couleur, son message et sa position et sa durée de vie.
 * On peut modifier sa couleur, sa position et son message.
 */
class Text {
public:
	Text(const std::string &_name, const std::string &_text, int _altitude, int _azimuth, const std::string &size, const Vec3f &color, int _timeout=0);
	~Text();

	//! occulte ou pas le text à l'affichage
	void setDisplay(bool b){
		fader=b;
	}

	//! indique l'état du text vis à vis de l'affichage
	bool getDisplay(void) const{
		return fader;
	}

	//! renvoie le nom du text
	std::string getName(){
		return name;
	}

	//! modifie le message du text
	void textUpdate(const std::string &_text){
		text=_text;
	};

	//! affiche le texte à l'écran 
	void draw(const Projector* prj, s_font *textFont[]);

	//! met à jour l'état du fader d'affichage
	void update(int delta_time);

	//! modifie la durée d'apparition/disparition d'un text
	void setFadingDuration(int a) {
		fader.setDuration(a);
	};

	//! renvoie si le texte est encore actif ou pas
	bool isDead() {
		return isTextDead;
	}

private:
	enum class FontSize : char {T_XX_SMALL, T_X_SMALL, T_SMALL, T_MEDIUM, T_LARGE, T_X_LARGE, T_XX_LARGE};
	std::string name;
	std::string text;
	Vec3f textColor;
	int altitude;
	int azimuth;
	LinearFader fader;
	FontSize textSize= FontSize::T_MEDIUM;
	void convertToFontSize(const std::string &size);
	int timeout = 0; 		//!< durée d'affichage du texte
	bool isTextDead = false;	//!< indicateur de vie du texte
	bool isDying = false;
};

#endif // TEXT_HPP
