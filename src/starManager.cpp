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

#define PARSEC_SPEED 30856775817672
#define AL            9460730472581
#define VLUM              299792458
#define CONV_PARSEC_AL 3.261563777459  // 1 al = CONV_PARSEC_AL pc
#define PLX_MIN 0.3
#define MAX_VALUE 99999

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>
#include "starManager.hpp"
#include "utility.hpp"
#include "log.hpp"
#include <unistd.h>
#include <list>
#include <cstdlib>

/**
 * Les entrées sorties des fichiers binaires sont de la forme
 * 
 * pour les hypercubes:
 * | Char c='H' | 3 floats posX, posY, posZ | 1 uint number (of cube) |
 *    1                          12                     4             = 17 octets
 * 
 * Pour les cubes:
 * | Char c='c' | 3 floats posX, posY, posZ | 1 uint number ( of stars) |
 *           1                     12                   4               = 17 octets
 * 
 * Pour les étoiles:
 * | Char c='s' | 1 uint  name HIP| 3floats posX, posY, posZ | 2floats pmRa, pmDe | float mag | int B_V | float pc(parsec)| 
 *           1                  4                       12                8               4          4        4          = 37 octets
 */

using namespace std;

unsigned int Cube::NbTotalCube = 0;

unsigned int HyperCube::NbTotalHyperCube = 0;

// ===========================================================================
//
//  Class Cube
//
// ===========================================================================

Cube::Cube( int  a, int b, int c):size(CUBESIZE)
{
}


Cube::~Cube()
{
}


void Cube::addStar(starInfo *si)
{
}



// ===========================================================================
//
//  Class HyperCube
//
// ===========================================================================

HyperCube::HyperCube(int x, int y, int z) :hcSize(HCSIZE), min(9999), max(0)
{
}

void HyperCube::addCube(Cube *c)
{
}

unsigned int HyperCube::getNbrStars()
{
	return 0;
}

//Détermine si un cube existe, si oui retourne un pointeur
Cube* HyperCube::cubeExist(int a, int b, int c)
{
	return nullptr;
}

//Trouver dans quelle Cube se trouve l'étoile
void HyperCube::addCubeStar(starInfo* star)
{
}

HyperCube::~HyperCube()
{
}

float HyperCube::getMinMagnitude()
{
	return MinMagnitude;
}


// ===========================================================================
//
//  Class StarManager
//
// ===========================================================================

StarManager::StarManager():nbrCubes(0), nbrHyperCubes(0), MinMagnitude(500)
{
}

void StarManager::addHyperCube(HyperCube *hcb)
{
}

StarManager::~StarManager()
{
}


unsigned int StarManager::getNbrStars()
{
}

// LECTURE DU CATALOGUE INTERNE
bool StarManager::loadStarCatalog(const std::string &fileName)
{
	return false;
}


// LECTURE DU CATALOGUE INTERNE
bool StarManager::loadStarBinCatalog(const std::string &fileName)
{
	return false;
}


bool StarManager::saveStarBinCatalog(const std::string &fileName)
{
	return false;
}

bool StarManager::saveStarCatalog(const std::string &fileName)
{
	return true;
}

//Détermine si un hypercube existe, si oui retourne un pointeur
HyperCube* StarManager::hcExist(int a, int b, int c)
{
	return nullptr;
}


//Trouver dans quel Hypercube se trouve l'étoile
void StarManager::addHcStar(starInfo* star)
{
}

// Fonction de lecture du catalogue d'étoile
// Rempli une liste d'étoiles avec celles trouvées dans le catalogue
//
//    I/311 Hipparcos, the New Reduction  (van Leeuwen, 2007)
//    Hipparcos, the new Reduction of the Raw data van Leeuwen F.
//    <Astron. Astrophys. 474, 653 (2007)>
//
//    http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311
bool StarManager::loadStarRaw(const std::string &catPath)
{
	return false;
}


starInfo* StarManager::createStar(unsigned int hip, float RArad, float DErad, float Plx, float pmRA, float pmDE, float mag_app, float BV)
{
	return nullptr;
}

int StarManager::getNbrCubes()
{
	return nbrCubes;
}


float StarManager::getMinMagnitude()
{
	return MinMagnitude;
}


void StarManager::HyperCubeStatistiques()
{
}



void StarManager::MagStarStatistiques()
{
}


bool StarManager::verificationData()
{
	return false;
}

//TODO la meme en version BINAIRE
bool StarManager::saveAsterismStarsPosition(const std::string &fileNameIn,const std::string &fileNameOut)
{
	return false;
}

starInfo* StarManager::findStar(unsigned int HIPName)
{
	return nullptr;
}


bool StarManager::loadOtherStar(const std::string &fileName)
{
	return false;
}
