/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#ifndef _STAR_WRAPPER_HPP_
#define _STAR_WRAPPER_HPP_

#include "object_base.hpp"
#include "navigator.hpp"
#include "hip_star_mgr.hpp"
#include "hip_star.hpp"
#include "matrix4.hpp"


namespace BigStarCatalog {

template <class Star> struct SpecialZoneArray;
template <class Star> struct SpecialZoneData;


//! A Star (Star1,Star2,Star3,...) cannot be a StelObject. The additional
//! overhead of having a dynamic type would simply be too much.
//! Therefore the StarWrapper is needed when returning Stars as StelObjects,
//! e.g. for searching, and for constellations.
//! The StarWrapper is destroyed when it is not needed anymore, by utilizing
//! reference counting and boost::intrusive_ptr. So there is no chance that
//! more than a few hundreds of StarWrappers are alive simultanousely.
//! Another reason for having the StarWrapper is to encapsulate the differences
//! between the different kinds of Stars (Star1,Star2,Star3).
class StarWrapperBase : public ObjectBase {
protected:
	StarWrapperBase(void) : ref_count(0) {}
	virtual ~StarWrapperBase(void) {}

	virtual OBJECT_TYPE getType(void) const {
		return OBJECT_STAR;
	}

	std::string getEnglishName(void) const {
		return "";
	}

	std::string getNameI18n(void) const = 0;
	std::string getInfoString(const Navigator *nav) const;
	std::string getShortInfoString(const Navigator *nav) const;
	std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const;
	void getRaDeValue(const Navigator *nav ,double *ra, double *de) const;
	virtual float getBV(void) const = 0;
private:
	int ref_count;
	void retain(void) {
		assert(ref_count>=0);
		ref_count++;
	}
	void release(void) {
		assert(ref_count>0);
		if (--ref_count==0) delete this;
	}
};

template <class Star>
class StarWrapper : public StarWrapperBase {
protected:
	StarWrapper(const SpecialZoneArray<Star> *a, const SpecialZoneData<Star> *z, const Star *s) : a(a),z(z),s(s) {}
	Vec3d getObsJ2000Pos(const Navigator*) const {
		const double d2000 = 2451545.0;
		return s->getJ2000Pos(z,
		                      (C_PI/180)*(0.0001/3600)
		                      * ((HipStarMgr::getCurrentJDay()-d2000)/365.25)
		                      / a->star_position_scale
		                     );
	}
	Vec3d getEarthEquPos(const Navigator *nav) const {
		return nav->j2000ToEarthEqu(getObsJ2000Pos(nav));
	}
	Vec3f getRGB(void) const {
		return HipStarMgr::color_table[s->getBVIndex()];
	}
	float getMag(const Navigator *nav) const {
		return 0.001f*a->mag_min + s->getMag()*(0.001f*a->mag_range)/a->mag_steps;
	}
	float getSelectPriority(const Navigator *nav) const {
		return getMag(nav);
	}
	float getBV(void) const {
		return s->getBV();
	}
	std::string getEnglishName(void) const {
		return "";
	}
	std::string getNameI18n(void) const {
		return s->getNameI18n();
	}

protected:
	const SpecialZoneArray<Star> *const a;
	const SpecialZoneData<Star> *const z;
	const Star *const s;
};


class StarWrapper1 : public StarWrapper<Star1> {
public:
	StarWrapper1(const SpecialZoneArray<Star1> *a, const SpecialZoneData<Star1> *z, const Star1 *s) : StarWrapper<Star1>(a,z,s) {}
	std::string getInfoString(const Navigator *nav) const;
	void getRaDeValue(const Navigator *nav, double *ra, double *de) const;
	std::string getShortInfoString(const Navigator *nav) const;
	std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const;
	std::string getEnglishName(void) const;
	float getStarDistance( void );
};

class StarWrapper2 : public StarWrapper<Star2> {
public:
	StarWrapper2(const SpecialZoneArray<Star2> *a, const SpecialZoneData<Star2> *z, const Star2 *s) : StarWrapper<Star2>(a,z,s) {}
};

class StarWrapper3 : public StarWrapper<Star3> {
public:
	StarWrapper3(const SpecialZoneArray<Star3> *a, const SpecialZoneData<Star3> *z, const Star3 *s) : StarWrapper<Star3>(a,z,s) {}
};

} // namespace BigStarCatalog

#endif
