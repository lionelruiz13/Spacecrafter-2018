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
//! \file trail.hpp
//! \brief Draws a body's trail
//! \author Julien LAFILLE
//! \date april 2018

#ifndef _TRAIL_HPP_
#define _TRAIL_HPP_

#include "fader.hpp"
#include <list>
#include <string>
#include "shader.hpp"
#include "stateGL.hpp"
#include <vector>
#include <list>


class Body;
class Navigator;
class Projector;
class TimeMgr;

typedef struct TrailPoint {
	Vec3d point;
	double date;
} TrailPoint;

class Trail {

public:

	Trail() = delete;
	Trail(const Trail&) = delete;

	Trail(Body * body,
	      int MaxTrail = 1460,
	      double DeltaTrail = 1,
	      double last_trailJD = 0,
	      bool trail_on = 0,
	      bool first_point = 1);
	~Trail();

	void setFlagTrail(bool b) {
		if (b == trail_fader)
			return;
		trail_fader = b;
		startTrail(b);
	}

	void drawTrail(const Navigator * nav, const Projector* prj);
	void updateTrail(const Navigator* nav, const TimeMgr* timeMgr);
	void startTrail(bool b);
	void updateShader(int delta_time);

	static void createShader();
	static void deleteShader();

private:

	Body * body = nullptr;

	static shaderProgram* shaderTrail;
	static DataGL TrailData;
	LinearFader trail_fader;

	std::vector<float> vecTrailPos;
	std::vector<float> vecTrailColor;
	std::list<TrailPoint>trail;

	int MaxTrail;
	double DeltaTrail;
	double last_trailJD;
	bool trail_on;  // accumulate trail data if true
	bool first_point;  // if need to take first point of trail still
};

#endif //_TRAIL_HPP_
