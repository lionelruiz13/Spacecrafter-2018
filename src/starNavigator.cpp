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

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <thread>

#include "starNavigator.hpp"
#include "starManager.hpp"
#include <unistd.h>
#include "utility.hpp"

#include "stateGL.hpp"
#include "perf_debug.hpp"
#include "ThreadPool.hpp"

#include "tone_reproductor.hpp"
#include "projector.hpp"
#include "navigator.hpp"

using namespace std;

static float magnitude_max = 6.5;

#define DELTA_PARSEC 0.005

//////////////////// PARAMETRES STARS //////////////////////////////////////////
static float fov_stars = 60.f;
static float fov_factor = 108064.73f / (fov_stars*fov_stars); //30.017979
static float objectSizeLimit = 0;
static float min_rmag = 0.346891f;
static double fov= 180.;
////////////////////////////////////////////////////////////////////////////////

// ===========================================================================
//
//  Class StarNavigator
//
// ===========================================================================

StarNavigator::StarNavigator()
{
}

void StarNavigator::createShader()
{
}

void StarNavigator::deleteShader()
{
}

void StarNavigator::loadRawData(const std::string &fileName) noexcept
{
}

void StarNavigator::loadOtherData(const std::string &fileName) noexcept
{
}

void StarNavigator::loadData(const std::string &fileName, bool binaryData) noexcept
{
}


void StarNavigator::saveData(const std::string &fileName, bool binaryData) noexcept
{
}

StarNavigator::~StarNavigator()
{
}

void StarNavigator::clearBuffer()
{
}

void StarNavigator::setListGlobalStarVisible()
{
}




Vec3f StarNavigator::color_table[128] = {
	Vec3f(0.780392,0.866666,1),
	Vec3f(0.788235,0.870588,1),
	Vec3f(0.8,0.878431,1),
	Vec3f(0.807843,0.882352,1),
	Vec3f(0.815686,0.886274,1),
	Vec3f(0.82745,0.894117,1),
	Vec3f(0.835294,0.898039,1),
	Vec3f(0.843137,0.905882,1),
	Vec3f(0.854901,0.909803,1),
	Vec3f(0.866666,0.917647,1),
	Vec3f(0.874509,0.921568,1),
	Vec3f(0.886274,0.92549,1),
	Vec3f(0.894117,0.933333,1),
	Vec3f(0.905882,0.941176,1),
	Vec3f(0.917647,0.945098,1),
	Vec3f(0.92549,0.952941,1),
	Vec3f(0.937254,0.956862,1),
	Vec3f(0.949019,0.964705,1),
	Vec3f(0.956862,0.968627,1),
	Vec3f(0.968627,0.97647,1),
	Vec3f(0.980392,0.980392,1),
	Vec3f(0.988235,0.988235,1),
	Vec3f(1,0.992156,0.996078),
	Vec3f(1,0.988235,0.984313),
	Vec3f(1,0.980392,0.972549),
	Vec3f(1,0.97647,0.960784),
	Vec3f(1,0.972549,0.952941),
	Vec3f(1,0.968627,0.941176),
	Vec3f(1,0.964705,0.929411),
	Vec3f(1,0.960784,0.917647),
	Vec3f(1,0.956862,0.905882),
	Vec3f(1,0.949019,0.898039),
	Vec3f(1,0.945098,0.886274),
	Vec3f(1,0.941176,0.878431),
	Vec3f(1,0.937254,0.866666),
	Vec3f(1,0.933333,0.854901),
	Vec3f(1,0.929411,0.843137),
	Vec3f(1,0.92549,0.835294),
	Vec3f(1,0.917647,0.823529),
	Vec3f(1,0.913725,0.815686),
	Vec3f(1,0.909803,0.803921),
	Vec3f(1,0.905882,0.796078),
	Vec3f(1,0.90196,0.784313),
	Vec3f(1,0.898039,0.77647),
	Vec3f(1,0.894117,0.764705),
	Vec3f(1,0.890196,0.752941),
	Vec3f(1,0.886274,0.745098),
	Vec3f(1,0.882352,0.737254),
	Vec3f(1,0.878431,0.72549),
	Vec3f(1,0.870588,0.713725),
	Vec3f(1,0.866666,0.705882),
	Vec3f(1,0.862745,0.694117),
	Vec3f(1,0.858823,0.686274),
	Vec3f(1,0.854901,0.674509),
	Vec3f(1,0.85098,0.666666),
	Vec3f(1,0.847058,0.654901),
	Vec3f(1,0.843137,0.643137),
	Vec3f(1,0.839215,0.635294),
	Vec3f(1,0.835294,0.62745),
	Vec3f(1,0.831372,0.615686),
	Vec3f(1,0.82745,0.607843),
	Vec3f(1,0.823529,0.596078),
	Vec3f(1,0.819607,0.588235),
	Vec3f(1,0.815686,0.57647),
	Vec3f(1,0.811764,0.568627),
	Vec3f(1,0.807843,0.556862),
	Vec3f(1,0.803921,0.545098),
	Vec3f(1,0.8,0.533333),
	Vec3f(1,0.796078,0.529411),
	Vec3f(1,0.792156,0.517647),
	Vec3f(1,0.788235,0.505882),
	Vec3f(1,0.784313,0.494117),
	Vec3f(1,0.780392,0.486274),
	Vec3f(1,0.77647,0.474509),
	Vec3f(1,0.772549,0.458823),
	Vec3f(1,0.768627,0.45098),
	Vec3f(1,0.764705,0.439215),
	Vec3f(1,0.760784,0.423529),
	Vec3f(1,0.756862,0.415686),
	Vec3f(1,0.752941,0.4),
	Vec3f(1,0.749019,0.392156),
	Vec3f(1,0.745098,0.37647),
	Vec3f(1,0.741176,0.360784),
	Vec3f(1,0.737254,0.349019),
	Vec3f(1,0.733333,0.329411),
	Vec3f(1,0.729411,0.313725),
	Vec3f(1,0.729411,0.30196),
	Vec3f(1,0.721568,0.278431),
	Vec3f(1,0.717647,0.258823),
	Vec3f(1,0.717647,0.235294),
	Vec3f(1,0.709803,0.215686),
	Vec3f(1,0.709803,0.196078),
	Vec3f(1,0.70196,0.164705),
	Vec3f(1,0.70196,0.141176),
	Vec3f(1,0.698039,0.109803),
	Vec3f(1,0.690196,0.062745),
	Vec3f(1,0.690196,0),
	Vec3f(1,0.686274,0),
	Vec3f(1,0.682352,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.674509,0),
	Vec3f(1,0.670588,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.662745,0),
	Vec3f(1,0.658823,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.65098,0),
	Vec3f(1,0.647058,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.639215,0),
	Vec3f(1,0.635294,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.623529,0),
	Vec3f(1,0.619607,0),
	Vec3f(1,0.615686,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.607843,0),
	Vec3f(1,0.603921,0),
	Vec3f(1,0.6,0),
	Vec3f(1,0.6,0)
};


void StarNavigator::initRCMagTable()
{
}


int StarNavigator::computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2])
{
	return 0;
}

//pos designe la position de la caméra
void StarNavigator::computePosition(Vec3f posI) noexcept
{
}

bool StarNavigator::computeChunk(unsigned int first, unsigned int last)
{
	return false;
}



void StarNavigator::draw(const Navigator * nav, const Projector* prj) const noexcept
{
}
