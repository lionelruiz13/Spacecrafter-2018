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
//! \file orbit_2d.hpp
//! \brief Draws a body's orbit
//! \author Julien LAFILLE
//! \date april 2018

#ifndef ORBIT2D_HPP
#define ORBIT2D_HPP

#include "orbit_plot.hpp"
#include "fader.hpp"
#include "shader.hpp"
#include "stateGL.hpp"
#include <vector>

class Body;
class Projector;
class Navigator;

class Orbit2D : public OrbitPlot {
public:

	Orbit2D()=delete;
	Orbit2D(const Orbit2D&) = delete;
	Orbit2D(Body* body, int segments = 320);

	~Orbit2D(){
		vecOrbit2dVertex.clear();
	};

	void drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat);
	
	void computeShader();

private:

	std::vector<float> vecOrbit2dVertex;

};

#endif
