/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 * Copyright (C) 2017 Immersive Adventure
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

#include <iostream>
#include <iomanip>

#include "body.hpp"
#include "observer.hpp"
#include "projector.hpp"
#include "s_font.hpp"
#include "planetsephems/sideral_time.h"
#include "log.hpp"
#include "fmath.hpp"
#include <chrono>
#include "ring.hpp"
#include "stateGL.hpp"
#include "file_path.hpp"
#include "trail.hpp"
#include "hints.hpp"
#include "axis.hpp"
#include "halo.hpp"
#include "orbit_plot.hpp"
// #include "atmosphere_ext.hpp"
#include "s_font.hpp"
#include "navigator.hpp"
#include "projector.hpp"
#include "translator.hpp"
#include "tone_reproductor.hpp"
#include "observer.hpp"
#include "body_color.hpp"
#include "time_mgr.hpp"

using namespace std;

s_font* Body::planet_name_font = nullptr;
float Body::object_scale = 1.f;
float Body::object_size_limit = 9;
LinearFader Body::flagClouds;
s_texture *Body::defaultTexMap = nullptr;
s_texture *Body::tex_eclipse_map = nullptr;

AtmosphereParams *Body::defaultAtmosphereParams = nullptr;

shaderProgram* Body::shaderBump=nullptr;
shaderProgram* Body::shaderNight=nullptr;
shaderProgram* Body::shaderRinged=nullptr;
shaderProgram* Body::shaderNormal=nullptr;
shaderProgram* Body::shaderMoonNormal=nullptr;
shaderProgram* Body::shaderMoonBump=nullptr;
shaderProgram* Body::shaderArtificial=nullptr;

Body::Body(Body *parent,
           const string& englishName,
           BODY_TYPE _typePlanet,
           bool _flagHalo,
           double _radius,
           double oblateness,
           BodyColor* _myColor,
           float _sol_local_day,
           float _albedo,
           const string& tex_map_name,
           Orbit *orbit,
           bool close_orbit,
           ObjL* _currentObj,
           //~ bool _hidden,
           //~ bool _deleteable,
           double orbit_bounding_radius,
           const string& tex_norm_name):


	englishName(englishName), /*flagHalo(_flagHalo),*/
	/*radius(radius),*/ initialRadius(_radius), one_minus_oblateness(1.0-oblateness),
	albedo(_albedo), axis_rotation(0.),
	tex_map(nullptr), tex_norm(nullptr), eye_sun(0.0f, 0.0f, 0.0f),
	lastJD(J2000), /*last_orbitJD(0),*/ deltaJD(JD_SECOND/4), /*orbit_cached(0),*/ orbit(orbit),
	parent(parent), close_orbit(close_orbit), //hidden(_hidden), //deleteable(_deleteable),
	is_satellite(0), orbit_bounding_radius(orbit_bounding_radius),
	boundingRadius(-1), sun_half_angle(0.0)
{
	radius = _radius;

	myColor = _myColor;
	orbit_position = v3fNull;
	sol_local_day = _sol_local_day;
	typePlanet = _typePlanet;
	//~ hiddenInitial = hidden;
	initialScale= 1.0;
	if (parent) {
		parent->satellites.push_back(this);
		if (parent->getEnglishName() != "Sun") is_satellite = 1; // quicker lookup
	}
	ecliptic_pos= v3dNull;
	rot_local_to_parent = Mat4d::identity();
	rot_local_to_parent_unprecessed = Mat4d::identity();
	mat_local_to_parent = Mat4d::identity();

	//fix graphical bug
	if (_radius==0.0)
		radius = 1/AU;

	if(tex_map_name == "") {
		tex_map = defaultTexMap;
	} else {
		tex_map = new s_texture(FilePath(tex_map_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
	}

	if (tex_norm_name != "") {  //preparation au bump shader
		tex_norm = new s_texture(FilePath(tex_norm_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
	}

	nameI18 = englishName;

	atmosphereParams = defaultAtmosphereParams;

	visibilityFader = true;

	currentObj = _currentObj; //modelSpheroide->select(model_name);

	flags.flag_axis =false;
	flags.flag_trail =false;
	flags.flag_hints =false;
	flags.flag_orbit =false;
	flags.flag_halo =_flagHalo;

	hints = new Hints(this);
	axis = new Axis(this);
	halo = new Halo(this);
}

void Body::setAtmExt(double radiusFactor, const std::string &gradient)
{
		// cout << "Atmosphere created for : " << this->getEnglishName() << endl;
		// atmExt = new AtmosphereExt(currentObj, radiusFactor, gradient);
		//~ atmExt->setPlanetRadius(1.0f);
		//~ atmExt->setSunPos(Vec3f(0.0f,0.0f,0.0f));
		//~ atmExt->setAtmAlphaScale(1.0f);
}

Body::~Body()
{
	if (tex_map && tex_map != defaultTexMap) delete tex_map;
	tex_map = nullptr;
	if (tex_norm) delete tex_norm;
	tex_norm = nullptr;
	if(orbit) delete orbit;
	orbit = nullptr;
	if (hints) delete hints;
	hints = nullptr;
	if (axis) delete axis;
	axis = nullptr;
	if (halo) delete halo;
	halo = nullptr;
	// if (atmExt) delete atmExt;
	// atmExt = nullptr;
	if (myColor) delete myColor;
	myColor = nullptr;
}

void Body::setFlagHints(bool b)
{
	hints->setFlagHints(b);
	flags.flag_hints = b;
}

bool Body::getFlagHints(void) const
{
	return flags.flag_hints;
}

bool Body::getFlagAxis(void) const
{
	return flags.flag_axis;
}

void Body::setFlagAxis(bool b)
{
	axis->setFlagAxis(b);
	flags.flag_axis = b;
}

void Body::setFlagTrail(bool b)
{

	if(trail!= nullptr) {
		trail->setFlagTrail(b);
		flags.flag_trail = b;
	}
}

bool Body::getFlagTrail(void) const
{
	if(trail!=nullptr)
		return flags.flag_trail;
	return false;
}

void Body::setFlagHalo(bool b)
{
	//~ halo->setFlagHalo(b);
	flags.flag_halo = b;
}

bool Body::getFlagHalo(void) const
{
	return flags.flag_halo;
}


bool Body::setTexHaloMap(const std::string &texMap)
{
	return Halo::setTexHaloMap(texMap);
}


void Body::setFlagOrbit(bool b)
{
	if(orbitPlot != nullptr) {
		orbitPlot->setFlagOrbit(b);
		flags.flag_orbit = b;
	}
}

bool Body::getFlagOrbit(void)const
{
	return flags.flag_orbit;
}

// fixe une couleur
void Body::setColor(const std::string& colorName,  const Vec3f& oc) {
	myColor->set(colorName, oc);
}

// récupère une couleur d'un paramètre
const Vec3f Body::getColor(const std::string& colorName) {
	return myColor->get(colorName);
}

void Body::deleteDefaultTexMap()
{
	if (defaultTexMap !=nullptr) delete defaultTexMap;
	defaultTexMap = nullptr;
	if (tex_eclipse_map !=nullptr) delete tex_eclipse_map;
	tex_eclipse_map = nullptr;
	Halo::deleteDefaultTexMap();
}

void Body::createShader()
{
	shaderNight = new shaderProgram();
	shaderNight->init( "body_night.vert", "body_night.frag");
	shaderNight->setUniformLocation("Clouds");
	shaderNight->setUniformLocation("CloudNormalTexture");
	shaderNight->setUniformLocation("CloudTexture");

	//commum
	shaderNight->setUniformLocation("planetRadius");
	shaderNight->setUniformLocation("planetScaledRadius");
	shaderNight->setUniformLocation("planetOneMinusOblateness");

	shaderNight->setUniformLocation("SunHalfAngle");
	shaderNight->setUniformLocation("LightPosition");
	shaderNight->setUniformLocation("ModelViewProjectionMatrix");
	shaderNight->setUniformLocation("ModelViewMatrix");
	shaderNight->setUniformLocation("NormalMatrix");
	shaderNight->setUniformLocation("ViewProjection");
	shaderNight->setUniformLocation("Model");

	shaderNight->setUniformLocation("MoonPosition1");
	shaderNight->setUniformLocation("MoonRadius1");
	shaderNight->setUniformLocation("MoonPosition2");
	shaderNight->setUniformLocation("MoonRadius2");
	shaderNight->setUniformLocation("MoonPosition3");
	shaderNight->setUniformLocation("MoonRadius3");
	shaderNight->setUniformLocation("MoonPosition4");
	shaderNight->setUniformLocation("MoonRadius4");

	//fisheye
	shaderNight->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderNight->setUniformLocation("clipping_fov");

	shaderBump = new shaderProgram();
	shaderBump->init( "body_bump.vert","", "" ,"", "body_bump.frag");
	shaderBump->setUniformLocation("UmbraColor");

	//commum
	shaderBump->setUniformLocation("planetRadius");
	shaderBump->setUniformLocation("planetScaledRadius");
	shaderBump->setUniformLocation("planetOneMinusOblateness");

	shaderBump->setUniformLocation("SunHalfAngle");
	shaderBump->setUniformLocation("LightPosition");
	shaderBump->setUniformLocation("ModelViewProjectionMatrix");
	shaderBump->setUniformLocation("ModelViewMatrix");
	shaderBump->setUniformLocation("NormalMatrix");

	shaderBump->setUniformLocation("MoonPosition1");
	shaderBump->setUniformLocation("MoonRadius1");
	shaderBump->setUniformLocation("MoonPosition2");
	shaderBump->setUniformLocation("MoonRadius2");
	shaderBump->setUniformLocation("MoonPosition3");
	shaderBump->setUniformLocation("MoonRadius3");
	shaderBump->setUniformLocation("MoonPosition4");
	shaderBump->setUniformLocation("MoonRadius4");

	//fisheye
	shaderBump->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderBump->setUniformLocation("clipping_fov");
	shaderRinged = new shaderProgram();
	shaderRinged->init( "body_ringed.vert", "body_ringed.frag");
	//~ shaderRinged->setUniformLocation("Ambient");
	shaderRinged->setUniformLocation("RingInnerRadius");
	shaderRinged->setUniformLocation("RingOuterRadius");
	shaderRinged->setUniformLocation("UmbraColor");

	//commum
	shaderRinged->setUniformLocation("planetRadius");
	shaderRinged->setUniformLocation("planetScaledRadius");
	shaderRinged->setUniformLocation("planetOneMinusOblateness");

	shaderRinged->setUniformLocation("SunHalfAngle");
	shaderRinged->setUniformLocation("LightPosition");
	shaderRinged->setUniformLocation("ModelViewProjectionMatrix");
	shaderRinged->setUniformLocation("ModelViewMatrix");
	shaderRinged->setUniformLocation("ModelViewMatrixInverse");
	shaderRinged->setUniformLocation("NormalMatrix");

	shaderRinged->setUniformLocation("MoonPosition1");
	shaderRinged->setUniformLocation("MoonRadius1");
	shaderRinged->setUniformLocation("MoonPosition2");
	shaderRinged->setUniformLocation("MoonRadius2");
	shaderRinged->setUniformLocation("MoonPosition3");
	shaderRinged->setUniformLocation("MoonRadius3");
	shaderRinged->setUniformLocation("MoonPosition4");
	shaderRinged->setUniformLocation("MoonRadius4");

	//fisheye
	shaderRinged->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderRinged->setUniformLocation("clipping_fov");

	shaderNormal = new shaderProgram();
	shaderNormal->init( "body_normal.vert", "body_normal.frag");
	shaderNormal->setUniformLocation("UmbraColor");

	//commum
	shaderNormal->setUniformLocation("planetRadius");
	shaderNormal->setUniformLocation("planetScaledRadius");
	shaderNormal->setUniformLocation("planetOneMinusOblateness");

	shaderNormal->setUniformLocation("SunHalfAngle");
	shaderNormal->setUniformLocation("LightPosition");
	shaderNormal->setUniformLocation("ModelViewProjectionMatrix");
	shaderNormal->setUniformLocation("ModelViewMatrix");
	shaderNormal->setUniformLocation("NormalMatrix");

	shaderNormal->setUniformLocation("MoonPosition1");
	shaderNormal->setUniformLocation("MoonRadius1");
	shaderNormal->setUniformLocation("MoonPosition2");
	shaderNormal->setUniformLocation("MoonRadius2");
	shaderNormal->setUniformLocation("MoonPosition3");
	shaderNormal->setUniformLocation("MoonRadius3");
	shaderNormal->setUniformLocation("MoonPosition4");
	shaderNormal->setUniformLocation("MoonRadius4");

	//fisheye
	shaderNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderNormal->setUniformLocation("clipping_fov");

	shaderArtificial = new shaderProgram();
	shaderArtificial->init( "body_artificial.vert", "body_artificial.frag");

	//commum
	shaderArtificial->setUniformLocation("radius");
	shaderArtificial->setUniformLocation("useTexture");
	shaderArtificial->setUniformLocation("ProjectionMatrix");
	shaderArtificial->setUniformLocation("ModelViewMatrix");
	shaderArtificial->setUniformLocation("NormalMatrix");
	shaderArtificial->setUniformLocation("MVP");

	shaderArtificial->setUniformLocation("Light.Position");
	shaderArtificial->setUniformLocation("Light.Intensity");
	shaderArtificial->setUniformLocation("Material.Ka");
	shaderArtificial->setUniformLocation("Material.Kd");
	shaderArtificial->setUniformLocation("Material.Ks");
	shaderArtificial->setUniformLocation("Material.Ns");

	//fisheye
	shaderArtificial->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderArtificial->setUniformLocation("clipping_fov");


	shaderMoonBump = new shaderProgram();
	shaderMoonBump->init( "moon_bump.vert","", "" ,"", "moon_bump.frag");
	shaderMoonBump->setUniformLocation("UmbraColor");
	//commum
	shaderMoonBump->setUniformLocation("planetRadius");
	shaderMoonBump->setUniformLocation("planetScaledRadius");
	shaderMoonBump->setUniformLocation("planetOneMinusOblateness");

	shaderMoonBump->setUniformLocation("SunHalfAngle");
	shaderMoonBump->setUniformLocation("LightPosition");
	shaderMoonBump->setUniformLocation("ModelViewProjectionMatrix");
	shaderMoonBump->setUniformLocation("ModelViewMatrix");
	shaderMoonBump->setUniformLocation("NormalMatrix");

	shaderMoonBump->setUniformLocation("MoonPosition1");
	shaderMoonBump->setUniformLocation("MoonRadius1");

	//fisheye
	shaderMoonBump->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderMoonBump->setUniformLocation("clipping_fov");


	shaderMoonNormal = new shaderProgram();
	shaderMoonNormal->init( "moon_normal.vert", "moon_normal.frag");
	shaderMoonNormal->setUniformLocation("UmbraColor");

	//commum
	shaderMoonNormal->setUniformLocation("planetRadius");
	shaderMoonNormal->setUniformLocation("planetScaledRadius");
	shaderMoonNormal->setUniformLocation("planetOneMinusOblateness");

	shaderMoonNormal->setUniformLocation("SunHalfAngle");
	shaderMoonNormal->setUniformLocation("LightPosition");
	shaderMoonNormal->setUniformLocation("ModelViewProjectionMatrix");
	shaderMoonNormal->setUniformLocation("ModelViewMatrix");
	shaderMoonNormal->setUniformLocation("NormalMatrix");

	shaderMoonNormal->setUniformLocation("MoonPosition1");
	shaderMoonNormal->setUniformLocation("MoonRadius1");

	//fisheye
	shaderMoonNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderMoonNormal->setUniformLocation("clipping_fov");

	OrbitPlot::createShader();

	Trail::createShader();

	Halo::createShader();

	Hints::createShader();

	Axis::createShader();
}


void Body::deleteShader()
{
	if (shaderNight) delete shaderNight;
	if (shaderBump) delete shaderBump;
	if (shaderRinged) delete shaderRinged;
	if (shaderNormal) delete shaderNormal;

	if (shaderArtificial) delete shaderArtificial;
	if (shaderMoonBump) delete shaderMoonBump;
	if (shaderMoonNormal) delete shaderMoonNormal;

	OrbitPlot::deleteShader();

	Trail::deleteShader();

	Halo::deleteShader();

	Hints::deleteShader();

	Axis::deleteShader();
}


void Body::selectShader()
{
	Log.write("Body::selectShader pour  "+englishName, cLog::LOG_TYPE::L_WARNING);
	printf("Body::selectShader utilisé pour %s\n", englishName.c_str());
}


// Return the information string "ready to print" :)
string Body::getInfoString(const Navigator * nav) const
{
	double tempDE, tempRA;
	ostringstream oss;

	oss << _(englishName);  // UI translation can differ from sky translation
	oss.setf(ios::fixed);
	oss.precision(1);
	oss << endl;

	oss.precision(2);
	oss << _("Magnitude: ") << computeMagnitude(nav->getObserverHelioPos()) << endl;

	Vec3d equPos = getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA,&tempDE,equPos);

	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << " / " << Utility::printAngleDMS(tempDE) << endl;

	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA,&tempDE,localPos);
	tempRA = 3*C_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > C_PI*2) tempRA -= C_PI*2;

	oss << _("Alt/Az: ") << Utility::printAngleDMS(tempDE) << " / " << Utility::printAngleDMS(tempRA) << endl;

	oss.precision(8);
	oss << _("Distance: ") << equPos.length() << " " << _("AU");

	return oss.str();
}

//! Get sky label (sky translation)
string Body::getSkyLabel(const Navigator * nav) const
{
	return nameI18;
}


// Return the information string "ready to print" :)
string Body::getShortInfoString(const Navigator * nav) const
{
	ostringstream oss;
	oss << _(englishName);  // UI translation can differ from sky translation
	oss << ", " << getTypePlanet(typePlanet) << ":";
	oss.setf(ios::fixed);
	oss.precision(2);
	oss << "  " << _("Magnitude: ") << computeMagnitude(nav->getObserverHelioPos());

	Vec3d equPos = getEarthEquPos(nav);
	oss.precision(4);
	oss << "  " <<  _("Distance: ") << equPos.length() << " " << _("AU");
	return oss.str();
}

void Body::getAltAz(const Navigator * nav, double *alt, double *az) const
{
	double _alt, _az;
	Vec3d equPos = getEarthEquPos(nav);
	Vec3d local_pos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&_az,&_alt,local_pos);
	_az = 3*C_PI - _az;
	if (_az > C_PI*2) _az -= C_PI*2;
	*az=_az;
	*alt= _alt;
}

string Body::getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
	ostringstream oss;
	double tempDE, tempRA;
	Vec3d equPos = getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA,&tempDE,equPos);
	double latitude = observatory->getLatitude();
	double daytime = tan(tempDE)*tan(latitude*C_PI/180); // partial calculation to determinate if midnight sun or not
	oss << _("RA/DE: ") << Utility::printAngleHMS(tempRA) << "/" << Utility::printAngleDMS(tempDE);

	double jd=timeMgr->getJulian();
	double sidereal;
	double T;
	double Le;
	double HA;
	double GHA;
	double PA;

	T = (jd - 2451545.0) / 36525.0;
	Le = observatory->getLongitude();
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	HA=sidereal+Le-tempRA*180.0/C_PI;
	GHA=sidereal-tempRA*180.0/C_PI;
	while (HA>=360) HA-=360;
	while (HA<0)    HA+=360;
	if (HA<180) PA=HA;
	else PA=360-HA;
	while (GHA>=360) GHA-=360;
	while (GHA<0)    GHA+=360;
	while (tempRA>=2*C_PI) tempRA-=2*C_PI;
	while (tempRA<0)    tempRA+=2*C_PI;

	oss << _("SA ") << Utility::printAngleDMS(2*C_PI-tempRA)
	    << " GHA " << Utility::printAngleDMS(GHA*C_PI/180.0)
	    << " LHA " << Utility::printAngleDMS(HA*C_PI/180.0);
	// calculate alt az
	Vec3d local_pos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA,&tempDE,local_pos);
	tempRA = 3*C_PI - tempRA;  // N is zero, E is 90 degrees
	if (tempRA > C_PI*2) tempRA -= C_PI*2;
	oss << "@" << _(" Az/Alt/coA: ") << Utility::printAngleDMS(tempRA) << "/" << Utility::printAngleDMS(tempDE) << "/" << Utility::printAngleDMS(((C_PI/180.f)*90.)-tempDE) << " LPA " << Utility::printAngleDMS(PA*C_PI/180.0);
	if (englishName == "Sun") {
		oss << " Day length: " ;
		if (daytime<-1) oss << "00h00m00s";
		else if (daytime>1) oss << "24h00m00s";
		else {
			daytime=2*(C_PI-acos(daytime));
			oss << Utility::printAngleHMS(daytime);
		}
	}
	return oss.str();
}

double Body::getCloseFov(const Navigator* nav) const
{
	return atanf(radius*2.f/getEarthEquPos(nav).length())*180./C_PI * 4;
}

double Body::getSatellitesFov(const Navigator * nav) const
{

	if ( !satellites.empty() && englishName != "Sun") {
		double rad = getBoundingRadius();
		if ( rad > 0 ) return atanf(rad/getEarthEquPos(nav).length()) *180./C_PI * 4;
	}

	return -1.;

}

double Body::getParentSatellitesFov(const Navigator *nav) const
{
	if (parent && parent->parent) return parent->getSatellitesFov(nav);
	return -1.0;
}

// Set the orbital elements
void Body::set_rotation_elements(float _period, float _offset, double _epoch, float _obliquity,
                                 float _ascendingNode, float _precessionRate, double _sidereal_period, float _axial_tilt)
{
	re.period = _period;
	re.offset = _offset;
	re.epoch = _epoch;
	re.obliquity = _obliquity;
	re.ascendingNode = _ascendingNode;
	re.precessionRate = _precessionRate;
	re.sidereal_period = _sidereal_period;  // used for drawing orbit lines
	re.axialTilt = _axial_tilt; // Used for drawing tropic lines
}

void Body::setSphereScale(float s, bool initial_scale )
{
	radius = initialRadius * s;
	if (initial_scale)
		initialScale = s;
	updateBoundingRadii();
}

// Return the Body position in rectangular earth equatorial coordinate
Vec3d Body::getEarthEquPos(const Navigator * nav) const
{
	Vec3d v = get_heliocentric_ecliptic_pos();
	return nav->helioToEarthPosEqu(v);		// this is earth equatorial but centered
}

Vec3d Body::getObsJ2000Pos(const Navigator *nav) const
{
	return mat_vsop87_to_j2000.multiplyWithoutTranslation(
	           get_heliocentric_ecliptic_pos()
	           - nav->getObserverHelioPos());
}


// Compute the position in the parent Body coordinate system
// Actually call the provided function to compute the ecliptical position
void Body::computePositionWithoutOrbits(const double date)
{
	if (fabs(lastJD-date)>deltaJD) {
		if(orbit) orbit->positionAtTimevInVSOP87Coordinates(date, ecliptic_pos);
		lastJD = date;
	}
}


void Body::compute_position(const double date)
{
/*
	//~ if(!orbit) 
		//~ return;

	//~ // Large performance advantage from avoiding object overhead
	//~ OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction();

	//~ // for performance only update orbit points if visible
	//~ if (visibilityFader.getInterstate()>0.000001 && delta_orbitJD > 0 && (fabs(last_orbitJD-date)>delta_orbitJD || !orbit_cached)) {
		//~ // calculate orbit first (for line drawing)
		//~ double date_increment = re.sidereal_period/ORBIT_SEGMENTS;
		//~ double calc_date;
		//~ //	  int delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		//~ int delta_points;

		//~ if ( date > last_orbitJD ) {
			//~ delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		//~ } else {
			//~ delta_points = (int)(-0.5 + (date - last_orbitJD)/date_increment);
		//~ }
		//~ double new_date = last_orbitJD + delta_points*date_increment;

		//~ //printf( "Updating orbit coordinates for %s (delta %f) (%d points)\n", name.c_str(), delta_orbitJD, delta_points);
		//~ //cout << englishName << ": " << delta_points << "  " << orbit_cached << endl;

		//~ if ( delta_points > 0 && delta_points < ORBIT_SEGMENTS && orbit_cached) {
			//~ for ( int d=0; d<ORBIT_SEGMENTS; d++ ) {
				//~ if (d + delta_points >= ORBIT_SEGMENTS ) {
					//~ // calculate new points
					//~ calc_date = new_date + (d-ORBIT_SEGMENTS/2)*date_increment;
					//~ // date increments between points will not be completely constant though

					//~ if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
					//~ else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
				//~ } else {
					//~ orbitPoint[d] = orbitPoint[d+delta_points];
				//~ }
			//~ }

			//~ last_orbitJD = new_date;

		//~ } 
		//~ else{
			//~ if ( delta_points < 0 && abs(delta_points) < ORBIT_SEGMENTS  && orbit_cached) {
				//~ for ( int d=ORBIT_SEGMENTS-1; d>=0; d-- ) {
					//~ if (d + delta_points < 0 ) {
						//~ // calculate new points
						//~ calc_date = new_date + (d-ORBIT_SEGMENTS/2)*date_increment;

						//~ if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
						//~ else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					//~ } else {
						//~ orbitPoint[d] = orbitPoint[d+delta_points];
					//~ }
				//~ }

				//~ last_orbitJD = new_date;

			//~ } 
			//~ else{
				//~ if ( delta_points || !orbit_cached) {
					//~ // update all points (less efficient)
					//~ for ( int d=0; d<ORBIT_SEGMENTS; d++ ) {
						//~ calc_date = date + (d-ORBIT_SEGMENTS/2)*date_increment;

						//~ if(oscFunc) (*oscFunc)(date,calc_date,orbitPoint[d]);
						//~ else orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					//~ }

					//~ last_orbitJD = date;

					//~ // \todo remove this for efficiency?  Can cause rendering issues near body though
					//~ // If orbit is largely constant through time cache it
					//~ if (orbit->isStable(date)) orbit_cached = 1;
				//~ }
			//~ }

		//~ }
		//~ // calculate actual Body position
		//~ if(oscFunc) (*oscFunc)(date,date,ecliptic_pos);
		//~ else orbit->positionAtTimevInVSOP87Coordinates(date,date,ecliptic_pos);

		//~ lastJD = date;
	//~ } 
	//~ else {
*/

		OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction();

		if(orbitPlot != nullptr && flags.flag_orbit==true){
			orbitPlot->init();
			orbitPlot->computeOrbit(date);
		}

		double delta = date-lastJD;
		delta = fabs(delta);

		if(delta >= deltaJD ) {
			if(oscFunc) 
				(*oscFunc)(date,date,ecliptic_pos);
			else 
				orbit->positionAtTimevInVSOP87Coordinates(date,date,ecliptic_pos);

			lastJD = date;
		}
	//}
}

// Compute the transformation matrix from the local Body coordinate to the parent Body coordinate
void Body::compute_trans_matrix(double jd)
{
	axis_rotation = getSiderealTime(jd);

	// Special case - heliocentric coordinates are on ecliptic, not solar equator...
	if (parent) {
		rot_local_to_parent = Mat4d::zrotation(re.ascendingNode
		                                       -re.precessionRate*(jd-re.epoch))
		                      * Mat4d::xrotation(re.obliquity);
		rot_local_to_parent_unprecessed = Mat4d::zrotation(re.ascendingNode)
		                                  * Mat4d::xrotation(re.obliquity);

	}

	mat_local_to_parent = Mat4d::translation(ecliptic_pos)
	                      * rot_local_to_parent;

}

Mat4d Body::getRotEquatorialToVsop87(void) const
{
	Mat4d rval = rot_local_to_parent;
	if (parent) for (const Body *p=parent; p->parent; p=p->parent) {
			rval = p->rot_local_to_parent * rval;
		}
	return rval;
}

void Body::setRotEquatorialToVsop87(const Mat4d &m)
{
	Mat4d a = Mat4d::identity();
	if (parent) for (const Body *p=parent; p->parent; p=p->parent) {
			a = p->rot_local_to_parent * a;
		}
	rot_local_to_parent = a.transpose() * m;
}

string Body::getTypePlanet(const BODY_TYPE str)  const
{
	switch (str) {
		case 0 :
			return "sun";
		case 1 :
			return "planet";
		case 2 :
			return "moon";
		case 3 :
			return "dwarf planet";
		case 4 :
			return "asteroid";
		case 5 :
			return "kuiper belt object";
		case 6 :
			return "comet";
		case 7 :
			return "artificial";
		case 8 :
			return "observer";
		default :
			return "unknown";
	}
	return "";
}

// Compute the z rotation to use from equatorial to geographic coordinates
double Body::getSiderealTime(double jd) const
{
	if (englishName=="Earth") return get_apparent_sidereal_time(jd);

	double t = jd - re.epoch;
	double rotations = t / (double) re.period;
	double wholeRotations = floor(rotations);
	double remainder = rotations - wholeRotations;

	return remainder * 360. + re.offset;
}

// Get the Body position in the parent Body ecliptic coordinate
Vec3d Body::get_ecliptic_pos() const
{
	return ecliptic_pos;
}

// Return the heliocentric ecliptical position used only for earth shadow, lunar eclipse -- DIGITALIS: this statement is not true!
Vec3d Body::get_heliocentric_ecliptic_pos() const
{
	Vec3d pos = ecliptic_pos;
	const Body *p = parent;

	while (p && p->parent) {
		pos += p->ecliptic_pos;
		p = p->parent;
	}

	return pos;
}


void Body::set_heliocentric_ecliptic_pos(const Vec3d &pos)
{
	ecliptic_pos = pos;
	const Body *p = parent;
	if (p) while (p->parent) {
			ecliptic_pos -= p->ecliptic_pos;
			p = p->parent;
		}
}


// Compute the distance to the given position in heliocentric coordinate (in AU)
double Body::compute_distance(const Vec3d& obs_helio_pos)
{
	distance = (obs_helio_pos-get_heliocentric_ecliptic_pos()).length();
	return distance;
}

// Get the phase angle for an observer at pos obs_pos in the heliocentric coordinate (dist in AU)
double Body::get_phase(Vec3d obs_pos) const
{
	const double sq = obs_pos.lengthSquared();
	const Vec3d heliopos = get_heliocentric_ecliptic_pos();
	const double Rq = heliopos.lengthSquared();
	const double pq = (obs_pos - heliopos).lengthSquared();
	const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
	return (1.0 - acos(cos_chi)/C_PI) * cos_chi + sqrt(1.0 - cos_chi*cos_chi) / C_PI;
}

float Body::computeMagnitude(Vec3d obs_pos) const
{
	float rval = 0;
	const double sq = obs_pos.lengthSquared();
	const Vec3d heliopos = get_heliocentric_ecliptic_pos();
	const double Rq = heliopos.lengthSquared();
	const double pq = (obs_pos - heliopos).lengthSquared();
	const double cos_chi = (pq + Rq - sq)/(2.0*sqrt(pq*Rq));
	const double phase = (1.0 - acos(cos_chi)/C_PI) * cos_chi
	                     + sqrt(1.0 - cos_chi*cos_chi) / C_PI;
	const float F = 2.0 * albedo * radius * radius * phase / (3.0*pq*Rq);
	rval = -26.73f - 2.5f * log10f(F);
	return rval;
}

float Body::computeMagnitude(const Navigator * nav) const
{
	return computeMagnitude(nav->getObserverHelioPos());
}


// Return the radius of a circle containing the object on screen
float Body::getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only)
{
	double rad = radius;
	return atanf(rad*2.f/getEarthEquPos(nav).length())*180./C_PI/prj->getFov()*prj->getViewportHeight();
}

// Return the angle (degrees) of the Body orb
float Body::get_angular_size(const Projector* prj, const Navigator * nav)
{

	return atanf(radius*2.f/getEarthEquPos(nav).length())*180./C_PI;
}


// Return the radius of a circle containing the object and its satellites on screen
float Body::get_on_screen_bounding_size(const Projector* prj, const Navigator * nav)
{
	double rad = getBoundingRadius();

	return atanf(rad*2.f/getEarthEquPos(nav).length())*180./C_PI/prj->getFov()*prj->getViewportHeight();
}

void Body::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	////AutoPerfDebug apd(&pd, "Body::drawBody$"); //Debug
	StateGL::enable(GL_CULL_FACE);
	//~ StateGL::disable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, tex_map->getID());

	myShaderProg->use();

	//load specific values for shader
	switch (myShader) {

		case SHADER_BUMP:
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_norm->getID());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());
			//~ myShaderProg->setUniform("Ambient",ambientLight);
			break;

		case SHADER_NORMAL :
		default: //shader normal
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());
			myShaderProg->setUniform("mapTexture",0);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());
			myShaderProg->setUniform("shadowTexture",2);
			//~ myShaderProg->setUniform("Ambient",ambientLight);
			break;
	}
	//paramétrage des matrices pour opengl4
	Mat4f proj = prj->getMatProjection().convert();
	//TODO mettre extra rotation en cache
	Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(C_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
	myShaderProg->setUniform("ModelViewProjectionMatrix",proj*matrix);
	myShaderProg->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("clipping_fov",prj->getClippingFov());
	myShaderProg->setUniform("planetScaledRadius",radius);

	if ( myShader == SHADER_RINGED )
		myShaderProg->setUniform("ModelViewMatrixInverse", inv_matrix);

	//paramètres commun aux shaders sauf Sun
	myShaderProg->setUniform("planetRadius",initialRadius);
	myShaderProg->setUniform("planetOneMinusOblateness",one_minus_oblateness);

	//utilisable si on est pas le soleil
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	int index=1;
	myShaderProg->setUniform("LightPosition",eye_sun);
	myShaderProg->setUniform("SunHalfAngle",sun_half_angle);

	double length;
	double moonDotLight;
	Vec3f tmp(0,0,0);
	Vec3f tmp2(0.4, 0.12, 0.0);

	if (myShader == SHADER_BUMP || myShader == SHADER_MOON_BUMP) {
		if(getEnglishName() == "Moon") {
			myShaderProg->setUniform("UmbraColor",tmp2);
		} else
			myShaderProg->setUniform("UmbraColor",tmp);
	}

	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

	// Parent may shadow this satellite
	if( isSatellite() ) {
		tmp = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
		myShaderProg->setUniform("MoonPosition1",tmp);
		myShaderProg->setUniform("MoonRadius1",parent->getRadius());
		index++;
	}

	list<Body*>::iterator iter;
	for(iter=satellites.begin(); iter!=satellites.end() && index <= 4; iter++) {
		tmp2 = (*iter)->get_heliocentric_ecliptic_pos() - planet_helio;
		length = tmp2.length();
		tmp2.normalize();
		moonDotLight = tmp2.dot(light);
		if(moonDotLight > 0 && length*sin(acos(moonDotLight)) <= radius + 2*(*iter)->getRadius()) {
			tmp = nav->getHelioToEyeMat() * (*iter)->get_heliocentric_ecliptic_pos();

			if (index==1) {
				myShaderProg->setUniform("MoonPosition1",tmp);
				myShaderProg->setUniform("MoonRadius1",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==2) {
				myShaderProg->setUniform("MoonPosition2",tmp);
				myShaderProg->setUniform("MoonRadius2",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==3) {
				myShaderProg->setUniform("MoonPosition3",tmp);
				myShaderProg->setUniform("MoonRadius3",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==4) {
				myShaderProg->setUniform("MoonPosition4",tmp);
				myShaderProg->setUniform("MoonRadius4",(*iter)->getRadius()/(*iter)->getSphereScale());
			}

			index++;
		}
	}

	// clear any leftover values
	for(; index<=4; index++) {
		if (index==1) // No moon data
			myShaderProg->setUniform("MoonRadius1",0.0);
		if (index==2)
			myShaderProg->setUniform("MoonRadius2",0.0);
		if (index==3)
			myShaderProg->setUniform("MoonRadius3",0.0);
		if (index==4)
			myShaderProg->setUniform("MoonRadius4",0.0);
	}

	currentObj->draw(screen_sz);

	myShaderProg->unuse();
	glActiveTexture(GL_TEXTURE0);
	StateGL::disable(GL_CULL_FACE);
}


// Start/stop accumulating new trail data (clear old data)
void Body::startTrail(bool b)
{
	if(trail != nullptr)
		trail->startTrail(b);
}


void Body::translateName(Translator& trans)
{
	nameI18 = trans.translateUTF8(englishName);
}

void Body::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	radius.update(delta_time);

	visibilityFader.update(delta_time);

	if(orbitPlot!= nullptr)
		orbitPlot->updateShader(delta_time);

	hints->updateShader(delta_time);

	if(trail!= nullptr) {
		trail->updateShader(delta_time);
		trail->updateTrail(nav, timeMgr);
	}
}

// Update bounding radii from child up to parent(s)
void Body::updateBoundingRadii()
{
	calculateBoundingRadius();
	Body *p;
	p = parent;
	while (p) {
		p->calculateBoundingRadius();
		p = p->parent;
	}
}

// Calculate a bounding radius in AU
// For a Body with satellites, this bounds the most distant satellite orbit
// for planets with no elliptical satellites, this is the ring (if one)
// or Body radius
// Caches result until next call, can retrieve with getBoundingRadius
double Body::calculateBoundingRadius()
{
	// TODO: only have boundary function for elliptical orbits right now
	//~ if ( satellites.empty() ) {
		//~ boundingRadius = -1;
		//~ return -1;  // special case, not visible
	//~ }
	double d = radius.final();

	list<Body *>::const_iterator iter;
	list<Body *>::const_iterator end = satellites.end();

	double r;
	for ( iter=satellites.begin(); iter != end; iter++) {

		r = (*iter)->getBoundingRadius();
		if ( r > d ) d = r;
	}

	// if we are a planet, we want the boundary radius including all satellites
	// if we are a satellite, we want the full orbit radius as well
	if ( is_satellite ) {
		if ( d > orbit_bounding_radius + radius.final() ) boundingRadius = d;
		else boundingRadius = orbit_bounding_radius + radius.final();
	} else boundingRadius = d;

	return boundingRadius;
}


void Body::computeDraw(const Projector* prj, const Navigator* nav)
{
	//~ if (hidden) return ; //0;
	eye_sun = nav->getHelioToEyeMat() * v3fNull;

	mat = mat_local_to_parent;
	parent_mat = Mat4d::identity();

	// \todo account for moon orbit precession (independent of parent)
	// also does not allow for multiple levels of precession
	const Body *p = parent;

	bool myParent = true;
	while (p && p->get_parent()) {   //cette boucle ne sert que pour les lunes des planetes

		// Some orbits are already precessed, namely elp82
		if(myParent && !useParentPrecession(lastJD)) {
			mat = Mat4d::translation(p->get_ecliptic_pos())
			      * mat
			      * p->get_rot_local_to_parent_unprecessed();
		} else {
			mat = Mat4d::translation(p->get_ecliptic_pos())
			      * mat
			      * p->get_rot_local_to_parent();
		}

		parent_mat = Mat4d::translation(p->get_ecliptic_pos())
		             * parent_mat;

		p = p->get_parent();

		myParent = false;
	}

	model = mat.convert();
	view = nav->getHelioToEyeMat().convert();
	vp = prj->getMatProjection().convert() * view;
	viewBeforeLookAt = Mat4d::getViewFromLookAt(nav->getHelioToEyeMat()).convert();

	mat = nav->getHelioToEyeMat() * mat;

	proj = prj->getMatProjection().convert();
	matrix=mat.convert();

	parent_mat = nav->getHelioToEyeMat() * parent_mat;

	eye_planet = mat * v3fNull;

	lightDirection = eye_sun - eye_planet;
	sun_half_angle = atan(696000.0/AU/lightDirection.length());  // hard coded Sun radius!

	lightDirection.normalize();

	// Compute the 2D position and check if in the screen
	screen_sz = getOnScreenSize(prj, nav);
	isVisible = prj->projectCustomCheck(v3fNull, screenPos, mat, (int)(screen_sz/2));

	// Do not draw anything else if was not visible
	// Draw the name, and the circle if it's not too close from the body it's turning around
	// this prevents name overlaping (ie for jupiter satellites)
	ang_dist = 300.f*atan(get_ecliptic_pos().length()/getEarthEquPos(nav).length())/prj->getFov();

	//~ if (ang_dist==0.f)
		//~ ang_dist = 1.f; // if ang_dist == 0, the Body is sun..
}


bool Body::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool selected){
	
	bool drawn = false;
	
	if(skipDrawingThisBody(observatory, drawHomePlanet)){
		
		if(hasRings()){
			drawRings(prj,mat,1000.0,lightDirection,eye_planet,initialRadius);
		}
		
		return drawn;
	}
		
		
	handleVisibilityFader(observatory, prj, nav);
	
	drawOrbit(observatory,nav, prj);
	
	drawTrail(nav, prj);
	
	drawHints(nav, prj);
	      
	if(depthTest){
		StateGL::enable(GL_DEPTH_TEST);
	}
	
	if(isVisibleOnScreen()){
		
		if(hasRings()){
			StateGL::enable(GL_DEPTH_TEST);
			drawAxis(prj,mat);
			drawBody(prj, nav, mat, screen_sz);
			drawRings(prj,mat,screen_sz,lightDirection,eye_planet,initialRadius);
		}
		else{
			drawAxis(prj,mat);
			drawBody(prj, nav, mat, screen_sz);			
		}
		
		if(atmExt != nullptr)
			drawAtmExt(prj, nav, observatory);
			
		drawn = true;
	}

	drawHalo(nav, prj, eye);

	//~ StateGL::disable(GL_STENCIL_TEST);
	StateGL::disable(GL_DEPTH_TEST);
	
	return drawn;
}

void Body::drawAtmExt(const Projector* prj, const Navigator* nav, const Observer* observatory)
{
	// atmExt->use();

	// //~ double radiusRatio = 1.2;

	// //~ cout << "draw atmExt taille " << screen_sz<< " radius " << this->getRadius()*radiusRatio << std::endl;
	// atmExt->setPlanetRadius(this->getRadius());
	// //~ atmExt->setRadius(this->getRadius() * radiusRatio);
	// atmExt->setAtmAlphaScale(0.5f);
	// atmExt->setSunPos(Vec3f(0.0f,0.0f,0.0f));

	// //~ cout << observatory->getHeliocentricPosition(0).convert() << endl; /* Utiliser la bonne date ici */
	// //~ cout << this->get_heliocentric_ecliptic_pos().convert() << endl;

	// // --------------------------------------------------------------------
	// // Ici les paramètres qui vont faire que le shader sera 100% opérationnel.
	// // --------------------------------------------------------------------
	// atmExt->setCameraPositionBeforeLookAt(nav->getObserverHelioPos());
	// atmExt->setPlanetPos(this->get_heliocentric_ecliptic_pos().convert());
	// // --------------------------------------------------------------------

	// //deformation fisheye pour les vertex
	// atmExt->setModelView(matrix);
	// atmExt->setModelViewProjectionMatrix(proj*matrix);
	// atmExt->setInverseModelViewProjectionMatrix((proj*matrix).inverse());
	// atmExt->setClippingFov(prj->getClippingFov());

	// atmExt->draw(screen_sz);
	// atmExt->unuse();
}

bool Body::skipDrawingThisBody(const Observer* observatory, bool drawHomePlanet)
{
	//~ if(hidden)
		//~ return true;
		
	return !drawHomePlanet && observatory->isOnBody(this);
}


void Body::drawOrbit(const Observer* observatory, const Navigator* nav, const Projector* prj){
	orbitPlot->drawOrbit(nav, prj, parent_mat);
}

void Body::drawTrail(const Navigator* nav, const Projector* prj){
	if(trail != nullptr)
		trail->drawTrail(nav, prj);
}

void Body::drawHints(const Navigator* nav, const Projector* prj){
	if (ang_dist>0.25)// {
		//~ if (ang_dist>1.f)
		//~ ang_dist = 1.f;
		hints->drawHints(nav, prj);
}

void Body::removeSatellite(Body *planet){
	list<Body *>::iterator iter;
	for (iter=satellites.begin(); iter != satellites.end(); iter++) {
		if ( (*iter) == planet ) {
			satellites.erase(iter);
			break;
		}
	}
}

void Body::drawAxis(const Projector* prj, const Mat4d& mat){
	axis->drawAxis(prj, mat);
}

void Body::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	if (isVisible && flags.flag_halo && this->getOnScreenSize(prj, nav) < 10) {
		//~ cout << "drawing halo from Body " << this->englishName << " class size " << this->getOnScreenSize(prj, nav) << endl;
		// Workaround for depth buffer precision and near planets
		StateGL::disable(GL_DEPTH_TEST);
		halo->drawHalo(nav, prj, eye);
	}
}

Vec3d Body::getPositionAtDate(double jDate) const{

	double * v = new double[3];
	const Body * b = this;

	this->getOrbit()->positionAtTimevInVSOP87Coordinates(jDate, v);
	Vec3d pos = Vec3d(v[0], v[1], v[2]);

	while(b->getParent()!=nullptr){
		b = b->getParent();
		b->getOrbit()->positionAtTimevInVSOP87Coordinates(jDate, v);
		pos +=  Vec3d(v[0], v[1], v[2]);
	}

	//we don't need v anymore
	delete[] v;

	return pos;

}
