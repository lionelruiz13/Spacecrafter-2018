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
//! \file orbit_creator_cor.hpp
//! \brief Chain of responsability for creating orbits from given parameters
//! \author Julien LAFILLE
//! \date may 2018

#include "utility.hpp"

class Orbit;
class SolarSystem;

class OrbitCreator{
public:
	OrbitCreator() = delete;
	OrbitCreator(const OrbitCreator * _next){
		next = _next;
	}

	virtual Orbit * handle(stringHash_t param)const = 0;

protected:
	const OrbitCreator * next = nullptr;
};

class OrbitCreatorEliptic : public OrbitCreator{
public:	
	OrbitCreatorEliptic() = delete;
	OrbitCreatorEliptic(const OrbitCreator * next, const SolarSystem * ssystem);
	virtual Orbit * handle(stringHash_t params)const;
	
private :
	const SolarSystem * ssystem;
};

class OrbitCreatorComet : public OrbitCreator{
public:	
	OrbitCreatorComet() = delete;
	OrbitCreatorComet(const OrbitCreator * next, const SolarSystem * ssystem);
	virtual Orbit * handle(stringHash_t params)const;
	
private :
	const SolarSystem * ssystem;
};

class OrbitCreatorSpecial : public OrbitCreator{
public:
	OrbitCreatorSpecial() = delete;
	OrbitCreatorSpecial(const OrbitCreator *);
	virtual Orbit * handle(stringHash_t params) const;
};

class OrbitCreatorBary : public OrbitCreator{
public:
	OrbitCreatorBary() = delete;
	OrbitCreatorBary(const OrbitCreator *, SolarSystem * ssytem);
	virtual Orbit * handle(stringHash_t params) const;
	
private:
	SolarSystem * ssystem;
};
