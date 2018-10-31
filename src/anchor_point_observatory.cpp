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
#include <sstream>
#include <string>
#include "anchor_point_observatory.hpp"

using namespace std;

AnchorPointObservatory::AnchorPointObservatory() noexcept : AnchorPoint() { }

AnchorPointObservatory::AnchorPointObservatory(double x, double y, double z) :
    AnchorPoint(x,y,z) { }

AnchorPointObservatory::AnchorPointObservatory(const Vec3d& v) :
    AnchorPoint(v) { }

AnchorPointObservatory::~AnchorPointObservatory() { }

Mat4d AnchorPointObservatory::getRotLocalToEquatorial(double jd, double lat, double lon, double altitude)const noexcept{

	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;

	return Mat4d::zrotation(lon*(C_PI/180.))
			* Mat4d::yrotation((90.-lat)*(C_PI/180.));

}

string AnchorPointObservatory::saveAnchor()const noexcept{

	ostringstream os;

	os << "type point" << endl;
	os << "x " << getHeliocentricEclipticPos()[0] << endl;
	os << "y " << getHeliocentricEclipticPos()[1] << endl;
	os << "z " << getHeliocentricEclipticPos()[2] << endl;

	return os.str();

}
