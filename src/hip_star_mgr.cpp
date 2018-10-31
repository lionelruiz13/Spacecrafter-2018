/*
 * Spacecrafter
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2013 of the LSS team
 *
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006
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

// class used to manage groups of Stars

#include "projector.hpp"
#include "hip_star_mgr.hpp"
#include "object.hpp"
#include "object_base.hpp"
#include "s_texture.hpp"
#include "s_font.hpp"
#include "navigator.hpp"
#include "utility.hpp"
#include "tone_reproductor.hpp"
#include "geodesic_grid.hpp"
#include "translator.hpp"
//~ #include "app.hpp"
#include "init_parser.hpp"
//~ #include "core.hpp"
#include "app_settings.hpp"

#include "zone_array.hpp"
#include "string_array.hpp"

#include "time_mgr.hpp"

#include "log.hpp"

#include <string>
#include <list>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "fmath.hpp"
#include <iostream>

using namespace std;
using namespace BigStarCatalog;

static StringArray spectral_array;
static StringArray component_array;

bool HipStarMgr::flagSciNames = true;
double HipStarMgr::current_JDay = 2451545.0;  // Default to J2000 so that constellation art shows up in correct positions at init
map<int,string> HipStarMgr::common_names_map;
map<int,string> HipStarMgr::common_names_map_i18n;
map<string,int> HipStarMgr::common_names_index;
map<string,int> HipStarMgr::common_names_index_i18n;

map<int,string> HipStarMgr::sci_names_map_i18n;
map<string,int> HipStarMgr::sci_names_index_i18n;

string HipStarMgr::convertToSpectralType(int index)
{
	if (index < 0 || index >= spectral_array.getSize()) {
		stringstream oss;
		oss << "convertToSpectralType: bad index: " << index << ", max: " << spectral_array.getSize();
		Log.write(oss.str(), cLog::LOG_TYPE::L_ERROR);
		return "";
	}
	return spectral_array[index];
}

string HipStarMgr::convertToComponentIds(int index)
{
	if (index < 0 || index >= component_array.getSize()) {
		stringstream oss;
		oss << "convertToComponentIds: bad index: " << index << ", max: " << component_array.getSize() << endl;
		Log.write(oss.str(), cLog::LOG_TYPE::L_ERROR);
		return "";
	}
	return component_array[index];
}

void HipStarMgr::initTriangle(int lev,int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2)
{
	ZoneArrayMap::const_iterator it(zone_arrays.find(lev));
	if (it!=zone_arrays.end()) it->second->initTriangle(index,c0,c1,c2);
}



Vec3f HipStarMgr::color_table[128];

void HipStarMgr::iniColorTable()
{
	for (int i=0; i<128; i++)
		HipStarMgr::color_table[i] = v3fNull;
}

void HipStarMgr::readColorTable ()
{
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigDir() + "stars.ini");

	for (int i=0; i<128; i++) {
		char entry[256];
		sprintf(entry,"color_%03i",i);
		const string s(conf.getStr("colors",entry));
		if (!s.empty()) {
			const Vec3f c(Utility::strToVec3f(s));
			HipStarMgr::color_table[i] = c;
		} else {
			Log.write("Star color not define", cLog::LOG_TYPE::L_WARNING);
			HipStarMgr::color_table[i] = v3fNull;
		}
	}
}

void HipStarMgr::setColorStarTable(int p, Vec3f a)
{
	if (p<0 || p>127) {
		Log.write("ColorTable index Error", cLog::LOG_TYPE::L_ERROR);
		return;
	} else
		HipStarMgr::color_table[p]= a;
}

#define GAMMA 0.45

static double Gamma(double gamma,double x)
{
	return ((x<=0.0) ? 0.0 : exp(gamma*log(x)));
}

static Vec3f Gamma(double gamma,const Vec3f &x)
{
	return Vec3f(Gamma(gamma,x[0]),Gamma(gamma,x[1]),Gamma(gamma,x[2]));
}

static void InitColorTableFromConfigFile(const InitParser &conf)
{
	std::map<float,Vec3f> color_map;

	if (color_map.size() > 1) {
		for (int i=0; i<128; i++) {
			const float b_v = IndexToBV(i);
			std::map<float,Vec3f>::const_iterator greater(color_map.upper_bound(b_v));
			if (greater == color_map.begin()) {
				HipStarMgr::color_table[i] = greater->second;
			} else {
				std::map<float,Vec3f>::const_iterator less(greater);
				--less;
				if (greater == color_map.end()) {
					HipStarMgr::color_table[i] = less->second;
				} else {
					HipStarMgr::color_table[i] = Gamma(0.45,
					                                   ((b_v-less->first)*greater->second
					                                    + (greater->first-b_v)*less->second)
					                                   *(1.f/(greater->first-less->first)));
				}
			}
		}
	}
}


HipStarMgr::HipStarMgr(int width,int height) :
	starTexture(),
	hip_index(new HipIndexStruct[NR_OF_HIP+1]),
	mag_converter(new MagConverter(*this)),
	fontSize(13.),
	starFont(0)
{
	starsFader.setDuration(3000);
	setMagConverterMaxScaled60DegMag(6.5f);
	if (hip_index == 0 || mag_converter == 0) {
		cerr << "ERROR: HipStarMgr::HipStarMgr: no memory" << endl;
		exit(1);
	}
	max_geodesic_grid_level = -1;
	last_max_search_level = -1;

	shaderStars =  nullptr;
	shaderStars= new shaderProgram();
	shaderStars->init("stars.vert","stars.geom", "stars.frag");

	shaderFBO = nullptr;
	shaderFBO= new shaderProgram();
	shaderFBO->init("fbo.vert","fbo.frag");

	createShaderParams( width, height);
}

//TODO fix float[NBR_MAX_STARS];
void HipStarMgr::createShaderParams(int width,int height)
{
	dataColor.reserve(NBR_MAX_STARS*3);
	dataMag.reserve(NBR_MAX_STARS);
	dataPos.reserve(NBR_MAX_STARS*2);

	// shader pour FBO
	glGenVertexArrays(1,&drawFBO.vao);
	glBindVertexArray(drawFBO.vao);

	float dataTex[]= {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
	glGenBuffers(1,&drawFBO.tex);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,drawFBO.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, dataTex, GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	float dataPos[]= {-1.0,-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	glGenBuffers(1,&drawFBO.pos);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER,drawFBO.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, dataPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	// shader pour les étoiles
	glGenVertexArrays(1,&stars.vao);
	glBindVertexArray(stars.vao);

	glGenBuffers(1,&stars.color);
	glGenBuffers(1,&stars.mag);
	glGenBuffers(1,&stars.pos);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	
		//generate and bind fbo ID
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);

	//generate and bind render buffer ID
	glGenRenderbuffers(1, &rbID);
	glBindRenderbuffer(GL_RENDERBUFFER, rbID);

	//set the render buffer storage
	sizeTexFbo = min (width, height); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, sizeTexFbo, sizeTexFbo);

	//generate the offscreen texture
	glGenTextures(1, &renderTextureID);
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sizeTexFbo, sizeTexFbo, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	//bind the renderTextureID as colour attachment of FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureID, 0);
	//set the render buffer as the depth attachment of FBO
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, rbID);

	//check for frame buffer completeness status
	GLuint status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(status==GL_FRAMEBUFFER_COMPLETE) {
		cout << "FBO setup succeeded." << endl;
	} else {
		cout << "Error in FBO setup." << endl;
	}

	//unbind the texture and FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

HipStarMgr::~HipStarMgr(void)
{
	ZoneArrayMap::iterator it(zone_arrays.end());
	while (it!=zone_arrays.begin()) {
		--it;
		delete it->second;
		it->second = nullptr;
	}
	zone_arrays.clear();
	if (mag_converter) {
		delete mag_converter;
		mag_converter = 0;
	}
	if (hip_index) delete[] hip_index;
	if (starTexture) delete starTexture;
	if (starFont) delete starFont;

	dataColor.clear();
	dataMag.clear();
	dataPos.clear();

	deleteShader();
}

void HipStarMgr::deleteShader()
{
	if (shaderStars)
		delete shaderStars;
	shaderStars =  nullptr;

	glDeleteBuffers(1, &stars.color);
	glDeleteBuffers(1, &stars.mag);
	glDeleteBuffers(1, &stars.pos);
	glDeleteVertexArrays(1, &stars.vao);

	if (shaderFBO)
		delete shaderFBO;
	shaderFBO = nullptr;

	glDeleteBuffers(1,&drawFBO.tex);
	glDeleteBuffers(1,&drawFBO.pos);
	glDeleteVertexArrays(1,&drawFBO.vao);
}

string HipStarMgr::getCommonName(int hip)
{
	map<int,string>::const_iterator it(common_names_map_i18n.find(hip));
	if (it!=common_names_map_i18n.end()) return it->second;
	return "";
}

string HipStarMgr::getSciName(int hip)
{
	map<int,string>::const_iterator it(sci_names_map_i18n.find(hip));
	if (it!=sci_names_map_i18n.end()) return it->second;
	return "";
}

void HipStarMgr::init(float font_size, const string& font_name, const InitParser &conf)
{
	load_data(conf);
	InitColorTableFromConfigFile(conf);
	// Load star texture no mipmap:
	starTexture = new s_texture("star16x16.png",TEX_LOAD_TYPE_PNG_SOLID,false);  // Load star texture no mipmap
	starFont = new s_font(font_size, font_name);
	if (!starFont) {
		Log.write("HipStarMgr: Can't create starFont", cLog::LOG_TYPE::L_ERROR);
		assert(0);
	}
}

void HipStarMgr::setGrid(GeodesicGrid* geodesic_grid)
{
	geodesic_grid->visitTriangles(max_geodesic_grid_level,initTriangleFunc,this);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		it->second->scaleAxis();
	}
}

/***************************************************************************
 Load star catalogue data from files.
 If a file is not found, it will be skipped.
***************************************************************************/
void HipStarMgr::load_data(const InitParser &baseConf)
{
	// Please do not init twice:
	assert(max_geodesic_grid_level < 0);

	Log.write( "Loading star data..." , cLog::LOG_TYPE::L_INFO);

	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigDir() + "stars.ini");

	for (int i=0; i<8; i++) {
		char key_name[64];
		sprintf(key_name,"cat_file_name_%02d",i);
		const string cat_file_name = conf.getStr("stars",key_name).c_str();
		if (!cat_file_name.empty()) {
			Log.write(_("Loading catalog ") + string(cat_file_name) , cLog::LOG_TYPE::L_INFO);
			ZoneArray *const z = ZoneArray::create(*this,cat_file_name);
			if (z) {
				if (max_geodesic_grid_level < z->level) {
					max_geodesic_grid_level = z->level;
				}
				ZoneArray *&pos(zone_arrays[z->level]);
				if (pos) {
					cerr << cat_file_name << ", " << z->level << ": duplicate level" << endl;
					delete z;
				} else {
					pos = z;
				}
			}
		}
	}

	for (int i=0; i<=NR_OF_HIP; i++) {
		hip_index[i].a = 0;
		hip_index[i].z = 0;
		hip_index[i].s = 0;
	}
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
	        it != zone_arrays.end(); it++) {
		it->second->updateHipIndex(hip_index);
	}

	const string cat_hip_sp_file_name = conf.getStr("stars","cat_hip_sp_file_name").c_str();
	if (cat_hip_sp_file_name.empty()) {
		cerr << "ERROR: stars:cat_hip_sp_file_name not found" << endl;
	} else {
		try {
			spectral_array.initFromFile(AppSettings::Instance()->getDataRoot() + "stars/" + cat_hip_sp_file_name);
		} catch (std::exception& e) {
			cerr << "ERROR while loading data from " << ("stars/" + cat_hip_sp_file_name) << ": " << e.what();
		}
	}

	const string cat_hip_cids_file_name = conf.getStr("stars","cat_hip_cids_file_name").c_str();
	if (cat_hip_cids_file_name.empty()) {
		cerr << "ERROR: stars:cat_hip_cids_file_name not found" << endl;
	} else {
		try {
			component_array.initFromFile(AppSettings::Instance()->getDataRoot() + "stars/" + cat_hip_cids_file_name);
		} catch (std::exception& e) {
			cerr << "ERROR while loading data from " << ("stars/" + cat_hip_cids_file_name) << ": " << e.what() << endl;
		}
	}

	last_max_search_level = max_geodesic_grid_level;
	std::ostringstream oss;
	oss <<  "finished, max_geodesic_level: " << max_geodesic_grid_level;
	Log.write( oss.str() , cLog::LOG_TYPE::L_INFO);
	Log.mark();
}

//! Load common names from file
int HipStarMgr::loadCommonNames(const string& commonNameFile)
{
	common_names_map.clear();
	common_names_map_i18n.clear();
	common_names_index.clear();
	common_names_index_i18n.clear();

	Log.write("Loading star names from " + commonNameFile);

	FILE *cnFile=fopen(commonNameFile.c_str(),"r");
	if (!cnFile) {
		cerr << "Warning " << commonNameFile << " not found." << endl;
		return 0;
	}

	// Assign names to the matching stars, now support spaces in names
	char line[256];
	while (fgets(line, sizeof(line), cnFile)) {
		line[sizeof(line)-1] = '\0';
		unsigned int hip;
		if (sscanf(line,"%u",&hip)!=1) {
			cerr << "ERROR: StarMgr::loadCommonNames(" << commonNameFile << "): bad line: \"" << line << '"' << endl;
			return 0;
		}
		unsigned int i = 0;
		while (line[i]!='|' && i<sizeof(line)-2) ++i;
		i++;
		string englishCommonName =  line+i;
		// remove newline
		englishCommonName.erase(englishCommonName.length()-1, 1);

		// remove underscores
		for (string::size_type j=0; j<englishCommonName.length(); ++j) {
			if (englishCommonName[j]=='_') englishCommonName[j]=' ';
		}
		const string commonNameI18n = _(englishCommonName.c_str());
		string commonNameI18n_cap = commonNameI18n;
		transform(commonNameI18n.begin(), commonNameI18n.end(), commonNameI18n_cap.begin(), ::toupper);

		common_names_map[hip] = englishCommonName;
		common_names_index[englishCommonName] = hip;
		common_names_map_i18n[hip] = commonNameI18n;
		common_names_index_i18n[commonNameI18n_cap] = hip;

	}
	fclose(cnFile);
	return 1;
}


//! Load scientific names from file
void HipStarMgr::loadSciNames(const string& sciNameFile)
{
	sci_names_map_i18n.clear();
	sci_names_index_i18n.clear();

	Log.write("Loading star sci names from " + sciNameFile);

	FILE *snFile;
	snFile=fopen(sciNameFile.c_str(),"r");
	if (!snFile) {
		cerr << "Warning " << sciNameFile << " not found." << endl;
		return;
	}

	// Assign names to the matching stars, now support spaces in names
	char line[256];
	while (fgets(line, sizeof(line), snFile)) {
		line[sizeof(line)-1] = '\0';
		unsigned int hip;
		if (sscanf(line,"%u",&hip)!=1) {
			cerr << "ERROR: StarMgr::loadSciNames(" << sciNameFile << "): bad line: \"" << line << '"' << endl;
			return;
		}
		if (sci_names_map_i18n.find(hip)!=sci_names_map_i18n.end())
			continue;
		unsigned int i = 0;
		while (line[i]!='|' && i<sizeof(line)-2) ++i;
		i++;
		char *tempc = line+i;
		string sci_name = tempc;
		sci_name.erase(sci_name.length()-1, 1);
		string sci_name_i18n = sci_name;

		// remove underscores
		for (string::size_type j=0; j<sci_name_i18n.length(); ++j) {
			if (sci_name_i18n[j]==L'_') sci_name_i18n[j]=L' ';
		}

		string sci_name_i18n_cap = sci_name_i18n;
		transform(sci_name_i18n.begin(), sci_name_i18n.end(), sci_name_i18n_cap.begin(), ::toupper);

		sci_names_map_i18n[hip] = sci_name_i18n;
		sci_names_index_i18n[sci_name_i18n_cap] = hip;
	}

	fclose(snFile);
}

int HipStarMgr::drawStar(const Projector *prj,const Vec3d &XY, const float rc_mag[2], const Vec3f &color) const
{
	if (rc_mag[0]<=0.f || rc_mag[1]<=0.f || nbStarsToDraw >= NBR_MAX_STARS) return -1;

	float mag = 2.f*rc_mag[0];

	// Roll off star size limit as fov decreases to match planet halo scale
	RangeMap<float> rmap(180, 1, -starSizeLimit, -(starSizeLimit + objectSizeLimit));
	float rolloff = -rmap.Map(prj->getFov());
	if( mag > rolloff )
		mag = rolloff;

	dataPos.push_back(XY[0]);
	dataPos.push_back(XY[1]);
	dataMag.push_back(mag);
	dataColor.push_back(color[0]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX));
	dataColor.push_back(color[1]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX));
	dataColor.push_back(color[2]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX));

	nbStarsToDraw += 1;

	return 0;
}

int HipStarMgr::getMaxSearchLevel(const ToneReproductor *eye, const Projector *prj) const
{
	int rval = -1;
	mag_converter->setFov(prj->getFov());
	mag_converter->setEye(eye);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		const float mag_min = 0.001f*it->second->mag_min;
		float rcmag[2];
		if (mag_converter->computeRCMag(mag_min, eye, rcmag) < 0) break;
		rval = it->first;
	}
	return rval;
}

void MagConverter::setFov(float fov)
{
	if (fov > max_fov) fov = max_fov;
	else if (fov < min_fov) fov = min_fov;
	fov_factor = 108064.73f / (fov*fov);
}

void MagConverter::setEye(const ToneReproductor *eye)
{
	min_rmag
	    = std::sqrt(eye->adaptLuminance(
	                    std::exp(-0.92103f*(max_scaled_60deg_mag + mag_shift + 12.12331f))
	                    * (108064.73f / 3600.f))) * 30.f;
}

int MagConverter::computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2]) const
{
	if (mag > max_mag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// rmag:
	rc_mag[0] = std::sqrt(
	                eye->adaptLuminance(
	                    std::exp(-0.92103f*(mag + mag_shift + 12.12331f)) * fov_factor))
	            * 30.f;

	if (rc_mag[0] < min_rmag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rc_mag[0]<1.2f) {
		if (rc_mag[0] * mgr.getScale() < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[1] = rc_mag[0] * rc_mag[0] / 1.44f;
		if (rc_mag[1] * mgr.getMagScale() < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[0] = 1.2f;
	} else {
		// cmag:
		rc_mag[1] = 1.f;
		if (rc_mag[0] > mgr.getStarSizeLimit()) {
			rc_mag[0] = mgr.getStarSizeLimit();
		}
	}
	// Global scaling
	rc_mag[0] *= mgr.getScale();
	rc_mag[1] *= mgr.getMagScale();

	return 0;
}

//! Draw all the stars
double HipStarMgr::preDraw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, TimeMgr* timeMgr, float altitude)
{
	starNameToDraw.clear();
	double twinkle_param=1.;
	if (altitude>2000) twinkle_param=max(0.,1.-(altitude-2000.)/50000.);
	nbStarsToDraw = 0;
	dataPos.clear();
	dataMag.clear();
	dataColor.clear();
	current_JDay = timeMgr->getJulian();

	// If stars are turned off don't waste time below projecting all stars just to draw disembodied labels
	if(!starsFader.getInterstate()) return 0.;

	int max_search_level = getMaxSearchLevel(eye, prj);
	const GeodesicSearchResult* geodesic_search_result = grid->search(prj->unprojectViewport(),max_search_level);

	mag_converter->setFov(prj->getFov());
	mag_converter->setEye(eye);

	// Set temporary static variable for optimization
	if (flagStarTwinkle) twinkle_amount = twinkleAmount*twinkle_param;
	else twinkle_amount = 0;
	const float names_brightness = names_fader.getInterstate() * starsFader.getInterstate();

	float rcmag_table[2*256];

	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		const float mag_min = 0.001f*it->second->mag_min;

		const float k = (0.001f*it->second->mag_range)/it->second->mag_steps;
		for (int i=it->second->mag_steps-1; i>=0; i--) {
			const float mag = mag_min+k*i;
			if (mag_converter->computeRCMag(mag, eye, rcmag_table + 2*i) < 0) {
				if (i==0) return 0.; //goto exit_loop;
			}
			rcmag_table[2*i] *= starsFader.getInterstate();
		}
		last_max_search_level = it->first;

		unsigned int max_mag_star_name = 0;
		if (names_fader.getInterstate()) {
			int x = (int)((maxMagStarName-mag_min)/k);
			if (x > 0) max_mag_star_name = x;
		}
		int zone;
		for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->draw(zone,true,rcmag_table,prj, max_mag_star_name,names_brightness, starNameToDraw);
		}
		for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->draw(zone,false,rcmag_table,prj, max_mag_star_name,names_brightness, starNameToDraw);
		}

	}
//~ exit_loop:
	return 1.;
}

double HipStarMgr::draw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, TimeMgr* timeMgr, float altitude)
{
	if (nbStarsToDraw==0)
		return 0.;

	//enable FBO 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
	//render to colour attachment 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//clear the colour and depth buffers
	if (!starTrace)
		glClear(GL_COLOR_BUFFER_BIT);

	//dessin des etoiles
	shaderStars->use();

	glBindVertexArray(stars.vao);

	glBindBuffer (GL_ARRAY_BUFFER, stars.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataPos.size(),dataPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer (GL_ARRAY_BUFFER, stars.mag);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataMag.size(),dataMag.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer (GL_ARRAY_BUFFER, stars.color);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataColor.size(),dataColor.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

	StateGL::enable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, starTexture->getID());

	// A modifier par Lionel
	StateGL::BlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MAX);

	glViewport(0,0 , sizeTexFbo, sizeTexFbo);

	glBindVertexArray(stars.vao);
	glDrawArrays(GL_POINTS,0,nbStarsToDraw);
	shaderStars->unuse();

	//unbind the FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//restore the default back buffer 
	glDrawBuffer(GL_BACK_LEFT);

	//ici rendre le FBO sur l'écran
	StateGL::BlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTextureID);

	prj-> applyViewport();

	shaderFBO->use();
	glBindVertexArray(drawFBO.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	shaderFBO->unuse();

	this->drawStarName(prj);

	return 0.;
}

void HipStarMgr::drawStarName( Projector* prj )
{
	for (auto const& token : starNameToDraw) {
		prj->printGravity180(starFont, std::get<0>(token), std::get<1>(token), std::get<2>(token), std::get<3>(token), true,4,4);
		//  prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
	}
	//cout << "Nombre de nom à afficher : " << starNameToDraw.size() << endl;
}


//! Look for a star by XYZ coords
ObjectBaseP HipStarMgr::search(Vec3d pos) const
{
	assert(0);
	pos.normalize();
	vector<ObjectBaseP > v = searchAround(pos, 0.8, nullptr);
	ObjectBaseP nearest;
	double cos_angle_nearest = -10.0;
	for (vector<ObjectBaseP >::const_iterator it(v.begin()); it!=v.end(); it++) {
		const double c = (*it)->getObsJ2000Pos(0)*pos;
		if (c > cos_angle_nearest) {
			cos_angle_nearest = c;
			nearest = *it;
		}
	}
	return nearest;
}

//! Return a stl vector containing the stars located
//! inside the lim_fov circle around position v
vector<ObjectBaseP > HipStarMgr::searchAround(const Vec3d& vv, double lim_fov, const GeodesicGrid* grid) const
{
	vector<ObjectBaseP > result;
	if (!getFlagStars())
		return result;

	Vec3d v(vv);
	v.normalize();
	// find any vectors h0 and h1 (length 1), so that h0*v=h1*v=h0*h1=0
	int i;
	{
		const double a0 = fabs(v[0]);
		const double a1 = fabs(v[1]);
		const double a2 = fabs(v[2]);
		if (a0 <= a1) {
			if (a0 <= a2) i = 0;
			else i = 2;
		} else {
			if (a1 <= a2) i = 1;
			else i = 2;
		}
	}
	Vec3d h0(0.0,0.0,0.0);
	h0[i] = 1.0;
	Vec3d h1 = h0 ^ v;
	h1.normalize();
	h0 = h1 ^ v;
	h0.normalize();
	// now we have h0*v=h1*v=h0*h1=0.
	// construct a region with 4 corners e0,e1,e2,e3 inside which
	// all desired stars must be:
	double f = 1.4142136 * tan(lim_fov * C_PI/180.0);
	h0 *= f;
	h1 *= f;
	Vec3d e0 = v + h0;
	Vec3d e1 = v + h1;
	Vec3d e2 = v - h0;
	Vec3d e3 = v - h1;
	f = 1.0/e0.length();
	e0 *= f;
	e1 *= f;
	e2 *= f;
	e3 *= f;
	// search the triangles
	const GeodesicSearchResult* geodesic_search_result = grid->search(e0,e1,e2,e3,last_max_search_level);
	// iterate over the stars inside the triangles:
	f = cos(lim_fov * C_PI/180.);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		int zone;
		for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->searchAround(zone,v,f,result);
		}
		for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->searchAround(zone,v,f,result);
		}
	}
	return result;
}

//! Update i18 names from english names according to passed translator.
//! The translation is done using gettext with translated strings defined in translations.h
void HipStarMgr::updateI18n(Translator& trans)
{
	common_names_map_i18n.clear();
	common_names_index_i18n.clear();
	for (map<int,string>::iterator it(common_names_map.begin()); it!=common_names_map.end(); it++) {
		const int i = it->first;
		const string t(trans.translateUTF8(it->second));
		common_names_map_i18n[i] = t;
		string t_cap = t;
		transform(t.begin(), t.end(), t_cap.begin(), ::toupper);
		common_names_index_i18n[t_cap] = i;
	}
}

ObjectBaseP HipStarMgr::search(const string& name) const
{
	const string catalogs("HP HD SAO");

	string n = name;
	for (string::size_type i=0; i<n.length(); ++i) {
		if (n[i]=='_') n[i]=' ';
	}

	istringstream ss(n);
	string cat;
	unsigned int num;

	ss >> cat;

	// check if a valid catalog reference
	if (catalogs.find(cat,0) == string::npos) {
		// try see if the string is a HP number
		istringstream cat_to_num(cat);
		cat_to_num >> num;
		if (!cat_to_num.fail()) return searchHP(num);
		return nullptr;
	}

	ss >> num;
	if (ss.fail()) return nullptr;

	if (cat == "HP") return searchHP(num);
	assert(0);
	return nullptr;
}

//! Search the star by HP number
ObjectBaseP HipStarMgr::searchHP(int _HP) const
{
	if (0 < _HP && _HP <= NR_OF_HIP) {
		const Star1 *const s = hip_index[_HP].s;
		if (s) {
			const SpecialZoneArray<Star1> *const a = hip_index[_HP].a;
			const SpecialZoneData<Star1> *const z = hip_index[_HP].z;
			return s->createStelObject(a,z);
		}
	}
	return ObjectBaseP();
}

ObjectBaseP HipStarMgr::searchByNameI18n(const string& nameI18n) const
{
	string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search by HP number if it's an HP formated number
	// Please help, if you know a better way to do this:
	if (nameI18n.length() >= 2 && nameI18n[0]=='H' && nameI18n[1]=='P') {
		bool hp_ok = false;
		string::size_type i=2;
		// ignore spaces
		for (; i<nameI18n.length(); i++) {
			if (nameI18n[i] != ' ') break;
		}
		// parse the number
		unsigned int nr = 0;
		for (; i<nameI18n.length(); i++) {
			if ( (hp_ok = ('0' <= nameI18n[i] && nameI18n[i] <= '9')) ) {
				nr = 10*nr+(nameI18n[i]-'0');
			} else {
				break;
			}
		}
		if (hp_ok) {
			return searchHP(nr);
		}
	}

	// Search by I18n common name
	map<string,int>::const_iterator it(common_names_index_i18n.find(objw));
	if (it!=common_names_index_i18n.end()) {
		return searchHP(it->second);
	}

	// Search by sci name
	it = sci_names_index_i18n.find(objw);
	if (it!=sci_names_index_i18n.end()) {
		return searchHP(it->second);
	}

	return ObjectBaseP();
}

ObjectBaseP HipStarMgr::searchByName(const string& name) const
{
	string objw = name;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search by HP number if it's an HP formated number
	// Please help, if you know a better way to do this:
	if (name.length() >= 2 && name[0]=='H' && name[1]=='P') {
		bool hp_ok = false;
		string::size_type i=2;
		// ignore spaces
		for (; i<name.length(); i++) {
			if (name[i] != ' ') break;
		}
		// parse the number
		unsigned int nr = 0;
		for (; i<name.length(); i++) {
			if ( (hp_ok = ('0' <= name[i] && name[i] <= '9')) ) {
				nr = 10*nr+(name[i]-'0');
			} else {
				break;
			}
		}
		if (hp_ok) {
			return searchHP(nr);
		}
	}

	map<string,int>::const_iterator it(common_names_index_i18n.find(objw));

	// Search by sci name
	it = sci_names_index_i18n.find(objw);
	if (it!=sci_names_index_i18n.end()) {
		return searchHP(it->second);
	}

	return ObjectBaseP();
}

//! Find and return the list of at most maxNbItem objects auto-completing
//! the passed object I18n name
vector<string> HipStarMgr::listMatchingObjectsI18n( const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search for common names
	for (map<string,int>::const_iterator it(common_names_index_i18n.lower_bound(objw)); it!=common_names_index_i18n.end(); it++) {
		const string constw(it->first.substr(0,objw.size()));
		if (constw == objw) {
			if (maxNbItem == 0) break;
			result.push_back(getCommonName(it->second));
			maxNbItem--;
		} else {
			break;
		}
	}
	// Search for sci names
	for (map<string,int>::const_iterator it(sci_names_index_i18n.lower_bound(objw)); it!=sci_names_index_i18n.end(); it++) {
		const string constw(it->first.substr(0,objw.size()));
		if (constw == objw) {
			if (maxNbItem == 0) break;
			result.push_back(getSciName(it->second));
			maxNbItem--;
		} else {
			break;
		}
	}

	sort(result.begin(), result.end());

	return result;
}

//! Define font file name and size to use for star names display
void HipStarMgr::setFont(float font_size, const string& font_name)
{
	if (starFont) delete starFont;
	starFont = new s_font(font_size, font_name);
	assert(starFont);
}

