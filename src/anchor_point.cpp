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
#include "anchor_point.hpp"
#include "body.hpp"

using namespace std;

AnchorPoint::AnchorPoint()noexcept
{
	heliocentric_ecliptic_pos = v3dNull;
}


AnchorPoint::AnchorPoint(double x, double y, double z) noexcept
{
	heliocentric_ecliptic_pos = Vec3d(x,y,z);
}

AnchorPoint::AnchorPoint(const Vec3d& pos)noexcept
{
	heliocentric_ecliptic_pos = pos;
}

Mat4d AnchorPoint::getRotLocalToEquatorial(double jd, double lat, double lon, double alt) const noexcept
{
	return rotLocalToEquatorial;
}

Mat4d AnchorPoint::getRotEquatorialToVsop87() const noexcept
{
	return rotEquatorialToVsop87;
}

void AnchorPoint::update()noexcept
{
	//nothing to do here
}

bool AnchorPoint::isOnBody(const Body * body) const noexcept
{
	return false;
}

bool AnchorPoint::isOnBody() const noexcept
{
	return false;
}

const Body* AnchorPoint::getBody() const noexcept
{
	return nullptr;
}

std::string AnchorPoint::saveAnchor()const noexcept
{

	ostringstream os;

	os << "type point" << endl;
	os << "x " << heliocentric_ecliptic_pos[0] << endl;
	os << "y " << heliocentric_ecliptic_pos[1] << endl;
	os << "z " << heliocentric_ecliptic_pos[2] << endl;

	return os.str();

}
