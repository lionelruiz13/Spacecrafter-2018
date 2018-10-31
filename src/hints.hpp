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


#ifndef _HINTS_HPP_
#define _HINTS_HPP_

//! \file hints.hpp
//! \brief Draws a body's name and a circle arround it
//! \author Julien LAFILLE
//! \date april 2018

#include "fader.hpp"
#include "shader.hpp"
#include "stateGL.hpp"
#include <vector>


class Body;
class Navigator;
class Projector;


class Hints {
public :

	Hints() = delete;
	Hints(const Hints&)=delete;
	Hints(Body * body);

	void setFlagHints(bool b) {
		hint_fader = b;
	}

	void drawHints(const Navigator* nav, const Projector* prj);

	void drawHintCircle(const Navigator* nav, const Projector* prj);

	void updateShader(double delta_time);

	void computeHints();

	static void createShader();
	static void deleteShader();

private :
	static const int nbrFacets;
	static const int hintCircleRadius;

	Body * body;

	static shaderProgram* shaderHints;
	static DataGL HintsData;
	LinearFader hint_fader;

	std::vector<float> vecHintsPos;
};

#endif // _HINTS_HPP_
