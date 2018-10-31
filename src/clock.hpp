/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017  Association Sirius
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

#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>

#define SECONDEDURATION 1000
#define SECONDEDURATIONF 1000.0


/*! \class Clock
* \brief classe s'occupant du framerate et des FPS
*
* La classe Clock gère le framerate du logiciel. La difficulté vient des arrondis des paramètes int delta_time des composants du logiciel avec leur valeur approchée qui est donnée en float.
* La classe se concentre sur la durée d'une frame et non le FPS. Aussi, la classe se focalise sur le décompte du temps passé à exécuter un tour complet de boucle.
* Deux fonctions gèrent la durée des frames
* - la fonction wait :  elle s'occupe de la durée d'une frame par rapport à une autre. (vue locale)
* - la fonction afterOneSecond : elle s'occupe de de la durée des frames sur une période d'une seconde (vue globale)
*/
class Clock {
public:
	Clock() {
	}

	~Clock() {
	}

	//! Initialise les paramètres de l'horloge
	void init() {
		initCount= SDL_GetTicks();
		lastCount= SDL_GetTicks();
	};

	//! renvoie le nombre de frames affichées depuis le lancement du logiciel
	unsigned long int getElapsedFrame() const {
		return numberFrames;
	}

	//! ajoute la durée théorique d'une frame  écoulée
	void addCalculatedTime(int delta_time) {
		calculatedTime += delta_time;
		effectiveCalculatedTime += effectiveFrameDuration;
	}

	//! ajoute une frame
	void addFrame() {
		numberFrames++;
		frame++;
	}

	//! renvoie la durée d'un tour de boucle 
	unsigned int getDeltaTime() {
		return tickCount - lastCount;
	}

	//! fixe le FPS du logiciel lorsque l'on exécutera un enregistrement de script
	void setScriptFps(float fps) {
		scriptFPS = fps;
	}

	//! fixe le FPS du logiciel en mode normal
	void setMaxFps(float fps) {
		maxFPS = fps;
	}

	//! fixe le FPS du logiciel lorsque l'on exécutera une video
	void setVideoFps(float fps) {
		videoFPS = fps;
	}

	//! bascule en mode enregistrement de script
	void fixScriptFps() {
		frameDuration= (unsigned int) (SECONDEDURATIONF/scriptFPS);
		effectiveFrameDuration = SECONDEDURATIONF/double(scriptFPS);
	}

	//! bascule en mode normal
	void fixMaxFps() {
		frameDuration= (unsigned int) (SECONDEDURATIONF/maxFPS);
		effectiveFrameDuration = SECONDEDURATIONF/double(maxFPS);
	}

	//! bascule en mode video
	void fixVideoFps() {
		frameDuration= (unsigned int) (SECONDEDURATIONF/videoFPS);
		effectiveFrameDuration = SECONDEDURATIONF/double(videoFPS);
	}

	//! Prend une mesure de temps
	void setTickCount() {
		tickCount = SDL_GetTicks();
	}

	//! Modifie le temps de référence de l'horloge
	void setLastCount() {
		lastCount = tickCount;
	}

	//! Indique la durée d'une frame en int
	unsigned int getFrameDuration() {
		return frameDuration;
	}

	//! indique le FPS actuel
	float getFps() {
		return fps;
	}

	/** Détermine la durée d'attente entre deux frames pour obtenir le FPS théorique
	* @param videoCount : la variation indiquée par le module vidéo
	*/
	inline void wait(int videoCount);

	//! Calcule le FPS par seconde et corrige les différences
	inline bool afterOneSecond();

private:
	uint64_t numberFrames=0;
	int frame = 0;
	int fps;
	double calculatedTime = 0;
	float scriptFPS=0.f;
	float videoFPS=0.f;
	float maxFPS=0.f;
	uint64_t lastCount = 0;
	uint64_t initCount = 0;
	uint64_t tickCount = 0;
	uint16_t frameDuration=0;
	int delay = 0;
	uint64_t timeBase = 0;
	double effectiveCalculatedTime=0.0;
	double effectiveFrameDuration=0.0;
};


inline bool Clock::afterOneSecond()
{
	if (calculatedTime-timeBase > SECONDEDURATION) {
		fps=frame*SECONDEDURATIONF/(calculatedTime-timeBase);

		timeBase+=SECONDEDURATION;

		frame -= fps;

		delay =int(effectiveCalculatedTime)-calculatedTime;
		if (delay>0) {
			SDL_Delay(delay);
			calculatedTime += delay;
		}
		return true;
	} else
		return false;
}

inline void Clock::wait(int videoCount)
{
	if (tickCount-lastCount < frameDuration) {
		delay = frameDuration - (tickCount-lastCount) + videoCount ;
		if (delay<0)
			delay = 0;
		SDL_Delay(delay);
	} else {
		SDL_Delay(1); // no full speed
	}
}

#endif
