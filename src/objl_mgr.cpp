// #include "spacecrafter.hpp"
#include "call_system.hpp" //direxist
#include <SDL2/SDL.h>

#include "objl_mgr.hpp"
#include "objl.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include "log.hpp"

// Models ?
//
// https://space.frieger.com/asteroids/asteroids/

using namespace std;


ObjLMgr::ObjLMgr()
{
}


ObjLMgr::~ObjLMgr()
{
	defaultObject = nullptr;
	std::map<std::string, ObjL *>::iterator it;
	for (it=objectMap.begin(); it!=objectMap.end(); ++it) {
		delete (it->second);
	}
	objectMap.clear();
}


ObjL* ObjLMgr::select(const std::string &name)
{
	if (name =="")
		return defaultObject;

	ObjL* tmp=nullptr;
	tmp = find(name);

	if (tmp == nullptr) {
		//~ printf("Objet selectionné %s inexistant!\n", name.c_str());
		Log.write("objet "+ name + " sélectionné inexistant", cLog::LOG_TYPE::L_ERROR);
		return defaultObject;
	}
	return tmp;
}


ObjL* ObjLMgr::find(const std::string &_name)
{
	auto it = objectMap.find(_name);
	if (it != objectMap.end())
		return it->second;
	else
		return nullptr;
}
	//~ std::map<std::string, ObjL* >::iterator it;
	//~ for (it=objectMap.begin(); it!=objectMap.end(); ++it) {
		//~ if (it->first == _name) {
			//~ //printf("Retour de %s\n", _name.c_str());
			//~ return it->second;
		//~ }
	//~ }
	//~ //printf("Objet %s inexistant!\n", _name.c_str());
	//~ //Log.write("Objet "+ _name + " inexistant", cLog::LOG_TYPE::L_ERROR);
	//~ return nullptr;
//~ }


bool ObjLMgr::insert(const std::string &name, bool _defaultObject)
{
	ObjL* tmp=nullptr;
	// verification que l'objet n'est pas déjà intégré
	if (!_defaultObject) {
		tmp = find(name);
		if (tmp !=nullptr)
			return true; //already loaded
	}

	string fullDirectory=defaultDirectory+name;
	if ( CallSystem::dirExist(fullDirectory) ) {
		tmp = new ObjL();
		if (tmp->init(fullDirectory, name))  {
			objectMap.insert(std::pair<std::string,ObjL*>(name, tmp));
			//~ printf("ObjL insert %s\n", name.c_str());
			Log.write("Succesfull loading model3D "+ name, cLog::LOG_TYPE::L_INFO);
			if (_defaultObject)
				defaultObject = tmp;
			return true;
		} else {
			if (_defaultObject)
				Log.write("Erreur de chargement du model3D par défaut "+ name, cLog::LOG_TYPE::L_ERROR);
			else
				Log.write("Erreur de chargement du model3D "+ name, cLog::LOG_TYPE::L_ERROR);
			//~ printf("erreur de chargement du model3D %s\n", name.c_str());
			delete tmp;
			return false;
		}
	} else {
		Log.write("Erreur de dossier "+ defaultDirectory, cLog::LOG_TYPE::L_ERROR);
		//~ printf("erreur à propos du dossier %s\n", repertory.c_str());
		return false;
	}
}
