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

#ifndef __SKY_DRAW_HPP__
#define __SKY_DRAW_HPP__

#include <string>
#include <fstream>
#include "projector.hpp"
#include "navigator.hpp"
#include "fader.hpp"
#include <vector>
#include "shader.hpp"
#include "stateGL.hpp"


//! Class which manages a personal line to display around the sky
class SkyDraw {
public:

	SkyDraw();
	~SkyDraw();

	//!	void draw(const Projector* prj) const; 20060825 patch
	void draw() const;

	void setColor(const Vec3f& c) {
		color = c;
	}

	const Vec3f& getColor() {
		return color;
	}

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

	void aleaPoints(int nbr);

	void addPoints(std::string alphaList);

	void clear();

protected:
	Vec3f color;
	LinearFader fader;
private:
	void createShader();
	void deleteShader();
	void addPoint(float a, float b);
	void update_data();
	std::vector<float> pointsPos;
	std::vector<float> pointsColors;
	shaderProgram* shaderDraw;
	DataGL dataPts;
};

#endif // __SKY_DRAW_HPP__
