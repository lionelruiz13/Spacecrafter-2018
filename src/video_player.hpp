/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright 2017 Immersive Adventure
 *
 *
 */

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#include "shader.hpp"
#include "stateGL.hpp"


class s_texture;

//! \class VideoPlayer
//! \brief Classe qui gere toute les fonctions de la ffmpeg pour le player vidéo.
//! Son but est de rendre un pointeur sur texture utilisable dans le reste du logiciel.
class VideoPlayer {
public:
	//! \fn VideoPlayer
	//! \brief Constructeur: initialise les états de la ffmpeg
	VideoPlayer();

	//! \fn ~VideoPlayer
	//! \brief Destructeur
	~VideoPlayer();

	//! recherche la frame suivante
	// TODO renvoyer code d'information
	void update();

	//! initialise la ffmpeg avec le nom du fichier passé en argument
	int play(std::string fileName);

	//! termine la lecture d'une vidéo en cours
	void playStop();

	//! met la video en pause
	void pause();

	//! affiche sur la console les informations liées au fichier vidéo
	void getInfo();

	//! \fn RestartVideo
	//! \brief Redemarre la vidéo du début
	bool RestartVideo();

	bool Invertflow(float &reallyDeltaTime);

	//! \fn JumpVideo
	//! \param seconde temps a sauter (en secondes)
	//! \brief Permet de faire un saut relatif en avant
	bool JumpVideo(float seconde, float &reallyDeltaTime);

	//! renvoie les tailles de la vidéo
	void getSize(unsigned int &width, unsigned int &height) {
		if (isAlive){
			width = screen_w;
			height  = screen_h;
		} else {
			width = 0;
			height = 0;
		}
	}


	//! Renvoie la différence d'affichage en ms entre la durée réelle de la vidéo et la durée théorique
	//! Utilisé dans la classe App, elle indique au soft de ralentir si besoin afin de garder la synchronisation
	//! video - son
	int getDerive() {
		if (!isAlive) {
			return 0;
		} else {
			return derive;
		}
	}

	//! Renvoie le FPS moyen du fichier vidéo
	int getFps() {
		if (!isAlive)
			return 1;
		else {
			return static_cast<int>(frameRate);
		}
	}

	//! Renvoie l'état du player
	//! @return true si un fichier est en cours de lecture , false sinon
	bool getAlive() {
		return isAlive;
	}

	//! Renvoie l'ID de la texture dans le GPU représentant la frame lue du fichier vidéo
	GLuint getVideoTexture() {
		return texture;
	}

private:
	void init();

	bool isAlive;		//! indique si une vidéo est en cours de lecture
	bool getNextFrame; //! cherche la  frame suivante dans la vidéo
	// concernant la vidéo à lire
	std::string fileName;
	int screen_w;
	int screen_h;
	bool isInPause;    //met la video en pause
	bool isSeeking; //indique que l'on a fait un saut.

	int64_t nbFrames; //! compteur de frames
	int64_t timeBase;

	//Gestion frameRate
	int64_t TickCount ;
	int64_t firstCount;
	int derive;
	int diffFrameRate;

	double frameRate;
	double frameRateDuration;
	double elapsedTime;
	int64_t nbTotalFrame;

	//gestion de la pause
	Uint32 startPause;
	Uint32 endPause;

	void initTexture();	//! initialise une texture à la taille de la vidéo
	GLuint texture; //! texture représentant la frame actuelle
	//~ s_texture *myTex;
};
