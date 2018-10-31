/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/


#ifndef __VP_HPP__
#define __VP_HPP__

#include "vecmath.hpp"
#include "shader.hpp"
#include "stateGL.hpp"
#include "fader.hpp"

#define VP_FADER_DURATION 3000

class ViewPort {
public:
	ViewPort();
	~ViewPort();
	//! créer le shader
	void createShader();
	//! détruit le shader
	void deleteShader();
	//! trace une texture sur le viewport
	void draw();

	//! indique quelle id de texture (dans la CG) ViewPort utilisera pour affichage
	//! \param tex, id de texture dans la CG
	void setTexture(GLuint tex) {
		texture = tex;
	}

	//! indique si la classe doit etre active ou pas
	void display(bool alive) {
		isAlive = alive;
		if (alive) {
			fader=true;
		}
		else {
			fader=false;
			fader.update(VP_FADER_DURATION);
		}
	}

	//! indique à la classe de se remettre en position de départ
	void displayStop() {
		isAlive = false;
		fader=false;
		fader.update(VP_FADER_DURATION);
		transparency = false;
		noColor = Vec4f::null();
	}

	//! update le fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! indique si on active la transparence sur la KeyColor
	void setTransparency(bool v) {
		transparency = v;
	}

	//! KeyColor a utiliser pour la transparence
	void setKeyColor(const Vec3f&color, float intensity) {
		noColor = Vec4f(color[0], color[1], color[2],intensity);
	}

private:
	//initialisation shader
	void initParam();
	// shader
	shaderProgram* shaderViewPort;
	// Données openGL
	DataGL viewport;
	// indique quelle texture est utilisée pour affichage
	GLuint texture;
	// active la classe
	bool isAlive;
	//active la transparence
	bool transparency = false;
	// indique quelle couleur est à effacer de l'image
	// nocolor[3] indique le delta de couleur
	Vec4f noColor=Vec4f::null();
	ParabolicFader fader;
};

#endif // __VP_HPP__
