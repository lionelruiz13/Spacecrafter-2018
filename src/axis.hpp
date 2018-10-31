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
//! \file axis.hpp
//! \brief Draws a body's axis of rotation
//! \author Julien LAFILLE
//! \date april 2018

#ifndef _AXIS_HPP_
#define _AXIS_HPP_

#include "fader.hpp"
#include "shader.hpp"
#include "stateGL.hpp"
#include <vector>

class Body;
class Projector;

class Axis {
public:

	Axis() = delete;
	Axis(const Axis&)=delete;
	Axis(Body * body);

	void setFlagAxis(bool b) {
		drawaxis = b;
	}

	void drawAxis(const Projector* prj, const Mat4d& mat);

	void computeAxis(const Projector* prj, const Mat4d& mat);

	static void createShader();
	static void deleteShader();

private :

	Body * body;

	static shaderProgram* shaderAxis;
	static DataGL AxisData;

	std::vector<float> vecAxisPos;
	bool drawaxis = false;  // display or not Body axis

};

#endif // _AXIS_HPP_

