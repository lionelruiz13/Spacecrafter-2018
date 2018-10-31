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

#ifndef STARNAVIGATOR_HPP
#define STARNAVIGATOR_HPP

#include <vector>
#include <mutex>
#include <GL/glew.h>

#include "vecmath.hpp"
#include "starManager.hpp"
#include "shader.hpp"
#include "s_texture.hpp"
#include "ThreadPool.hpp"

/*! \class StarNavigator
  * \brief classe permettant la balade dans les étoiles
  *
  *  \details La classe a la responsabilité de traiter l'ensemble des étoiles pour permettre
  *  un voyage dans l'espace. (unité de base: le parsec)
  */

class Projector;
class Navigator;
class ToneReproductor;

class StarNavigator {
public:
	StarNavigator();
	~StarNavigator();
	StarNavigator(StarNavigator const &) = delete;
	StarNavigator& operator = (StarNavigator const &) = delete;

	/*! /fn
	 * \brief Charge en mémoire le catalogue d'étoiles
	 * \param fileName, le nom complet du fichier des data
	 * \param mode, quel type de data doit lire le programme 
	 * \param binaryData: lecture des données binaires ou non.
	 */
	void loadRawData(const std::string &fileName) noexcept;
	void loadOtherData(const std::string &fileName) noexcept;
	void loadData(const std::string &fileName, bool binaryData) noexcept;

	void saveData(const std::string &fileName, bool binaryData) noexcept;
	/*! /fn
	 * \brief affiche à l'écran les étoiles du catalogue
	 * \param nav pour les matrices de changement de repères
	 * \param prj (non utilisé)
	 */
	void draw(const Navigator * nav, const Projector* prj) const noexcept;
	/*! /fn
	 * \brief calcule les étoiles à afficher à partir de la structure
	 * \param posI, position en parsec de l'observateur par rapport au soleil
	 * \todo que faire en cas d'erreur ?
	 */
	void computePosition(Vec3f posI) noexcept;

	void setMagConverterMagShift(float s){
		mag_shift = s;
	}

	void setMagConverterMaxMag(float mag) {
		max_mag = mag;
	}

	void setStarSizeLimit(float f) {
		starSizeLimit = f;
	}

	void setScale(float s) {
		starScale = s;
	}

	void setMagScale(float s) {
		starMagScale = s;
	}

	//! Set display flag for Stars.
	void setFlagStars(bool b) {
		starsFader=b;
	}

	void clear(){
		listGlobalStarVisible.clear();
	}

protected:
	//tampons pour l'affichage des shaders
	std::vector<float> starPos;
	std::vector<float> starColorIntensity;
	std::vector<float> starRadius;

	bool starsFader = true;

	// position de l'observateur en parsec
	Vec3f pos;
	// valeur de pos à la frame d'avant
	Vec3f old_pos;

	//le gestionnaire de l'ensemble des étoiles
	StarManager *starMgr;
	// shader utilisé pour affichage
	shaderProgram *shaderStarNav;
	// structure de gestion des VAO-VBO
	DataGL starNav;
	//initialisation du shader et des VAO-VBO
	void createShader();
	//supression du shader et des VAO-VBO
	void deleteShader();

	//initialisation de la table des couleurs
	void initRCMagTable();
	//liste des étoiles à afficher issue du StarManager
	std::vector<starInfo*> listGlobalStarVisible;
	// taille de la liste listGlobalStarVisible
	unsigned int maxStars;
	//fonction permettant d'établir listGlobalStarVisible
	void setListGlobalStarVisible();
	//fonction permettant la mise à mise à zéro des tampons pour les shaders
	void clearBuffer();

	// fonction permettant le calcul des couleurs et rayon d'une étoile en fonction de sa magnitude
	int computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2]);

	//table des couleurs
	static Vec3f color_table[128];

	//texture utilisée pour afficher une étoile
	s_texture *starTexture;
	// tableau indiquant le rayon et l'intensité des couleurs
	float rc_mag_table[2*256];

	//mutex sur les tampons d'affichage
	std::mutex accesTab;
	// sous fonction prévue pour les threads de calcul de computePosition
	bool computeChunk(unsigned int first, unsigned int last);

	float mag_shift = 0.f;	//<stars, mag_converter_mag_schift>
	float max_mag= 6.5f;		//<stars, mag_converter_max_mag>
	float starScale = 0.9f; // <stars, star_scale>
	float starMagScale = 0.9f; // <stars,star_mag_scale>
	float starSizeLimit = 9.0; // <astro:star_size_limit>


	static bool threadWrapper(StarNavigator *a, int first, unsigned int last)
		{return a->computeChunk(first, last);};

	ThreadPool *pool=nullptr;
	std::vector< std::future<bool> > results;
};

#endif
