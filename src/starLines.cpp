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
#include <string>
#include <unistd.h>

#include "starLines.hpp"
#include "utility.hpp"
#include "log.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "ia.hpp"

using namespace std;


StarLines::StarLines()
{
	isAlive = false;
}


StarLines::~StarLines()
{
}


void StarLines::createShader()
{
}

void StarLines::deleteShader()
{
}

bool StarLines::loadHipCatalogue(std::string fileName) noexcept
{
	return false;
}

bool StarLines::loadHipBinCatalogue(std::string fileName) noexcept
{
	return false;
}


bool StarLines::loadData(std::string fileName) noexcept
{
	return false;
}


void StarLines::loadStringData(std::string record) noexcept
{
}


void StarLines::drop()
{
}

Vec3f StarLines::searchInHip(int HIP)
{
	return v3fNull;
}

//version 3D in GALAXY mode
void StarLines::draw(const Navigator * nav) noexcept
{
}

//version 2D, in SOLAR_SYSTEM MODE
void StarLines::draw(const Projector* prj) noexcept
{
}


//version 3D
void StarLines::draw(Mat4f & matrix)  noexcept
{
}
