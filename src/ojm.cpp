#include "ojm.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

// *****************************************************************************
// 
// CLASSE OJM
//
// *****************************************************************************


Ojm::Ojm(/*const string _directoryName,*/ const string & _fileName, const string & _pathFile, float multiplier)
{
	is_ok = false;
}

Ojm::~Ojm()
{
}

Ojm::Ojm(const std::string& _fileName){
}

bool Ojm::init(float multiplier)
{
	return is_ok;
}

bool Ojm::testIndices()
{
	return false;
}

void Ojm::draw(shaderProgram * shader)
{
}

void Ojm::initGLparam()
{
}

bool Ojm::readOJM(const string& filename, float multiplier)
{
	return false;
}


void Ojm::print()
{
}
