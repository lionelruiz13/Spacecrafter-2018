/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2016 Association Sirius
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

#ifndef __TIME_MGR_HPP__
#define __TIME_MGR_HPP__

// Conversion in standar Julian time format
#define JD_SECOND 0.000011574074074074074074
#define JD_MINUTE 0.00069444444444444444444
#define JD_HOUR   0.041666666666666666666
#define JD_DAY    1.

#include "utility.hpp"

class TimeMgr {
public:
	TimeMgr();
	~TimeMgr();
	TimeMgr(TimeMgr const &) = delete;
	TimeMgr& operator = (TimeMgr const &) = delete;

	// Time controls
	void setJDay(double JD) {
		JDay=JD;
	}
	double getJDay(void) const {
		return JDay;
	}
	void setTimeSpeed(double ts) {
		time_speed=ts;
	}
	double getTimeSpeed(void) const {
		return time_speed;
	}

	//! Increment time
	void update(int delta_time); // ancien update_time

	double getJulian(void) const {
		return JDay;
	}

	//! return the JD time when the sun go down
	double dateSunRise(double jd, double longitude, double latitude);
	//! return the JD time when the sun set up
	double dateSunSet(double jd, double longitude, double latitude);
	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double jd, double longitude, double latitude);

private:
	// Time variable
	double time_speed;				// Positive : forward, Negative : Backward, 1 = 1sec/sec
	double JDay;        			// Curent time in Julian day

};

#endif
