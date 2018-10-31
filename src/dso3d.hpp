/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/

#ifndef ___DSO3D_HPP___
#define ___DSO3D_HPP___

#include <string>
#include <fstream>
#include <vector>

#include "fader.hpp"
#include "shader.hpp"
#include "stateGL.hpp"


//! Class which manages the DSO Catalog for in_galaxy

class Projector;
class Navigator;
class s_texture;

class Dso3d {
public:
	Dso3d();
	~Dso3d();

	//! affiche le nuage de points
	void draw(double distance, const Projector *prj,const Navigator *nav) noexcept;

	//! mise à jour du fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! modifie la durée du fader
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! modifie le fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! renvoie la valeur du fader
	bool getFlagShow(void) const {
		return fader;
	}

	//! permet de mettre à jour la texture des nébuleuses
	void setTexture(const std::string& tex_file);

	//! lecture des données du catalogue dont le nom est passé en paramètre
	bool loadCatalog(const std::string &cat) noexcept;

private:
	// initialise le shader
	void createShader();
	//détruit le shader
	void deleteShader();
	// renseigne le nombre de textures dans texNebulae
	int nbTextures;
	// position camera
	Vec3f camPos;
	s_texture* texNebulae;
	// fader pour affichage
	LinearFader fader;
	//tableau de float pour tampon openGL
	std::vector<float> posDso3d;
	std::vector<float> scaleDso3d;
	std::vector<float> texDso3d;
	//renvoie le nombre de nebulae lues du/des catalogues
	unsigned int nbNebulae;
	// données openGL
	DataGL sData;
	// shader responsable de l'affichage du nuage
	shaderProgram *shaderDso3d;
};

#endif // ___DSO3D_HPP___
