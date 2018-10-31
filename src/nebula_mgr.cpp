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

#include <fstream>
#include <algorithm>
#include "nebula_mgr.hpp"
#include "nebula.hpp"
#include "s_texture.hpp"
#include "s_font.hpp"
#include "navigator.hpp"
#include "translator.hpp"
#include "log.hpp"
#include "fmath.hpp"


using namespace std;

NebulaMgr::NebulaMgr(void)
{
	nebZones = new vector<Nebula*>[nebGrid.getNbPoints()];
	if (! initTexPicto())
		Log.write("DSO: error while loading pictogram texture", cLog::LOG_TYPE::L_ERROR);

	createShaderHint();
	createShaderTex();
}

NebulaMgr::~NebulaMgr()
{
	vector<Nebula *>::iterator iter;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		delete (*iter);
	}

	if (Nebula::tex_NEBULA) delete Nebula::tex_NEBULA;
	Nebula::tex_NEBULA = nullptr;

	if (Nebula::nebulaFont) delete Nebula::nebulaFont;
	Nebula::nebulaFont = nullptr;

	deleteShaderHint();
	deleteShaderTex();

	delete[] nebZones;
}


void NebulaMgr::createShaderTex()
{
	Nebula::shaderNebulaTex = new shaderProgram();
	Nebula::shaderNebulaTex->init("nebulaTex.vert","nebulaTex.geom","nebulaTex.frag");
	Nebula::shaderNebulaTex->setUniformLocation("Mat");
	Nebula::shaderNebulaTex->setUniformLocation("fader");

	glGenVertexArrays(1,&Nebula::nebulaTex.vao);
	glBindVertexArray(Nebula::nebulaTex.vao);

	glGenBuffers(1,&Nebula::nebulaTex.tex);
	glGenBuffers(1,&Nebula::nebulaTex.pos);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void NebulaMgr::deleteShaderTex()
{
	if (Nebula::shaderNebulaTex)
		delete Nebula::shaderNebulaTex;
	Nebula::shaderNebulaTex = nullptr;

	glDeleteBuffers(1,&Nebula::nebulaTex.pos);
	glDeleteBuffers(1,&Nebula::nebulaTex.tex);
	glDeleteVertexArrays(1,&Nebula::nebulaTex.vao);
}

void NebulaMgr::deleteShaderHint()
{
	if (shaderNebulaHint)
		delete shaderNebulaHint;
	shaderNebulaHint = nullptr;

	glDeleteBuffers(1,&nebulaHint.pos);
	glDeleteBuffers(1,&nebulaHint.tex);
	glDeleteBuffers(1,&nebulaHint.color);
	glDeleteVertexArrays(1,&nebulaHint.vao);
}

void NebulaMgr::createShaderHint()
{
	shaderNebulaHint = new shaderProgram();
	shaderNebulaHint->init("nebulaHint.vert","nebulaHint.frag");
	shaderNebulaHint->setUniformLocation("fader");

	glGenVertexArrays(1,&nebulaHint.vao);
	glBindVertexArray(nebulaHint.vao);

	glGenBuffers(1,&nebulaHint.tex);
	glGenBuffers(1,&nebulaHint.color);
	glGenBuffers(1,&nebulaHint.pos);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}


bool NebulaMgr::initTexPicto()
{
	bool succes = true;

	if (!Nebula::tex_NEBULA)
		Nebula::tex_NEBULA = new s_texture("tex_nebulaes.png");
	if (Nebula::tex_NEBULA == nullptr)	succes=false;

	return succes;
}

// read from file
bool NebulaMgr::loadDeepskyObject(const string& cat)
{
	return loadDeepskyObjectFromCat(cat);
}

// read from stream
bool NebulaMgr::initFontName (float font_size, const string& font_name)
{
	if (!Nebula::nebulaFont)
		Nebula::nebulaFont = new s_font(font_size, font_name); // load Font
	if (!Nebula::nebulaFont) {
		Log.write("Nebula: Can't create nebulaFont\n", cLog::LOG_TYPE::L_ERROR);
		return false;
	}
	return true;
}

// Clear user added nebula
string NebulaMgr::removeNebula(const string& name, bool showOriginal=true)
{
	string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	vector <Nebula*>::iterator iter;
	vector <Nebula*>::iterator iter2;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		string testName = (*iter)->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);

		// if(testName != "" ) cout << ">" << testName << "< " << endl;
		if (testName==uname) {

			if(!(*iter)->isDeletable()) {
				if(showOriginal) (*iter)->show(); // make sure original is now visible
				continue;
			}

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ);

			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
			neb_array.erase(iter);
//			cerr << "Erased nebula " << uname << endl;

			return "";
		}
	}

	return "Requested nebula to delete not found by name.";
}

// remove all user added nebula and make standard nebulae visible again
// all standard nebulae visible become again selected
string NebulaMgr::removeSupplementalNebulae()
{

	vector<Nebula *>::iterator iter;
	vector<Nebula *>::iterator iter2;

	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {

		if (!(*iter)->isDeletable()) {
			(*iter)->show();
			(*iter)->select();
		} else {

			// erase from locator grid
			int zone = nebGrid.GetNearest((*iter)->XYZ);

			for (iter2 = nebZones[zone].begin(); iter2!=nebZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
//					cerr << "Deleting nebula from zone " << zone << " with name " << (*iter2)->englishName << endl;
					nebZones[zone].erase(iter2);
					break;
				}
			}

			// Delete nebula
			delete *iter;
			neb_array.erase(iter);
			iter--;
			// cerr << "Erased nebula " << uname << endl;
		}
	}

	return "";
}

// Draw all the Nebulae
void NebulaMgr::draw(const Projector* prj, const Navigator * nav, ToneReproductor* eye, double sky_brightness)
{
	if(!showFader) return;

	Nebula::hintsBrightness = hintsFader.getInterstate();
	Nebula::nebulaBrightness = showFader.getInterstate();
	Nebula::textBrightness = textFader.getInterstate();

	//cout << "Draw Nebulaes" << endl;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	Vec3f pXYZ;

	// Find the star zones which are in the screen
	int nbZones=0;
	// FOV is currently measured vertically, so need to adjust for wide screens
	// TODO: projector should probably use largest measurement itself
	float max_fov = myMax( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	nbZones = nebGrid.Intersect(nav->getPrecEquVision(), max_fov*C_PI/180.f*1.2f);
	static int * zoneList = nebGrid.getResult();

	//~ prj->set_orthographic_projection();	// set 2D coordinate

	// Print all the stars of all the selected zones
	static vector<Nebula *>::iterator end;
	static vector<Nebula *>::iterator iter;
	Nebula* n;

	// speed up the computation of n->getOnScreenSize(prj, nav)>5:
	const float size_limit = 5.0 * (C_PI/180.0) * (prj->getFov()/prj->getViewportHeight());

	for (int i=0; i<nbZones; ++i) {
		end = nebZones[zoneList[i]].end();
		for (iter = nebZones[zoneList[i]].begin(); iter!=end; ++iter) {

			n = *iter;

			// improve performance by skipping if too small to see
			if ( n->m_angular_size>size_limit|| (hintsFader.getInterstate()>0.0001 && n->mag <= getMaxMagHints())) {

				prj->projectJ2000(n->XYZ,n->XY);

				if (n->m_angular_size>size_limit) {
					n->drawTex(prj, nav, eye, sky_brightness);
				}

				if (textFader) {
					n->drawName(prj);
				}

				//~ cout << "drawhint " << n->getEnglishName() << endl;
				if ( n->m_angular_size<size_limit)
					n->drawHint(prj, nav, vecHintPos, vecHintTex, vecHintColor);
			}
		}
	}
	drawAllHint(prj);
}

void NebulaMgr::drawAllHint(const Projector* prj)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderNebulaHint->use();
	shaderNebulaHint->setUniform("fader", hintsFader.getInterstate());

	glBindTexture (GL_TEXTURE_2D, Nebula::tex_NEBULA->getID());

	glBindVertexArray(nebulaHint.vao);

	if(vecHintPos.size()>0) {
		glBindBuffer(GL_ARRAY_BUFFER,nebulaHint.pos);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHintPos.size(),vecHintPos.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

		glBindBuffer(GL_ARRAY_BUFFER,nebulaHint.tex);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHintTex.size(),vecHintTex.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

		glBindBuffer(GL_ARRAY_BUFFER,nebulaHint.color);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHintColor.size(),vecHintColor.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

		for(unsigned int i=0; i < (vecHintPos.size()/8) ; i++)
			glDrawArrays(GL_TRIANGLE_STRIP, 4*i, 4);

		vecHintPos.clear();
		vecHintTex.clear();
		vecHintColor.clear();

	}
	shaderNebulaHint->unuse();
}

// search by name
Object NebulaMgr::search(const string& name)
{
	Nebula *n = searchNebula(name, false);
	return Object(n);
}

// search by name
Nebula *NebulaMgr::searchNebula(const string& name, bool search_hidden=false)
{

	string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	vector <Nebula*>::const_iterator iter;

	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		string testName = (*iter)->getEnglishName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);
		//		if(testName != "" ) cout << ">" << testName << "< " << endl;
		if (testName==uname  && ((*iter)->m_hidden==false || search_hidden)) return *iter;
	}
	return nullptr;
}

void NebulaMgr::showAll()
{
	vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		(*iter)->select();
	}
}

void NebulaMgr::hideAll()
{
	vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		(*iter)->unselect();
	}
}

void NebulaMgr::selectConstellation(bool hide, string constellationName)
{
	vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getConstellation() == constellationName)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

void NebulaMgr::selectName(bool hide, string Name)
{
	vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getEnglishName() == Name)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

void NebulaMgr::selectType(bool hide, string dsoType)
{
	vector <Nebula*>::const_iterator iter;
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if ((*iter)->getStringType() == dsoType)
			hide ? (*iter)-> unselect() : (*iter)->select();
	}
}

// Look for a nebulae by XYZ coords
Object NebulaMgr::search(Vec3f Pos)
{
	Pos.normalize();
	vector<Nebula *>::iterator iter;
	Nebula * plusProche=nullptr;
	float anglePlusProche=0.;
	for (iter=neb_array.begin(); iter!=neb_array.end(); iter++) {
		if((*iter)->m_hidden==true) continue;
		if ((*iter)->XYZ[0]*Pos[0]+(*iter)->XYZ[1]*Pos[1]+(*iter)->XYZ[2]*Pos[2]>anglePlusProche) {
			anglePlusProche=(*iter)->XYZ[0]*Pos[0]+(*iter)->XYZ[1]*Pos[1]+(*iter)->XYZ[2]*Pos[2];
			plusProche=(*iter);
		}
	}
	if (anglePlusProche>Nebula::dsoRadius*0.999) {
		return plusProche;
	} else return nullptr;
}

// Return a stl vector containing the nebulas located inside the lim_fov circle around position v
vector<Object> NebulaMgr::searchAround(Vec3d v, double lim_fov) const
{
	vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * C_PI/180.);
	static Vec3d equPos;

	vector<Nebula*>::const_iterator iter = neb_array.begin();
	while (iter != neb_array.end()) {
		equPos = (*iter)->XYZ;
		equPos.normalize();
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {

			// NOTE: non-labeled nebulas are not returned!
			// Otherwise cursor select gets invisible nebulas - Rob
			if ((*iter)->getNameI18n() != "" && (*iter)->m_hidden==false) result.push_back(*iter);
		}
		iter++;
	}
	return result;
}

bool NebulaMgr::loadDeepskyObject(string _englishName, string _DSOType, string _constellation, float _ra, float _de, float _mag, float _size, string _classe,
                                  float _distance, string tex_name, bool path, float tex_angular_size, float _rotation, string _credit, float _luminance, bool deletable)
{
	Nebula *e = searchNebula(_englishName, false);
	if(e) {
		if(e->isDeletable()) {
			//~ cout << "Warning: replacing user added nebula with name " << name << ".\n";
			Log.write("nebula: replacing user added nebula with name " + _englishName, cLog::LOG_TYPE::L_WARNING);
			removeNebula(_englishName, false);
		} else {
			e->hide();
			Log.write("dso: hide nebula with name " + _englishName, cLog::LOG_TYPE::L_WARNING);
		}
	}
	e = new Nebula(_englishName, _DSOType, _constellation, _ra, _de, _mag, _size, _classe, _distance, tex_name, path,
	               tex_angular_size, _rotation, _credit, _luminance, deletable, false);

	if (e != nullptr) {
		neb_array.push_back(e);
		nebZones[nebGrid.GetNearest(e->XYZ)].push_back(e);
		return true;
	} else
		return false;
}


// read from file
bool NebulaMgr::loadDeepskyObjectFromCat(const string& cat)
{
	string recordstr;
	unsigned int i=0;
	unsigned int data_drop=0;

	//~ cout << "Loading NGC data... ";
	Log.write("Loading NGC data... ",cLog::LOG_TYPE::L_INFO);
	ifstream  ngcFile(cat);
	if (!ngcFile) {
		//~ cout << "NGC data file " << catNGC << " not found" << endl;
		Log.write("NGC data file " + cat + " not found""Loading NGC data... ",cLog::LOG_TYPE::L_ERROR);
		return false;
	}

	string name, type, constellation, deep_class, credits, tex_name;
	float ra, de, mag, tex_angular_size, distance, scale, tex_rotation, texLuminanceAdjust;

	// Read the cat entries
	while ( getline (ngcFile, recordstr )) {

		std::istringstream istr(recordstr);
		if (!(istr >> name >> type >> constellation >> ra >> de >> mag >> scale >> deep_class
		        >>  distance >> tex_name >> tex_angular_size >> tex_rotation >> credits >> texLuminanceAdjust )) {
			data_drop++;
		} else {
			if ( ! loadDeepskyObject(name, type, constellation, ra, de, mag, scale, deep_class, distance,
			                         tex_name, false, tex_angular_size, tex_rotation, credits, texLuminanceAdjust,false)) {
				//printf("error creating nebula\n");
				data_drop++;
			}
			i++;
		}
	}
	ngcFile.close();
	Log.write("Nebula: "+ Utility::intToString(i) + " items loaded, " + Utility::intToString(data_drop) + " dropped", cLog::LOG_TYPE::L_INFO);
	return true;
}


//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void NebulaMgr::translateNames(Translator& trans)
{
	vector<Nebula*>::iterator iter;
	for ( iter = neb_array.begin(); iter < neb_array.end(); iter++ ) {
		(*iter)->translateName(trans);
	}
	if(Nebula::nebulaFont) Nebula::nebulaFont->clearCache();
}


//! Return the matching Nebula object's pointer if exists or NULL
Object NebulaMgr::searchByNameI18n(const string& nameI18n) const
{
	string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
	vector <Nebula*>::const_iterator iter;

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->m_hidden==true) continue;
		string objwcap = (*iter)->nameI18;
		transform(objwcap.begin(), objwcap.end(), objwcap.begin(), ::toupper);
		if (objwcap==objw) return *iter;
	}

	return nullptr;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
vector<string> NebulaMgr::listMatchingObjectsI18n(const string& objPrefix, unsigned int maxNbItem) const
{
	vector<string> result;
	if (maxNbItem==0) return result;

	string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	vector <Nebula*>::const_iterator iter;

	// Search by common names
	for (iter = neb_array.begin(); iter != neb_array.end(); ++iter) {
		if((*iter)->m_hidden==true) continue;
		string constw = (*iter)->nameI18.substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back((*iter)->nameI18);
		}
	}

	sort(result.begin(), result.end());
	if (result.size()>maxNbItem) result.erase(result.begin()+maxNbItem, result.end());

	return result;
}
