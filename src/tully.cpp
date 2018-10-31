/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "tully.hpp"
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

Tully::Tully()
{
}

Tully::~Tully()
{
}

void Tully::createShaderPoints()
{
}

void Tully::deleteShaderPoints()
{
}

void Tully::createShaderSquare()
{
}

void Tully::deleteShaderSquare()
{
}

bool Tully::loadCatalog(const std::string &cat) noexcept
{
	return false;
}


void Tully::setTexture(const string& tex_file)
{
}

bool Tully::compTmpTully(const tmpTully &a,const tmpTully &b)
{
	return false;
}

void Tully::computeSquareGalaxies(Vec3f camPosition)
{
}


void Tully::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
}
