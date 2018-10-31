/*
 * Spacecrafter astronomy simulation and visualization
 *
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "landscape.hpp"
#include "init_parser.hpp"
#include "log.hpp"
#include "fmath.hpp"
#include "app_settings.hpp"
#include "s_texture.hpp"
#include "tone_reproductor.hpp"
#include "projector.hpp"
#include "navigator.hpp"


using namespace std;

int Landscape::slices = 20;
int Landscape::stacks = 10;

Landscape::Landscape(float _radius) : radius(_radius), sky_brightness(1.)
{
	fog_tex = nullptr;
	map_tex = nullptr;
	map_tex_night = nullptr;

	valid_landscape = 0;
	Log.write( "Landscape generic created" , cLog::LOG_TYPE::L_INFO);
	haveNightTex = false;

	shaderFog = nullptr;
	shaderFog = new shaderProgram();
	shaderFog-> init( "fog.vert","fog.frag");
}

void Landscape::initShaderFog()
{
	vector<float> dataTex;
	vector<float> dataPos;

	getFogDraw(0.95, radius*sinf(fog_alt_angle*C_PI/180.) , 128,1, &dataTex, &dataPos);

	glGenBuffers(1,&fog.tex);
	glBindBuffer(GL_ARRAY_BUFFER,fog.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataTex.size(), dataTex.data(),GL_STATIC_DRAW);

	glGenBuffers(1,&fog.pos);
	glBindBuffer(GL_ARRAY_BUFFER,fog.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataPos.size(),dataPos.data(),GL_STATIC_DRAW);

	glGenVertexArrays(1,&fog.vao);
	glBindVertexArray(fog.vao);

	glBindBuffer (GL_ARRAY_BUFFER, fog.pos);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	glBindBuffer (GL_ARRAY_BUFFER, fog.tex);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	dataTex.clear();
	dataPos.clear();
}

Landscape::~Landscape()
{
	if (fog_tex) delete fog_tex;
	if (shaderFog) {
		delete shaderFog;
		shaderFog= nullptr;
	}
}

Landscape* Landscape::createFromFile(const string& landscape_file, const string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	string s;
	s = pd.getStr(section_name, "type");
	Landscape* ldscp = nullptr;
	if (s=="spherical") {
		ldscp = new LandscapeSpherical();
	} else if (s=="fisheye") {
		ldscp = new LandscapeFisheye();
	} else {
		Log.write( "Unknown landscape type: " + s  , cLog::LOG_TYPE::L_ERROR);
		// to avoid making this a fatal error, will load as a basic Landscape
		ldscp = new Landscape();
	}
	ldscp->load(landscape_file, section_name);
	return ldscp;
}

// create landscape from parameters passed in a hash (same keys as with ini file)
// NOTE: maptex must be full path and filename
Landscape* Landscape::createFromHash(stringHash_t & param)
{
	// night landscape textures for spherical and fisheye landscape types
	string night_tex="";
	if (param["night_texture"] != "") night_tex = param["path"] + param["night_texture"];

	string texture="";
	if (param["texture"] == "") texture = param["path"] + param["maptex"];
	else
		texture = param["path"] + param["texture"];

	bool mipmap; // Default on
	if (param["mipmap"] == "on" || param["mipmap"] == "1") mipmap = true;
	else mipmap = false;

	// NOTE: textures should be full filename (and path)
	if (param["type"]=="fisheye") {
		LandscapeFisheye* ldscp = new LandscapeFisheye();
		ldscp->create(param["name"], 1, texture, Utility::strToDouble(param["fov"], Utility::strToDouble(param["texturefov"], 180)),
		              Utility::strToDouble(param["rotate_z"], 0.), night_tex, mipmap);
		return ldscp;
	} else if (param["type"]=="spherical") {
		LandscapeSpherical* ldscp = new LandscapeSpherical();
		ldscp->create(param["name"], 1, texture, Utility::strToDouble(param["base_altitude"], -90),
		              Utility::strToDouble(param["top_altitude"], 90), Utility::strToDouble(param["rotate_z"], 0.),  night_tex, mipmap);
		return ldscp;
	} else {  //wrong Landscape
		Landscape* ldscp = new Landscape();
		Log.write( "Unknown landscape type in createFromHash: " + param["name"], cLog::LOG_TYPE::L_ERROR);
		return ldscp;
	}
}

// Load attributes common to all landscapes
void Landscape::loadCommon(const string& landscape_file, const string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	name = pd.getStr(section_name, "name");
	author = pd.getStr(section_name, "author");
	description = pd.getStr(section_name, "description");
	fog_alt_angle = pd.getDouble(section_name, "fog_alt_angle", 30.);
	fog_angle_shift = pd.getDouble(section_name, "fog_angle_shift", 0.);

	if (name == "" ) {
		Log.write( "No valid landscape definition found for section " + section_name +" in file " + landscape_file , cLog::LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	} else {
		valid_landscape = 1;
	}
}

string Landscape::getFileContent(const string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getSecname(i) + '\n';
	}
	return result;
}

string Landscape::getLandscapeNames(const string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getStr(pd.getSecname(i), "name") + '\n';
	}
	return result;
}

string Landscape::nameToKey(const string& landscape_file, const string & name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	for (int i=0; i<pd.getNsec(); i++) {
		if (name==pd.getStr(pd.getSecname(i), "name")) return pd.getSecname(i);
	}
	assert(0);
	return "error";
}

void Landscape::setFlagShow(bool b)
{
	land_fader=b;
}

void Landscape::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav) {}

// Draw the horizon fog
void Landscape::drawFog(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const
{
	if (!fog_fader.getInterstate()) return;

	StateGL::BlendFunc(GL_ONE, GL_ONE);

	StateGL::enable(GL_BLEND);
	StateGL::enable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	shaderFog->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fog_tex->getID());
	shaderFog->setUniform("fader", fog_fader.getInterstate());
	shaderFog->setUniform("sky_brightness", sky_brightness);

	Mat4f proj=prj->getMatProjection().convert();
	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::translation(Vec3d(0.,0.,radius*sinf(fog_angle_shift*C_PI/180.)))).convert();

	shaderFog->setUniform("viewport",prj->getViewport());
	shaderFog->setUniform("clipping_fov",prj->getClippingFov());
	shaderFog->setUniform("viewport_center",prj->getViewportFloatCenter());
	shaderFog->setUniform("inverseModelViewProjectionMatrix", (proj*matrix).inverse());
	shaderFog->setUniform("ModelViewProjectionMatrix", proj*matrix);
	shaderFog->setUniform("ModelViewMatrix",matrix);

	glBindVertexArray(fog.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,nbFogVertex);
	glCullFace(GL_BACK);
	StateGL::disable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
	shaderFog->unuse();
}



void Landscape::load(const string& file_name, const string& section_name) {}

// only if shaderLandscape->getProgram() is defined.
void Landscape::initShaderParams()
{
	shaderLandscape->setUniformLocation("sky_brightness");
	shaderLandscape->setUniformLocation("fader");
	shaderLandscape->setUniformLocation("haveNightTex");
	shaderLandscape->setUniformLocation("clipping_fov");

	shaderLandscape->setUniformLocation("ModelViewProjectionMatrix");
	shaderLandscape->setUniformLocation("ModelViewMatrix");
	shaderLandscape->setUniformLocation("inverseModelViewProjectionMatrix");

	shaderLandscape->setUniformLocation("viewport");
	shaderLandscape->setUniformLocation("viewport_center");


	shaderFog->setUniformLocation("fader");
	shaderFog->setUniformLocation("sky_brightness");
	shaderFog->setUniformLocation("ModelViewProjectionMatrix");
	shaderFog->setUniformLocation("clipping_fov");
	shaderFog->setUniformLocation("ModelViewMatrix");
	shaderFog->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderFog->setUniformLocation("viewport");
	shaderFog->setUniformLocation("viewport_center");
}

void Landscape::deleteVboVoa()
{
	glDeleteBuffers(1, &landscape.tex);
	glDeleteBuffers(1, &landscape.pos);
	glDeleteVertexArrays(1, &landscape.vao);

	glDeleteBuffers(1, &fog.tex);
	glDeleteBuffers(1, &fog.pos);
	glDeleteVertexArrays(1, &fog.vao);
}

void Landscape::deleteMapTex()
{
	if (map_tex) delete map_tex;
	map_tex = nullptr;
	if (map_tex_night) delete map_tex_night;
	map_tex_night = nullptr;
}

// *********************************************************************
//
// Fisheye landscape
//
// *********************************************************************

LandscapeFisheye::LandscapeFisheye(float _radius) : Landscape(_radius), rotate_z(0)
{
	if (fog_tex) delete fog_tex;
	shaderLandscape =  nullptr;
	shaderLandscape= new shaderProgram();
	shaderLandscape->init( "landscape2T.vert", "landscape2T.geom","landscape2T.frag");
	initShaderParams();
}


LandscapeFisheye::~LandscapeFisheye()
{
	deleteMapTex();
	deleteVboVoa();
	if (shaderLandscape) delete shaderLandscape;
	if (shaderFog) {
		delete shaderFog;
		shaderFog= nullptr;
	}
}

void LandscapeFisheye::load(const string& landscape_file, const string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	string type = pd.getStr(section_name, "type");
	if (type != "fisheye") {
		Log.write( "type mismatch for landscape " + section_name + ": expected fisheye found " + type, cLog::LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	string texture = pd.getStr(section_name, "texture");
	if (texture == "") {
		Log.write( "No texture for landscape " + section_name , cLog::LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	string night_texture = pd.getStr(section_name, "night_texture", "");
	if (night_texture != "")
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	create(name, 1, texture,
	       pd.getDouble(section_name, "fov", pd.getDouble(section_name, "texturefov", 180)),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture,
	       pd.getBoolean(section_name, "mipmap", true));
}


// create a fisheye landscape from basic parameters (no ini file needed)
//TODO fullpath est inutile
void LandscapeFisheye::create(const string _name, bool _fullpath, const string _maptex, double _texturefov,
                              const float _rotate_z, const string _maptex_night, bool _mipmap)
{
	//~ cout << _name << " P " << _fullpath << " N " << _maptex << " T " << _texturefov << " N "  << _maptex_night << "\n";
	valid_landscape = 1;  // assume ok...
	haveNightTex = false;
	Log.write( "Landscape Fisheye " + _name + " created" , cLog::LOG_TYPE::L_INFO);
	name = _name;
	map_tex = new s_texture(/*_fullpath,*/_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);

	if (_maptex_night != "") {
		map_tex_night = new s_texture(/*_fullpath,*/_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	}

	tex_fov = _texturefov*C_PI/180.;
	rotate_z = _rotate_z*C_PI/180.;

	initShader();
	initShaderFog();
}

void LandscapeFisheye::initShader()
{
	nbVertex = 2*slices*stacks + 2* stacks;
	GLfloat *datatex = new float[nbVertex*2];
	GLfloat *datapos = new float[nbVertex*3];

	getLandscapeFisheye(radius,slices,stacks, tex_fov, datatex, datapos);

	glGenVertexArrays(1,&landscape.vao);
	glBindVertexArray(landscape.vao);

	glGenBuffers(1,&landscape.tex);
	glBindBuffer(GL_ARRAY_BUFFER,landscape.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*nbVertex*2,datatex,GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glGenBuffers(1,&landscape.pos);
	glBindBuffer(GL_ARRAY_BUFFER,landscape.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*nbVertex*3,datapos,GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	if (datatex) delete datatex;
	if (datapos) delete datapos;
}


void LandscapeFisheye::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav)
{
	if (!valid_landscape) return;
	if (!land_fader.getInterstate()) return;

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StateGL::enable(GL_CULL_FACE);
	StateGL::enable(GL_BLEND);

	shaderLandscape->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map_tex->getID());

	if (haveNightTex) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, map_tex_night->getID());
	}

	shaderLandscape->setUniform("haveNightTex",haveNightTex);

	shaderLandscape->setUniform("sky_brightness",fmin(sky_brightness,1.0));
	shaderLandscape->setUniform("fader",land_fader.getInterstate());
	shaderLandscape->setUniform("viewport",prj->getViewport());
	shaderLandscape->setUniform("clipping_fov",prj->getClippingFov());
	shaderLandscape->setUniform("viewport_center",prj->getViewportFloatCenter());

	Mat4f proj=prj->getMatProjection().convert();
	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::zrotation(-rotate_z)).convert();

	shaderLandscape->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	shaderLandscape->setUniform("ModelViewProjectionMatrix",proj*matrix);
	shaderLandscape->setUniform("ModelViewMatrix",matrix);

	glBindVertexArray(landscape.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,nbVertex);
	StateGL::disable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	//TODO a supprimer si tout en opengl4
	glActiveTexture(GL_TEXTURE0);
	shaderLandscape->unuse();

	drawFog(eye,prj,nav);
}


static inline double FisheyeTexCoordFastS(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * costheta;
}


static inline double FisheyeTexCoordFastT(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * sintheta;
}

void LandscapeFisheye::getLandscapeFisheye(double radius, int slices, int stacks, double texture_fov, GLfloat * datatex, GLfloat * datapos)
{
	unsigned int indice1=0;
	unsigned int indice3=0;
	double rho,x,y,z;
	int i, j;

	int nbr=0;
	const double drho = C_PI / stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		const double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const double dtheta = 2.0 * C_PI / slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		const double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	const int imax = stacks;

	// draw intermediate stacks as quad strips
	for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0; i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<= slices; ++j,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = cos_sin_rho_p[2];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export

				datatex[indice1] = FisheyeTexCoordFastS((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
				indice1++;
				datatex[indice1] = FisheyeTexCoordFastT((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
				indice1++;
				datapos[indice3]= x*radius;
				indice3++;
				datapos[indice3]= y*radius;
				indice3++;
				datapos[indice3]= z*radius;
				indice3++;
			}
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = cos_sin_rho_p[0];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export

				datatex[indice1] = FisheyeTexCoordFastS(rho/C_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
				indice1++;
				datatex[indice1] = FisheyeTexCoordFastT(rho/C_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
				indice1++;

				datapos[indice3]= x*radius;
				indice3++;
				datapos[indice3]= y*radius;
				indice3++;
				datapos[indice3]= z*radius;
				indice3++;
			}
		}
	}
	nbVertex = nbr;
}

// *********************************************************************
//
// spherical panoramas
//
// *********************************************************************

LandscapeSpherical::LandscapeSpherical(float _radius) : Landscape(_radius),  base_altitude(-90), top_altitude(90), rotate_z(0)
{
	if (fog_tex) delete fog_tex;
	shaderLandscape =  nullptr;
	shaderLandscape= new shaderProgram();
	shaderLandscape->init( "landscape2T.vert", "landscape2T.geom","landscape2T.frag");
	initShaderParams();
}

LandscapeSpherical::~LandscapeSpherical()
{
	deleteMapTex();
	deleteVboVoa();
	if (shaderLandscape) delete shaderLandscape;
	if (shaderFog) {
		delete shaderFog;
		shaderFog= nullptr;
	}
}

void LandscapeSpherical::load(const string& landscape_file, const string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	string type = pd.getStr(section_name, "type");
	if (type != "spherical" ) {
		Log.write( "Type mismatch for landscape " + section_name +", expected spherical, found " + type , cLog::LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}

	string texture = pd.getStr(section_name, "texture");
	if (texture == "") {
		Log.write( "No texture for landscape " + section_name , cLog::LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	string night_texture = pd.getStr(section_name, "night_texture", "");
	if (night_texture != "")
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	create(name, 1, texture,
	       pd.getDouble(section_name, "base_altitude", -90),
	       pd.getDouble(section_name, "top_altitude", 90),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture,
	       pd.getBoolean(section_name, "mipmap", true));
}


// create a spherical landscape from basic parameters (no ini file needed)
//TODO bool _fullpath, est inutile
void LandscapeSpherical::create(const string _name, bool _fullpath, const string _maptex, const float _base_altitude,
                                const float _top_altitude, const float _rotate_z, const string _maptex_night, bool _mipmap)
{
	//~ cout << _name << " P " << _fullpath << " N " << _maptex << " " << _base_altitude << " " << _top_altitude << " N " << _maptex_night << "\n";
	valid_landscape = 1;  // assume ok...
	haveNightTex = false;
	Log.write( "Landscape Spherical " + _name + " created" , cLog::LOG_TYPE::L_INFO);
	name = _name;
	map_tex = new s_texture(/*_fullpath,*/_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
	if (_maptex_night != "") {
		map_tex_night = new s_texture(/*_fullpath,*/_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	}
	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);

	base_altitude = ((_base_altitude >= -90 && _base_altitude <= 90) ? _base_altitude : -90);
	top_altitude = ((_top_altitude >= -90 && _top_altitude <= 90) ? _top_altitude : 90);
	rotate_z = _rotate_z*C_PI/180.;

	initShader();
	initShaderFog();
}

void LandscapeSpherical::initShader()
{
	nbVertex = 2*slices*stacks + 2* stacks;

	GLfloat *datatex = new float[nbVertex*2];
	GLfloat *datapos = new float[nbVertex*3];

	getLandscapeSpherical(radius, 1.0, slices,stacks, base_altitude, top_altitude, datatex, datapos);

	glGenBuffers(1,&landscape.tex);
	glBindBuffer(GL_ARRAY_BUFFER,landscape.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*nbVertex*2,datatex,GL_STATIC_DRAW);

	glGenBuffers(1,&landscape.pos);
	glBindBuffer(GL_ARRAY_BUFFER,landscape.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*nbVertex*3,datapos,GL_STATIC_DRAW);

	glGenVertexArrays(1,&landscape.vao);
	glBindVertexArray(landscape.vao);

	glBindBuffer (GL_ARRAY_BUFFER, landscape.pos);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	glBindBuffer (GL_ARRAY_BUFFER, landscape.tex);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	if (datatex) delete[] datatex;
	if (datapos) delete[] datapos;
}


void LandscapeSpherical::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav)
{
	if (!valid_landscape) return;
	if (!land_fader.getInterstate()) return;

	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StateGL::enable(GL_CULL_FACE);
	StateGL::enable(GL_BLEND);

	shaderLandscape->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map_tex->getID());

	if (haveNightTex) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, map_tex_night->getID());
	}

	shaderLandscape->setUniform("haveNightTex",haveNightTex);
	shaderLandscape->setUniform("sky_brightness",fmin(sky_brightness,1.0));
	shaderLandscape->setUniform("fader",land_fader.getInterstate());
	shaderLandscape->setUniform("viewport",prj->getViewport());
	shaderLandscape->setUniform("clipping_fov",prj->getClippingFov());
	shaderLandscape->setUniform("viewport_center",prj->getViewportFloatCenter());

	Mat4f proj=prj->getMatProjection().convert();
	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::zrotation(-rotate_z)).convert();

	shaderLandscape->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	shaderLandscape->setUniform("ModelViewProjectionMatrix",proj*matrix);
	shaderLandscape->setUniform("ModelViewMatrix",matrix);

	glBindVertexArray(landscape.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,nbVertex);

	StateGL::disable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	//TODO a supprimer si tout en opengl4
	glActiveTexture(GL_TEXTURE0);
	shaderLandscape->unuse();

	drawFog(eye, prj, nav);
}


void LandscapeSpherical::getLandscapeSpherical(double radius, double one_minus_oblateness, int slices, int stacks,
        double bottom_altitude, double top_altitude , GLfloat * datatex, GLfloat * datapos)
{
	unsigned int indiceTex=0;
	unsigned int indicePos=0;

	double bottom = C_PI / 180. * bottom_altitude;
	double angular_height = C_PI / 180. * top_altitude - bottom;

	float x, y, z;
	float s, t;
	int i, j;
	t=0.0; // from inside texture is reversed

	const float drho = angular_height / (float) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = C_PI_2 + bottom + i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const float dtheta = 2.0 * C_PI / (float) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triane fan on texturing (s coord. at top/bottom tip varies)
	const float ds = 1.0 / slices;
	const float dt = -1.0 / stacks; // from inside texture is reversed

	// draw intermediate  as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = -1.0 * cos_sin_rho_p[0];

			datatex[indiceTex++]=1-s;
			datatex[indiceTex++]=t;
			datapos[indicePos++]=x*radius;
			datapos[indicePos++]=y*radius;
			datapos[indicePos++]=z * one_minus_oblateness * radius;

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = -1.0 * cos_sin_rho_p[2];

			datatex[indiceTex++]=1-s;
			datatex[indiceTex++]=t-dt;
			datapos[indicePos++]=x*radius;
			datapos[indicePos++]=y*radius;
			datapos[indicePos++]=z * one_minus_oblateness * radius;

			s += ds;
		}
		t -= dt;
	}
}

void Landscape::getFogDraw(GLdouble radius, GLdouble height, GLint slices, GLint stacks, vector<float>* dataTex, vector<float>* dataPos)
{
	nbFogVertex=0;
	GLdouble da, r, dz;
	GLfloat z ;
	GLint i; //, j;

	da = 2.0 * C_PI / slices;
	dz = height / stacks;

	GLfloat ds = 1.0 / slices;
	GLfloat dt = 1.0 / stacks;
	GLfloat t = 0.0;
	z = 0.0;
	r = radius;
	//~ for (j = 0; j < stacks; j++) {
	GLfloat s = 0.0;
	for (i = 0; i <= slices; i++) {
		GLfloat x, y;
		if (i == slices) {
			x = sinf(0.0);
			y = cosf(0.0);
		} else {
			x = sinf(i * da);
			y = cosf(i * da);
		}
		//~ glNormal3f(x , y , 0); // on n'utilise pas les normales
		//~ glTexCoord2f(s, t);

		dataTex->push_back(s);
		dataTex->push_back(t);
		//~ sVertex3(x * r, y * r, z, mat);

		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z);
		nbFogVertex++;
		//glNormal3f(x , y , 0); //on n'utilise pas les normales

		//~ glTexCoord2f(s, t + dt);
		dataTex->push_back(s);
		dataTex->push_back(t+dt);

		//~ sVertex3(x * r, y * r, z + dz, mat);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z+dz);
		nbFogVertex++;
		s += ds;
	}			/* for slices */
	//~ t += dt;
	//~ z += dz;
	//~ }				/* for stacks */
}
