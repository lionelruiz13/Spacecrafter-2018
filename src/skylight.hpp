/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

// Class which computes the daylight sky color
// Fast implementation of the algorithm from the article
// "A Practical Analytic Model for Daylight" by A. J. Preetham, Peter Shirley and Brian Smits.

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_

typedef struct {
	float zenith_angle; 	// zenith_angle : angular distance to the zenith in radian
	float dist_sun;			// dist_sun     : angular distance to the sun in radian
	float color[3];			// 3 component color, can be RGB or CIE color system
} skylight_struct;

typedef struct {
	float pos[3];			// Vector to the position (vertical = pos[2])
	float color[3];			// 3 component color, can be RGB or CIE color system
} skylight_struct2;

class Skylight {
public:
	Skylight();
	virtual ~Skylight();
	// Set the fixed parameters and precompute what can be
	// This funtion has to be called once before any call to get_*_value()
	void setParams(float sun_zenith_angle, float turbidity, std::string planetName);
	// Compute the sky color at the given position in the xyY color system and store it in position.color
	inline void get_xyY_Value(skylight_struct * position);
	// Return the current zenith color
	inline void getZenithColor(float * v) const;

	// Same functions but in vector mode : faster because prevents extra cosine calculations
	// The position vectors MUST be normalized, and the vertical z component is the third one
	void setParamsv(const float * sun_pos, float turbidity, std::string planetName);
	void get_xyY_Valuev(skylight_struct2& position) const;

private:
	float thetas;			// angular distance between the zenith and the sun in radian
	float T;				// Turbidity : i.e. sky "clarity"
	//  1 : pure air
	//  2 : exceptionnally clear
	//  4 : clear
	//  8 : light haze
	// 25 : haze
	// 64 : thin fog

	// Computed variables depending on the 2 above
	float zenith_luminance;		// Y color component of the CIE color at zenith (luminance)
	float zenith_color_x;		// x color component of the CIE color at zenith
	float zenith_color_y;		// y color component of the CIE color at zenith

	float eye_lum_conversion;	// luminance conversion for an eye adapted to screen luminance (around 40 cd/m^2)

	float AY, BY, CY, DY, EY;	// Distribution coefficients for the luminance distribution function
	float Ax, Bx, Cx, Dx, Ex;	// Distribution coefficients for x distribution function
	float Ay, By, Cy, Dy, Ey;	// Distribution coefficients for y distribution function

	float term_x;				// Precomputed term for x calculation
	float term_y;				// Precomputed term for y calculation
	float term_Y;				// Precomputed term for luminance calculation

	float sun_pos[3];

	// Compute CIE Y (luminance) for zenith in cd/m^2
	inline void computeZenithLuminance(void);
	// Compute CIE x and y color components
	inline void computeZenithEarthColor(void);
	inline void computeZenithVenusColor(void);
	inline void computeZenithMarsColor(void);
	// Compute the luminance distribution coefficients
	inline void computeLuminanceDistributionCoefs(void);
	// Compute the color distribution coefficients
	inline void computeColorDistributionCoefs(void);

};

#endif // _SKYLIGHT_H_
