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

#include "dso3d.hpp"
#include "utility.hpp"
#include <string>
#include "log.hpp"
#include "app_settings.hpp"
#include "observer.hpp"
#include "projector.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include <iomanip>
#include <math.h>
#include "fmath.hpp"
#include "s_texture.hpp"

using namespace std;

Dso3d::Dso3d()
{
}

Dso3d::~Dso3d()
{
}

void Dso3d::createShader()
{
}

void Dso3d::deleteShader()
{
}

bool Dso3d::loadCatalog(const std::string &cat) noexcept
{
}

void Dso3d::setTexture(const string& tex_file)
{
}


void Dso3d::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
}
