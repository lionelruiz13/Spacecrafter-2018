/*
* Copyright (C) 2003 Fabien Chereau
* Copyright (C) 2009 Digitalis Education Solutions, Inc.
* Copyright (C) 2013 of the LSS team
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

#ifndef _SKYBRIGHT_H_
#define _SKYBRIGHT_H_

class Skybright {
public:
	Skybright();

	// month : 1=Jan, 12=Dec
	// moon_phase in radian 0=Full Moon, PI/2=First Quadrant/Last Quadran, PI=No Moon
	void setDate(int year, int month, float moon_phase);

	// Set the position parameters
	// Latitude in radian, altitude in m, temperature in deg. C, humidity in %
	void setLoc(float latitude, float altitude, float temperature = 15.f,
	             float relative_humidity = 40.f);

	// Set the moon and sun zenith angular distance (cosin given) and precompute what can be
	// This funtion has to be called once before any call to getLuminance()
	// Input : cos_dist_moon_zenith = cos(angular distance between moon and zenith)
	//		   cos_dist_sun_zenith  = cos(angular distance between sun and zenith)
	void setSunMoon(float cos_dist_moon_zenith, float cos_dist_sun_zenith, int cor_optoma);

	// Compute the luminance at the given position
	// Inputs : cos_dist_moon = cos(angular distance between moon and the position)
	//			cos_dist_sun  = cos(angular distance between sun  and the position)
	//			cos_dist_zenith = cos(angular distance between zenith and the position)
	float getLuminance(float cos_dist_moon, float cos_dist_sun, float cos_dist_zenith, int cor_optoma);

private:
	float air_mass_moon;	// Air mass for the Moon
	float air_mass_sun;		// Air mass for the Sun
	float mag_moon;			// Moon magnitude
	float RA;				// Something related with date
	float K;				// Useful coef...
	float C3;				// Term for moon brightness computation
	float C4;				// Term for sky brightness computation
	float SN; 				// Snellen Ratio (20/20=1.0, good 20/10=2.0)

	// Optimisation variables
	float b_night_term;
	float b_moon_term1;
	float b_twilight_term;
};

#endif // _SKYBRIGHT_H_
