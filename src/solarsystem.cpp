/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#include <algorithm>
#include <iostream>
#include <string>
#include <future>
#include "solarsystem.hpp"
#include "s_texture.hpp"
#include "orbit.hpp"
// #include "spacecrafter.hpp"
#include "navigator.hpp"
#include "projector.hpp"
#include "utility.hpp"
#include <fstream>
#include "log.hpp"
#include "fmath.hpp"
#include "app_settings.hpp"
#include "ring.hpp"
#include "perf_debug.hpp"
#include "objl_mgr.hpp"
#include "orbit_creator_cor.hpp"
#include "space_date.hpp"

#define SOLAR_MASS 1.989e30
#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22
#define MARS_MASS  0.64185e24

using namespace std;

static bool removeFromVector(SolarSystem::BodyContainer * bc, vector<SolarSystem::BodyContainer *> &vec){
	for(auto it = vec.begin(); it != vec.end();it++){
		if(bc->englishName == (*it)->englishName){
			vec.erase(it);
			return true;
		}		
	}
	return false;
}

SolarSystem::SolarSystem()
	:sun(nullptr),moon(nullptr),earth(nullptr),
	 moonScale(1.), planet_name_font(nullptr),
	 flagPlanetsOrbits(false),flagSatellitesOrbits(false),
	 flag_light_travel_time(false),flagHints(false),flagTrails(false)
{
	bodyTrace = nullptr;

	objLMgr = new ObjLMgr();
	objLMgr -> setDirectoryPath(AppSettings::Instance()->getModel3DDir() );
	objLMgr->insertDefault("Sphere");

	if (!objLMgr->checkDefaultObject()) {
		Log.write("SolarSystem: no default objMgr loaded, system aborded", cLog::LOG_TYPE::L_ERROR);
		exit(-7);
	}

	Body::createShader();
	Body::createDefaultAtmosphereParams();
	
	OrbitCreator * special = new OrbitCreatorSpecial(nullptr);
	OrbitCreator * comet = new OrbitCreatorComet(special, this);
	OrbitCreator * elip = new OrbitCreatorEliptic(comet, this);
	orbitCreator = new OrbitCreatorBary(elip, this);
}

void SolarSystem::setFont(float font_size, const std::string& font_name)
{
	if (planet_name_font) delete planet_name_font;

	planet_name_font = new s_font(font_size, font_name);
	if (!planet_name_font) {
		Log.write("Can't create planet_name_font", cLog::LOG_TYPE::L_ERROR);
		exit(-1);
	}
	Body::setFont(planet_name_font);
}

SolarSystem::~SolarSystem()
{
	// release selected:
	selected = Object();
	
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if(it->second->body != nullptr){
			delete it->second->body;
			delete it->second;
		}
	}	
	systemBodies.clear();
	
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
		//~ if ((*iter)->body) delete (*iter)->body;
		//~ (*iter)->body = nullptr;
		//~ delete (*iter);
	//~ }
	
	renderedBodies.clear();
	//~ system_bodys.clear();
	
	Body::deleteDefaultTexMap();
	Body::deleteDefaultatmosphereParams();
	Body::deleteShader();

	sun = nullptr;
	moon = nullptr;
	earth = nullptr;

	if (planet_name_font) delete planet_name_font;
	if (objLMgr) delete objLMgr;
}


// Init and load the solar system data
void SolarSystem::load(const std::string& planetfile)
{
	stringHash_t bodyParams;

	ifstream fileBody (planetfile.c_str() , std::ifstream::in);
	if(fileBody) {
		string ligne;
		while(getline(fileBody , ligne)) {
			if (ligne[0] != '[' ) {
				if (ligne[0]!='#' && ligne.size() != 0) {
					int pos = ligne.find('=',0);
					string p1=ligne.substr(0,pos-1);
					string p2=ligne.substr(pos+2,ligne.size());
					bodyParams[p1]=p2;
				}
			} else {
				if (bodyParams.size() !=0) {
					//TODO récupérer cette erreur s'il y en a !
					cout << addBody(bodyParams, false);  // config file bodies are not deletable
					bodyParams.clear();
				}
			}
		}
		fileBody.close();
	} else
		Log.write("Unable to open file "+ planetfile, cLog::LOG_TYPE::L_ERROR);

	Log.write("(solar system loaded)", cLog::LOG_TYPE::L_INFO);
	Log.mark();
}

void SolarSystem::iniTextures() 
{
	if( ! Body::setTexEclipseMap("bodies/eclipse_map.png")) {
		Log.write("no tex_eclipse_map valid !", cLog::LOG_TYPE::L_ERROR);
	}

	if( ! Body::setTexDefaultMap("bodies/nomap.png")) {
		Log.write("no tex_default_map valid !", cLog::LOG_TYPE::L_ERROR);
	}

	if( ! Body::setTexHaloMap("planethalo.png")) {
		Log.write("no tex_halo valid !", cLog::LOG_TYPE::L_ERROR);
	}

	Log.write("(solar system) default textures loaded", cLog::LOG_TYPE::L_INFO);
}


BODY_TYPE SolarSystem::setPlanetType (const std::string &str)
{
	if (str =="Sun") return SUN;
	else if (str == "Planet") return PLANET;
	else if (str == "Moon") return MOON;
	else if (str == "Dwarf") return DWARF;
	else if (str == "Asteroid") return ASTEROID;
	else if (str == "KBO") return KBO;
	else if (str == "Comet") return COMET;
	else if (str == "Artificial") return ARTIFICIAL;
	else if (str == "Observer") return OBSERVER;
	else
		return UNKNOWN;
}

Body* SolarSystem::findBody(const std::string &name)
{
	
	if(systemBodies.count(name) != 0){
		return systemBodies[name]->body;
	}
	else{
		return nullptr;
	}	
	
	//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter!=system_bodys.end(); iter++) {
		//~ if ((*iter)->englishName == name) {
			//~ return (*iter)->body;
		//~ }
	//~ }
	//~ return nullptr;
}

SolarSystem::BodyContainer* SolarSystem::findBodyContainer(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name];
	}
	else{		
		return nullptr;
	}	
	
	//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter!=system_bodys.end(); iter++) {
		//~ if ((*iter)->englishName == name) {
			//~ return (*iter);
		//~ }
	//~ }
	//~ return nullptr;
}

// Init and load one solar system object
// This is a the private method
string SolarSystem::addBody(stringHash_t & param, bool deletable)
{
	//~ AutoPerfDebug apd(&pd, "SolarSystem::addBody$"); //Debug
	BODY_TYPE typePlanet= UNKNOWN;
	const string englishName = param["name"];
	string str_parent = param["parent"];
	const string type_Body = param["type"];
	Body *parent = nullptr;

	Log.write("Loading new Solar System object... " + englishName, cLog::LOG_TYPE::L_INFO);
	//~ cout << "Loading new Solar System object... " << englishName << endl;
	//~ for ( stringHashIter_t iter = param.begin(); iter != param.end(); ++iter ) {
	//~ cout << iter->first << " : " << iter->second << endl;
	//~ }

	// set the Body type: ie what it is in universe
	typePlanet= setPlanetType(type_Body);

	// do not add if no name or no parent or no typePlanet
	if (englishName=="")
		return("Can not add body with no name");

	// no parent ? so it's Sun
	if (str_parent=="")
		str_parent = "Sun";

	// no type ? it's an asteroid
	if (typePlanet == UNKNOWN)
		typePlanet = ASTEROID;

	// Do not add if body already exists - name must be unique
	if ( findBody(englishName)!=nullptr )
		return (string("Can not add body named \"") + englishName + string("\" because a body of that name already exists\n"));

	if (str_parent!="none") {
		parent = findBody(str_parent);

		if (parent == nullptr) {
			string error = string("WARNING : can't find parent for ") + englishName;
			return error;
		}
	}

	const string funcname = param["coord_func"];

	//
	// determination de l'orbite de l'astre
	//
	Orbit* orb = nullptr;
	bool close_orbit = Utility::strToBool(param["close_orbit"], 1);

	// default value of -1 means unused
	double orbit_bounding_radius = Utility::strToDouble(param["orbit_bounding_radius"], -1);

	if (funcname=="earth_custom") {
		// Special case to take care of Earth-Moon Barycenter at a higher level than in ephemeris library

		//cout << "Creating Earth orbit...\n" << endl;
		Log.write("Creating Earth orbit...", cLog::LOG_TYPE::L_INFO);
		SpecialOrbit *sorb = new SpecialOrbit("emb_special");

		if (!sorb->isValid()) {
			string error = string("ERROR : can't find position function ") + funcname + string(" for ") + englishName + string("\n");
			Log.write(error, cLog::LOG_TYPE::L_ERROR);
			//~ cout << error << endl;
			return error;
		}

		// NB. moon has to be added later
		orb = new BinaryOrbit(sorb, 0.0121505677733761);

	} else if(funcname == "lunar_custom") {
		// This allows chaotic Moon ephemeris to be removed once start leaving acurate and sane range

		SpecialOrbit *sorb = new SpecialOrbit("lunar_special");

		if (!sorb->isValid()) {
			string error = string("ERROR : can't find position function ") + funcname + string(" for ") + englishName + string("\n");
			Log.write(error, cLog::LOG_TYPE::L_ERROR);
			return error;
		}

		orb = new MixedOrbit(sorb,
		                     Utility::strToDouble(param["orbit_period"]),
		                     SpaceDate::JulianDayFromDateTime(-10000, 1, 1, 1, 1, 1),
		                     SpaceDate::JulianDayFromDateTime(10000, 1, 1, 1, 1, 1),
		                     EARTH_MASS + LUNAR_MASS,
		                     0, 0, 0,
		                     false);

	} else if (funcname == "still_orbit") {
		orb = new stillOrbit(Utility::strToDouble(param["orbit_x"]),
		                     Utility::strToDouble(param["orbit_y"]),
		                     Utility::strToDouble(param["orbit_z"]));
	} else {

		orb = orbitCreator->handle(param);
		
		if(orb == nullptr) {
			cout << "something went wrong when creating orbit from "<< englishName << endl;
			Log.write("Error when creating orbit from " + englishName, cLog::LOG_TYPE::L_ERROR);
		}
	}
	
	if(param["coord_func"] == "ell_orbit"){
		orbit_bounding_radius = orb->getBoundingRadius();
	}
	
	//
	// fin détermination de l'orbite
	//

	BodyColor* bodyColor = nullptr;
	bodyColor = new BodyColor(param["color"], param["label_color"], param["orbit_color"], param["trail_color"]);

	float solLocalDay= Utility::strToDouble(param["sol_local_day"],1.0);

	// Create the Body and add it to the list
	// p est un pointeur utilisé pour l'objet qui sera au final intégré dans la liste des astres que gère body_mgr
	Body* p = nullptr;
	Sun *p_sun =nullptr;
	Moon *p_moon =nullptr;
	BigBody *p_big =nullptr;
	SmallBody *p_small =nullptr;
	Artificial *p_artificial = nullptr;
	ObjL* currentOBJ = nullptr;

	string modelName = param["model_name"];

	if ( !modelName.empty()) {
		objLMgr->insertObj(modelName);
		currentOBJ = objLMgr->select(modelName);
	}
	else
		currentOBJ = objLMgr->selectDefault();

	switch (typePlanet) {
		case SUN : {
			p_sun = new Sun(parent,
			                englishName,
			                Utility::strToBool(param["halo"]),
			                Utility::strToDouble(param["radius"])/AU,
			                Utility::strToDouble(param["oblateness"], 0.0),
			                bodyColor,
			                solLocalDay,
			                Utility::strToDouble(param["albedo"]),
			                param["tex_map"],
			                orb,
			                close_orbit,
			                currentOBJ,
			                //~ Utility::strToBool(param["hidden"], 0),
			                orbit_bounding_radius,
			                param["tex_normal"]
			               );
			//update of sun's big_halo texture
			string bighalotexfile = param["tex_big_halo"];
			if (!bighalotexfile.empty()) {
				p_sun->setBigHalo(bighalotexfile, param["path"]);
				p_sun->setHaloSize(Utility::strToDouble(param["big_halo_size"], 50.f));
			}

			if (englishName == "Sun") {
				sun = p_sun;
				bodyTrace = p_sun;
			}
			p = p_sun;
		}
		break;

		case ARTIFICIAL: {
			p_artificial = new Artificial(parent,
							  englishName,
							  Utility::strToBool(param["halo"]),
							  Utility::strToDouble(param["radius"])/AU,
			                  bodyColor,
			                  solLocalDay,
			                  Utility::strToDouble(param["albedo"]),
			                  orb,
			                  close_orbit,
			                  param["model_name"],
			                  //~ Utility::strToBool(param["hidden"], 0),
			                  deletable,
			                  orbit_bounding_radius);
			p=p_artificial;
			}
			break;

		case MOON: {
			p_moon = new Moon(parent,
			                  englishName,
			                  Utility::strToBool(param["halo"]),
			                  Utility::strToDouble(param["radius"])/AU,
			                  Utility::strToDouble(param["oblateness"], 0.0),
			                  bodyColor,
			                  solLocalDay,
			                  Utility::strToDouble(param["albedo"]),
			                  param["tex_map"],
			                  orb,
			                  close_orbit,
			                  currentOBJ,
			                  //~ Utility::strToBool(param["hidden"], 0),
			                  orbit_bounding_radius,
			                  param["tex_normal"]
			                 );
			if (englishName == "Moon") {
				moon = p_moon;
				if(earth) {
					BinaryOrbit *earthOrbit = dynamic_cast<BinaryOrbit *>(earth->getOrbit());
					if(earthOrbit) {
						Log.write("Adding Moon to Earth binary orbit.", cLog::LOG_TYPE::L_INFO);
						earthOrbit->setSecondaryOrbit(orb);
					} else
						Log.write(englishName + " body could not be added to Earth orbit.", cLog::LOG_TYPE::L_WARNING);
				} else
					Log.write(englishName + " body could not be added to Earth orbit calculation, position may be inacurate", cLog::LOG_TYPE::L_WARNING);
			}
			p=p_moon;
		}
		break;

		case DWARF:
		case PLANET: {
			p_big = new BigBody(parent,
			                    englishName,
			                    typePlanet,
			                    Utility::strToBool(param["halo"]),
			                    Utility::strToDouble(param["radius"])/AU,
			                    Utility::strToDouble(param["oblateness"], 0.0),
			                    bodyColor,
			                    solLocalDay,
			                    Utility::strToDouble(param["albedo"]),
			                    param["tex_map"],
			                    orb,
			                    close_orbit,
			                    currentOBJ,
			                    //~ Utility::strToBool(param["hidden"], 0),
			                    orbit_bounding_radius,
			                    param["tex_normal"],
			                    param["tex_night"],
			                    param["tex_specular"],
			                    param["tex_cloud"],
			                    param["tex_cloud_normal"]);
			if (Utility::strToBool(param["rings"], 0)) {
				const double r_min = Utility::strToDouble(param["ring_inner_size"])/AU;
				const double r_max = Utility::strToDouble(param["ring_outer_size"])/AU;
				Ring *r = new Ring(r_min,r_max,param["tex_ring"],/*param["path"],*/ ringsInit);
				p_big->setRings(r);
				p_big->updateBoundingRadii();
			}

			if(englishName=="Earth") {
				earth = p_big;
			}

			p = p_big;
		}
		break;

		case ASTEROID:
		case KBO:
		case COMET: {
			p_small = new SmallBody(parent,
			                        englishName,
			                        typePlanet,
			                        Utility::strToBool(param["halo"]),
			                        Utility::strToDouble(param["radius"])/AU,
			                        Utility::strToDouble(param["oblateness"], 0.0),
			                        bodyColor,
			                        solLocalDay,
			                        Utility::strToDouble(param["albedo"]),
			                        param["tex_map"],
			                        orb,
			                        close_orbit,
			                        currentOBJ,
			                        //~ Utility::strToBool(param["hidden"], 0),
			                        orbit_bounding_radius,
			                        param["tex_normal"]
			                       );
			p = p_small;
		}
		break;

		default:
			Log.write("Undefined body", cLog::LOG_TYPE::L_ERROR);
	}

	// détermination de l'atmosphère extérieure
	// if(!param["has_atmosphere"].empty() && Utility::strToBool(param["has_atmosphere"]) == true){
	// 	//~ bool hasAtmExt = Utility::strToBool(param["has_atmosphere"]);
	// 	std::string gradient = param["atmosphere_gradient"];
	// 	float radiusRatio = Utility::strToDouble(param["atmosphere_radius_ratio"]);

	// 	cout << "radiusRatio " << radiusRatio << " gradient " << gradient << endl;

	// 	if (/*!gradient.empty() &&*/ radiusRatio!=0.f)
	// 		p->setAtmExt(radiusRatio, gradient);
	// }
	if (!param["has_atmosphere"].empty()) {
		AtmosphereParams* tmp = nullptr;
		tmp = new(AtmosphereParams);
		tmp->hasAtmosphere = Utility::strToBool(param["has_atmosphere"], false);
		tmp->limInf = Utility::strToFloat(param["atmosphere_lim_inf"], 40000.f);
		tmp->limSup = Utility::strToFloat(param["atmosphere_lim_sup"], 80000.f);
		tmp->limLandscape = Utility::strToFloat(param["atmosphere_lim_landscape"], 10000.f);
		p->setAtmosphereParams(tmp);
	}


	// Use J2000 N pole data if available
	double rot_obliquity = Utility::strToDouble(param["rot_obliquity"],0.)*C_PI/180.;
	double rot_asc_node  = Utility::strToDouble(param["rot_equator_ascending_node"],0.)*C_PI/180.;

	// In J2000 coordinates
	double J2000_npole_ra = Utility::strToDouble(param["rot_pole_ra"],0.)*C_PI/180.;
	double J2000_npole_de = Utility::strToDouble(param["rot_pole_de"],0.)*C_PI/180.;

	// NB: north pole needs to be defined by right hand rotation rule
	if (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "") {
		// cout << "Using north pole data for " << englishName << endl;

		Vec3d J2000_npole;
		Utility::spheToRect(J2000_npole_ra,J2000_npole_de,J2000_npole);

		Vec3d vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));

		double ra, de;
		Utility::rectToSphe(&ra, &de, vsop87_pole);

		rot_obliquity = (C_PI_2 - de);
		rot_asc_node = (ra + C_PI_2);

		//cout << "\tCalculated rotational obliquity: " << rot_obliquity*180./C_PI << endl;
		//cout << "\tCalculated rotational ascending node: " << rot_asc_node*180./C_PI << endl;
	}

	p->set_rotation_elements(
	    Utility::strToDouble(param["rot_periode"], Utility::strToDouble(param["orbit_period"], 24.))/24.,
	    Utility::strToDouble(param["rot_rotation_offset"],0.),
	    Utility::strToDouble(param["rot_epoch"], J2000),
	    rot_obliquity,
	    rot_asc_node,
	    Utility::strToDouble(param["rot_precession_rate"],0.)*C_PI/(180*36525),
	    Utility::strToDouble(param["orbit_visualization_period"],0.),
	    Utility::strToDouble(param["axial_tilt"],0.) );

	// Clone current flags to new body unless one is currently selected
	p->setFlagHints(getFlagHints());
	p->setFlagTrail(getFlagTrails());

	if (!selected || selected == Object(sun)) {
		p->setFlagOrbit(getFlagOrbits());
	}
	
	BodyContainer  * container = new BodyContainer();
	container->body = p;
	container->englishName = englishName;
	container->isDeleteable = deletable;
	container->isHidden = Utility::strToBool(param["hidden"], 0);
	container->initialHidden = container->isHidden;
	
	systemBodies.insert(pair<string, BodyContainer *>(englishName, container));
	
	if(!container->isHidden){
		renderedBodies.push_back(container);
	}	
	
	//~ BodyContainer* tmp = nullptr;
	//~ tmp = new BodyContainer {p,englishName, deletable};

	anchorManager->addAnchor(englishName, p);	
	//~ system_bodys.push_back(tmp);
	p->updateBoundingRadii();
	return("");  // OK

}

bool SolarSystem::removeBodyNoSatellite(const std::string &name){


	cout << "removing : " << name << endl;

	BodyContainer * bc = findBodyContainer(name);
	
	
	
	
	if(bc == nullptr){
		Log.write("SolarSystem::removeBodyNoSatellite : Could not find a body named : " + name );
		return false;
	}
		
	//check if the body was a satellite
	if(bc->body->getParent() != nullptr){
		bc->body->getParent()->removeSatellite(bc->body);
	}
	
	//remove from containers :
	systemBodies.erase(bc->englishName);
	if(!bc->isHidden){
		removeFromVector(bc, renderedBodies);
	}
	
	anchorManager->removeAnchor(bc->body);
	delete bc->body;
	delete bc;
	
	return true;

}

bool SolarSystem::removeBody(const std::string &name){
	
	BodyContainer * bc = findBodyContainer(name);
	
	if(bc == nullptr){
		Log.write("SolarSystem::removeBody : Could not find a body named : " + name );
		return false;
	}
	
	if(bc->body->hasSatellite()){
		
		vector<string> names;		
		list<Body *> satellites = bc->body->getSatellites();
				
		for(auto it = satellites.begin(); it != satellites.end(); it++){
			names.push_back((*it)->getEnglishName());		
		}
		
		for(string satName : names){
			if(!removeBody(satName)){
				Log.write("SolarSystem::removeBody : Could not remove satelite : " + satName );
			}
		}		
	}
	
	return removeBodyNoSatellite(name);
			
}

bool SolarSystem::removeSupplementalBodies(const std::string &name){	
	
	BodyContainer * bc = findBodyContainer(name);
		
	if(bc == nullptr){
		Log.write("SolarSystem::removeSupplementalBodies : Could not find a body named : " + name );
		return false;
	}
	
	if(bc->isDeleteable){
		Log.write("SolarSystem::removeSupplementalBodies : Can't distroy suplementary bodies if attached to one");
		return false;
	}
	
	vector<string> names;
	
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		
		if(it->second->isDeleteable){
			
			if(!it->second->body->getParent()){
				names.push_back(it->first);
			}
			else{				
				vector<string>::iterator ite;
				
				ite = find(names.begin(), names.end(),it->first);
				
				if(ite == names.end()){
					names.push_back(it->first);
				}
			}								
		}
	}
	
	for(string name : names){
		removeBody(name);
	}	
	
	return true;
}

void SolarSystem::initialSolarSystemBodies(){
	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		it->second->body->reinitParam();
		if (it->second->isHidden != it->second->initialHidden) {
			if(it->second->initialHidden){
				removeFromVector(it->second,renderedBodies);
			}
			else{
				renderedBodies.push_back(it->second);
			}
			it->second->isHidden = it->second->initialHidden;
		}
	}
}

void SolarSystem::toggleHideSatellites(bool val){

	val = !val;

	if(flagHideSatellites == val){
		return;
	}
	else{
		flagHideSatellites = val;
	}
	
	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
	
		//If we are a planet with satellites
		if(it->second->body->getParent() && 
		   it->second->body->getParent()->getEnglishName() == "Sun" &&
		   it->second->body->hasSatellite()){
			   
		   for(Body * satellite : it->second->body->getSatellites()){
			   BodyContainer * sat = findBodyContainer(satellite->getEnglishName());			   
				setPlanetHidden(sat->englishName, val);
			}
		}
	}	
}


void SolarSystem::setPlanetHidden(const std::string &name, bool planethidden)
{
	
	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		Body * body = it->second->body;
		if ( 
			body->getEnglishName() == name ||
			(body->get_parent() && body->get_parent()->getEnglishName() == name) ){
			
			//~ body->set_planet_hidden(planethidden);//bound to disapear
			
			it->second->isHidden = planethidden;
			
			if(planethidden){
				removeFromVector(it->second,renderedBodies);
			}
			else{
				renderedBodies.push_back(it->second);
			}
						
		}
	}
	
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter!=system_bodys.end(); iter++) {
		//~ if ( (*iter)->englishName == name || ((*iter)->body->get_parent() && (*iter)->body->get_parent()->getEnglishName() == name  ) )
			//~ (*iter)->body->set_planet_hidden(planethidden);
	//~ }
}

bool SolarSystem::getPlanetHidden(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name]->isHidden;
	}
	else{
		return false;
	}
	
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter!=system_bodys.end(); iter++) {
		//~ if ( (*iter)->englishName == name ) {
			//~ return (*iter)->body->get_planet_hidden();
		//~ }
	//~ }
	//~ return false;
}

// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystem::computePositions(double date,const Observer *obs)
{
	//~ AutoPerfDebug apd(&pd, "SolarSystem::computePositions"); //Debug

	if (flag_light_travel_time) {
		const Vec3d home_pos(obs->getHeliocentricPosition(date));
		//~ for (vector<BodyContainer*>::const_iterator iter(system_bodys.begin()); iter!=system_bodys.end(); iter++) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			const double light_speed_correction =
			    (it->second->body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			it->second->body->compute_position(date-light_speed_correction);
		}
	} else {
		//~ for (vector<BodyContainer*>::const_iterator iter(system_bodys.begin()); iter!=system_bodys.end(); iter++) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			it->second->body->compute_position(date);
		}
	}

	computeTransMatrices(date, obs);

}

// Compute the transformation matrix for every elements of the solar system.
// The elements have to be ordered hierarchically, eg. it's important to compute earth before moon.
void SolarSystem::computeTransMatrices(double date,const Observer * obs)
{
	if (flag_light_travel_time) {
		const Vec3d home_pos(obs->getHeliocentricPosition(date));
		//~ for (vector<BodyContainer*>::const_iterator iter(system_bodys.begin()); iter!=system_bodys.end(); iter++) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			const double light_speed_correction =
			    (it->second->body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			it->second->body->compute_trans_matrix(date-light_speed_correction);
		}
	} else {
		//~ for (vector<BodyContainer*>::const_iterator iter(system_bodys.begin()); iter!=system_bodys.end(); iter++) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			it->second->body->compute_trans_matrix(date);
		}
	}
}

void SolarSystem::computePreDraw(const Projector * prj, const Navigator * nav){
	
	if (!getFlagPlanets()) 
		return; // 0;
	
	//~ std::cout << "Debut de SolarSystem::draw" << std::endl;

	// Compute each Body distance to the observer
	Vec3d obs_helio_pos = nav->getObserverHelioPos();
	//	cout << "obs: " << obs_helio_pos << endl;

	//~ pd.startTimer("SolarSystem::draw$compute_dist+magn+draw"); //Debug
	for (auto it = renderedBodies.begin(); it != renderedBodies.end(); it++){
		(*it)->body->compute_distance(obs_helio_pos);
		(*it)->body->computeMagnitude(obs_helio_pos);
		(*it)->body->computeDraw(prj, nav);
	}
	//~ pd.stopTimer("SolarSystem::draw$compute_dist+magn+draw"); //Debug

	//~ pd.startTimer("SolarSystem::draw$sort"); //Debug
	// sort all body from the furthest to the closest to the observer
	//~ sort(system_bodys.begin(),system_bodys.end(),biggerDistance);
	
	sort(renderedBodies.begin(), renderedBodies.end(), biggerDistance);
	
	//~ pd.stopTimer("SolarSystem::draw$sort"); //Debug

	// Determine optimal depth buffer buckets for drawing the scene
	// This is similar to Celestia, but instead of using ranges within one depth
	// buffer we just clear and reuse the entire depth buffer for each bucket.
	double znear, zfar;
	double lastNear = 0;
	double lastFar = 0;
	int nBuckets = 0;
	listBuckets.clear();
	depthBucket db;

	//~ pd.startTimer("SolarSystem::draw$loop1"); //Debug
	for (auto it = renderedBodies.begin(); it!= renderedBodies.end();it++) {
		if ( (*it)->body->get_parent() == sun

		        // This will only work with natural planets
		        // and not some illustrative (huge) artificial planets for example
		        // && (!(*iter)->isSimplePlanet() || *iter == home_planet)
		        && (*it)->body->get_on_screen_bounding_size(prj, nav) > 3 ) {

			//~ std::cout << "Calcul de bucket pour " << (*iter)->englishName << std::endl;

			double dist = (*it)->body->getEarthEquPos(nav).length();  // AU

			double bounding = (*it)->body->getBoundingRadius() * 1.01;

			if ( bounding >= 0 ) {
				// this is not a hidden object

				znear = dist - bounding;
				zfar  = dist + bounding;

				if (znear < 0.001){
					znear = 0.00000001;
				}
				else{
					if (znear < 0.05) {
						znear *= 0.1;
					}
					else{
						if (znear < 0.5){
							znear *= 0.2;
						}
					}
				}

				// see if overlaps previous bucket
				// TODO check that buffer isn't too deep
				if ( nBuckets > 0 && zfar > lastNear ) {
					// merge with last bucket

					//cout << "merged buckets " << (*iter)->getEnglishName() << " " << znear << " " << zfar << " with " << lastNear << " " << lastFar << endl;
					db = listBuckets.back();

					if(znear < lastNear ) {
						// Artificial planets may cover real planets, for example
						lastNear = db.znear = znear;
					}

					if ( zfar > lastFar ) {
						lastFar = db.zfar = zfar;
					}

					listBuckets.pop_back();
					listBuckets.push_back(db);

				} else {

					// create a new bucket
					//cout << "New bucket: " << (*iter)->getEnglishName() << znear << " zfar: " << zfar << endl;
					lastNear = db.znear = znear;
					lastFar  = db.zfar  = zfar;
					nBuckets++;
					listBuckets.push_back( db );
				}
			}
		}
	}
	//~ pd.stopTimer("SolarSystem::draw$loop1"); //Debug
}

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
void SolarSystem::draw(Projector * prj, const Navigator * nav, const Observer* observatory, const ToneReproductor* eye, /*bool flag_point,*/ bool drawHomePlanet)
{
	//~ AutoPerfDebug apd(&pd, "SolarSystem::draw$"); //Debug
	if (!getFlagPlanets()) 
		return; // 0;
	
	int nBuckets = listBuckets.size();
	
	list<depthBucket>::iterator dbiter;

	//~ cout << "***\n";
	//~ dbiter = listBuckets.begin();
	//~ while( dbiter != listBuckets.end() ) {
	//~ cout << (*dbiter).znear << " " << (*dbiter).zfar << endl;
	//~ dbiter++;
	//~ }
	//~ cout << "***\n";

	// Draw the elements
	double z_near, z_far;
	prj->getClippingPlanes(&z_near,&z_far); // Save clipping planes

	dbiter = listBuckets.begin();

	// clear depth buffer
	prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
	glClear(GL_DEPTH_BUFFER_BIT);

	//float depthRange = 1.0f/nBuckets;
	float currentBucket = nBuckets - 1;

	// economize performance by not clearing depth buffer for each bucket... good?
	//	cout << "\n\nNew depth rendering loop\n";
	bool depthTest = true;  // small objects don't use depth test for efficiency
	double dist;

	bool needClearDepthBuffer = false;

	//~ pd.startTimer("SolarSystem::draw$loop2"); //Debug
	for (auto it = renderedBodies.begin(); it != renderedBodies.end(); it++) {
		////pd.startTimer("SolarSystem::draw$loop2$part1"); //Debug
		dist = (*it)->body->getEarthEquPos(nav).length();
		if (dist < (*dbiter).znear ) {
			//~ std::cout << "Changement de bucket pour " << (*iter)->englishName << " qui a pour parent " << (*iter)->body->getParent()->getEnglishName() << std::endl;
			//~ std::cout << "Changement de bucket pour " << (*iter)->englishName << std::endl;

			// potentially use the next depth bucket
			dbiter++;

			if (dbiter == listBuckets.end() ) {
				dbiter--;
				// now closer than the first depth buffer
			} else {
				currentBucket--;

				// TODO: evaluate performance tradeoff???
				// glDepthRange(currentBucket*depthRange, (currentBucket+1)*depthRange);
				if (needClearDepthBuffer) {
					glClear(GL_DEPTH_BUFFER_BIT);
					needClearDepthBuffer = false;
				}

				// get ready to start using
				prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
			}
		}
		////pd.stopTimer("SolarSystem::draw$loop2$part1"); //Debug
		////pd.startTimer("SolarSystem::draw$loop2$part2"); //Debug
		if (dist > (*dbiter).zfar || dist < (*dbiter).znear) {
			// don't use depth test (outside buckets)
			//~ std::cout << "Outside bucket pour " << (*iter)->englishName << std::endl;

			if ( depthTest )
				prj->setClippingPlanes(z_near, z_far);
			depthTest = false;

		} else {
			if (!depthTest)
				prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);

			depthTest = true;
			//~ std::cout << "inside bucket pour " << (*iter)->englishName << std::endl;
		}
		////pd.stopTimer("SolarSystem::draw$loop2$part2"); //Debug
		//~ double squaredDistance = 0;
		////pd.startTimer("SolarSystem::draw$loop2$draw"); //Debug
		//~ if ((*iter)->body==moon && nearLunarEclipse(nav, prj)) {

			//~ // TODO: moon magnitude label during eclipse isn't accurate...
			//~ // special case to update stencil buffer for drawing lunar eclipses
			//~ glClear(GL_STENCIL_BUFFER_BIT);
			//~ glClearStencil(0x0);

			//~ glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			//~ glStencilOp(GL_ZERO, GL_REPLACE, GL_REPLACE);

			//~ (*iter)->body->drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, selected == (*iter)->body);

		//~ } else {
		//~ if (depthTest)
			//~ cout << "tracé depthTest activé du body " << (*iter)->englishName << std::endl;
		//~ else
			//~ cout << "tracé depthTest déactivé du body " << (*iter)->englishName << std::endl;

		needClearDepthBuffer = (*it)->body->drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, selected == (*it)->body);
		//~ }
		////pd.stopTimer("SolarSystem::draw$loop2$draw"); //Debug
	}
	//~ pd.stopTimer("SolarSystem::draw$loop2"); //Debug
	prj->setClippingPlanes(z_near,z_far);  // Restore old clipping planes
	//~ std::cout << "Fin de SolarSystem::draw" << std::endl;
}

Body* SolarSystem::searchByEnglishName(const std::string &planetEnglishName) const
{
	//printf("SolarSystem::searchByEnglishName(\"%s\"): start\n", planetEnglishName.c_str());
	// side effect - bad?
	//	transform(planetEnglishName.begin(), planetEnglishName.end(), planetEnglishName.begin(), ::tolower);
	//~ for(vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
		//~ //printf("SolarSystem::searchByEnglishName(\"%s\"): %s\n",
		//~ //       planetEnglishName.c_str(), (*iter)->getEnglishName().c_str());
		//~ if ( (*iter)->englishName == planetEnglishName ) 
			//~ return (*iter)->body; // also check standard ini file names
	//~ }
	//~ //printf("SolarSystem::searchByEnglishName(\"%s\"): not found\n", planetEnglishName.c_str());
	//~ return nullptr;
	
	if(systemBodies.count(planetEnglishName) != 0){
		return systemBodies.find(planetEnglishName)->second->body;
	}
	else{
		return nullptr;
	}
	
}

Object SolarSystem::searchByNamesI18(const std::string &planetNameI18) const
{
	// side effect - bad?
	//	transform(planetNameI18.begin(), planetNameI18.end(), planetNameI18.begin(), ::tolower);
	//~ for(vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if ( it->second->body->getNameI18n() == planetNameI18 ) 
			return it->second->body; // also check standard ini file names
	}
	return nullptr;
}

// Search if any Body is close to position given in earth equatorial position and return the distance
Object SolarSystem::search(Vec3d pos, const Navigator * nav, const Projector * prj) const
{
	pos.normalize();
	Body * closest = nullptr;
	double cos_angle_closest = 0.;
	static Vec3d equPos;

	//~ for(vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		equPos = it->second->body->getEarthEquPos(nav);
		equPos.normalize();
		double cos_ang_dist = equPos[0]*pos[0] + equPos[1]*pos[1] + equPos[2]*pos[2];
		if (cos_ang_dist>cos_angle_closest) {
			closest = it->second->body;
			cos_angle_closest = cos_ang_dist;
		}
	}

	if (cos_angle_closest>0.999) {
		return closest;
	} 
	else 
		return nullptr;
}

// Return a stl vector containing the planets located inside the lim_fov circle around position v
vector<Object> SolarSystem::searchAround(Vec3d v,
        double lim_fov,
        const Navigator * nav,
        const Observer* observatory,
        const Projector * prj,
        bool *default_last_item,
        bool aboveHomeBody ) const
{
	vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * C_PI/180.);
	static Vec3d equPos;
	// getHomeBody peut renvoyer nullptr, on ne l'utilise que pour des tests.
	//~ const Body *home_Body = observatory->getHomePlanet();
	const Body *home_Body = observatory->getHomeBody();

	*default_last_item = false;

	// Should still be sorted by distance from farthest to closest
	// So work backwards to go closest to furthest

	//~ vector<BodyContainer*>::const_iterator iter = system_bodys.end();
	//~ while (iter != system_bodys.begin()) {
		//~ iter--;
		
	for(auto it = systemBodies.rbegin(); it != systemBodies.rend(); it++){//reverse order

		equPos = it->second->body->getEarthEquPos(nav);
		equPos.normalize();

		// First see if within a Body disk
		if (it->second->body != home_Body || aboveHomeBody) {
			// Don't want home Body too easy to select unless can see it

			double angle = acos(v*equPos) * 180.f / C_PI;

			  //~ cout << "Big testing " << (*iter)->getEnglishName()
			  //~ << " angle: " << angle << " screen_angle: "
			  //~ << (*iter)->get_angular_size(prj, nav)/2.f
			  //~ << endl;

			if ( angle < it->second->body->get_angular_size(prj, nav)/2.f ) {

				// If near planet, may be huge but hard to select, so check size
				result.push_back(it->second->body);
				*default_last_item = true;

				break;  // do not want any planets behind this one!

			}
		}

		// See if within area of interest
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {
			result.push_back(it->second->body);
		}

	}
	return result;
}

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void SolarSystem::translateNames(Translator& trans)
{
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->translateName(trans);
	}

	if(planet_name_font) 
		planet_name_font->clearCache();
}

vector<string> SolarSystem::getNamesI18(void)
{
	vector<string> names;
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter)
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		names.push_back(it->second->body->getNameI18n());
	}
	return names;
}

std::string SolarSystem::getPlanetsPosition()
{
	string msg;
	Vec3d tmp;

	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (it->second->body->isSatellite())
			continue;
		tmp = it->second->body->get_heliocentric_ecliptic_pos();
		tmp[0]= round(tmp[0]*100);
		tmp[1]= round(tmp[1]*100);
		tmp[2]= round(tmp[2]*100);
		msg= msg+it->second->body->getNameI18n()+":"+Utility::doubleToStr(tmp[0])+":"+Utility::doubleToStr(tmp[1])+":"+Utility::doubleToStr(tmp[2])+";";
	}
	return msg;
}


// returns a newline delimited hash of localized:standard Body names for tui
// Body translated name is PARENT : NAME
string SolarSystem::getPlanetHashString(void)
{
	ostringstream oss;
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (!it->second->isDeleteable ) { // no supplemental bodies in list
			if (it->second->body->get_parent() != nullptr && it->second->body->get_parent()->getEnglishName() != "Sun") {
				oss << Translator::globalTranslator.translateUTF8(it->second->body->get_parent()->getEnglishName())
				    << " : ";
			}

			oss << Translator::globalTranslator.translateUTF8(it->second->englishName) << "\n";
			oss << it->second->englishName << "\n";
		}
	}
	return oss.str();

}

void SolarSystem::startTrails(bool b)
{
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->startTrail(b);
	}
}

void SolarSystem::setFlagTrails(bool b)
{
	flagTrails = b;

	if (!b || !selected || selected == Object(sun)) {
		//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ )
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			it->second->body->setFlagTrail(b);
		}
	} else {
		// if a Body is selected and trails are on, fade out non-selected ones
		//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			if (selected == it->second->body || (it->second->body->get_parent() && it->second->body->get_parent()->getEnglishName() == selected.getEnglishName()) )
				it->second->body->setFlagTrail(b);
			else it->second->body->setFlagTrail(false);
		}
	}
}

void SolarSystem::setFlagAxis(bool b)
{
	flagAxis=b;
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ )
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->setFlagAxis(b);
	}
}

void SolarSystem::setFlagHints(bool b)
{
	flagHints = b;
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->setFlagHints(b);
	}
}

void SolarSystem::setFlagPlanetsOrbits(bool b)
{
	flagPlanetsOrbits = b;

	if (!b || !selected || selected == Object(sun)) {
		//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			if (it->second->body->get_parent() && it->second->body->getParent()->getEnglishName() =="Sun")
				it->second->body->setFlagOrbit(b);
		}
	} else {
		// if a Body is selected and orbits are on,
		// fade out non-selected ones
		// unless they are orbiting the selected Body 20080612 DIGITALIS
		//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			if (!it->second->body->isSatellite()) {
				if ((selected == it->second->body) && (it->second->body->getParent()->getEnglishName() =="Sun")){
					it->second->body->setFlagOrbit(true);
				}
				else {
					it->second->body->setFlagOrbit(false);
				}
			} 
			else {
				if (selected == it->second->body->getParent()) {
					it->second->body->setFlagOrbit(true);
				}
				else{ 
					it->second->body->setFlagOrbit(false);
				}
			}
		}
	}
}

void SolarSystem::setFlagPlanetsOrbits(const std::string &_name, bool b)
{
	//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (it->second->englishName == _name) {
			it->second->body->setFlagOrbit(b);
			return;
		}
	}
}


void SolarSystem::setFlagSatellitesOrbits(bool b)
{
	flagSatellitesOrbits = b;

	if (!b || !selected || selected == Object(sun)) {
		//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			if (it->second->body->get_parent() && it->second->body->getParent()->getEnglishName() !="Sun"){
				it->second->body->setFlagOrbit(b);
			}
		}
	}
	else {
		// if the mother Body is selected orbits are on, else orbits are off
		//~ for (vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++ ) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			if (it->second->body->isSatellite()) {
				if (it->second->body->get_parent()->getEnglishName() == selected.getEnglishName() || it->second->englishName == selected.getEnglishName()) {
					it->second->body->setFlagOrbit(true);
				} 
				else{
					it->second->body->setFlagOrbit(false);
				}
			}
		}
	}
}

void SolarSystem::setSelected(const Object &obj)
{
	if (obj.getType() == /*ObjectRecord::*/OBJECT_BODY){
		selected = obj;
	}
	else{
		selected = Object();
	}
	// Undraw other objects hints, orbit, trails etc..
	setFlagOrbits(getFlagOrbits());
	setFlagTrails(getFlagTrails());  // TODO should just hide trail display and not affect data collection
}

void SolarSystem::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++) {
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->update(delta_time, nav, timeMgr);
	}
}

float SolarSystem::getPlanetSizeScale(const std::string &name)
{
	
	Body * body = searchByEnglishName(name);
	
	return body == nullptr ? 1.0 : body->getSphereScale();
	
	//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++) {
		//~ if ((*iter)->englishName == name) {
			//~ return (*iter)->body->getSphereScale();
			//~ break;
		//~ }
	//~ }
	//~ return 1.0;
}

void SolarSystem::setPlanetSizeScale(const std::string &name, float s)
{
	Body * body = searchByEnglishName(name);
	
	if(body != nullptr){
		body->setSphereScale(s);
	}
	
	//~ for(vector<BodyContainer*>::iterator iter = system_bodys.begin(); iter != system_bodys.end(); iter++) {
		//~ if ((*iter)->englishName == name) {
			//~ (*iter)->body->setSphereScale(s);
			//~ break;
		//~ }
	//~ }
}

// is a lunar eclipse close at hand?
bool SolarSystem::nearLunarEclipse(const Navigator * nav, Projector *prj)
{
	// TODO: could replace with simpler test

	Vec3d e = getEarth()->get_ecliptic_pos();
	Vec3d m = getMoon()->get_ecliptic_pos();  // relative to earth
	Vec3d mh = getMoon()->get_heliocentric_ecliptic_pos();  // relative to sun

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if (mdist.length() > r_penumbra + 2000/AU) return 0;  // not visible so don't bother drawing

	return 1;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
vector<string> SolarSystem::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	//~ for (vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
	
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
	
		string constw = it->second->body->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back(it->second->body->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}


void SolarSystem::bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const
{
	bodyTrace->getAltAz(nav,alt,az);
}

void SolarSystem::bodyTraceBodyChange(const std::string &bodyName)
{
	Body * body = searchByEnglishName(bodyName);
	
	if(body != nullptr){
		bodyTrace = body;
	}
	else{
		Log.write("Unknown planet_name in bodyTraceBodyChange", cLog::LOG_TYPE::L_ERROR);
	}	
	
	//~ for (vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
		//~ if ((*iter)->englishName == bodyName) {
			//~ bodyTrace = (*iter)->body;
			//~ break;
		//~ }
	//~ }
	//~ Log.write("Unknown planet_name in bodyTraceBodyChange", cLog::LOG_TYPE::L_ERROR);
}


void SolarSystem::setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c)
{
	if (englishName=="all")
		//~ for (vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
		for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
			it->second->body->setColor(colorName,c);
		}
	else{
		
		Body * body = searchByEnglishName(englishName);
		
		if(body != nullptr){
			body->setColor(colorName, c);
		}
		
		//~ for (vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
			//~ if ((*iter)->englishName == englishName) {
				//~ (*iter)->body->setColor(colorName,c);
				//~ break;
			//~ }
		//~ }
	}
}

const Vec3f SolarSystem::getBodyColor(const std::string &englishName, const std::string& colorName) const
{
	Body * body = searchByEnglishName(englishName);
	
	if(body != nullptr){
		return body->getColor(colorName);
	}
	else{
		return v3fNull;
	}
	
	//~ for (vector<BodyContainer*>::const_iterator iter = system_bodys.begin(); iter != system_bodys.end(); ++iter) {
		//~ if ((*iter)->englishName == englishName) {
			//~ return (*iter)->body->getColor(colorName);
		//~ }
	//~ }
	//~ return v3fNull;
}

void SolarSystem::setDefaultBodyColor(const std::string& colorName, const Vec3f& c){
	BodyColor::setDefault(colorName, c);
}

const Vec3f SolarSystem::getDefaultBodyColor(const std::string& colorName) const{
	return BodyColor::getDefault(colorName);
}
