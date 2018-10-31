/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of the LSS Team && Immersive Adventure
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

#include <vector>
#include "object.hpp"
#include "ojm_mgr.hpp"
#include "log.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "stateGL.hpp"

OjmMgr::OjmMgr()
{
}

OjmMgr::~OjmMgr()
{
}

OjmMgr::STATE_POSITION OjmMgr::convert(const std::string & value)
{
}

bool OjmMgr::load(const std::string &mode, const std::string &name, const std::string &fileName, const std::string &pathFile, Vec3f Position, float multiplier)
{
}

bool OjmMgr::remove(const std::string &mode, const std::string& _name)
{
}

bool OjmMgr::remove(STATE_POSITION state, const std::string& _name)
{
}

void OjmMgr::removeAll(const std::string &mode)
{
}

void OjmMgr::removeAll(STATE_POSITION state)
{
}


void OjmMgr::update(int delta_time)
{}

void OjmMgr::draw(Projector *prj, const Navigator *nav, STATE_POSITION state)
{
}


void OjmMgr::createShader()
{
}

void OjmMgr::deleteShader()
{
}
