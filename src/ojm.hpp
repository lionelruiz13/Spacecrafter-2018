#ifndef OJM_HPP_INCLUDED
#define OJM_HPP_INCLUDED

#include "vecmath.hpp"
#include "s_texture.hpp"
#include "shader.hpp"
#include <vector>
#include <string>

struct Shape {
    std::string name;

	std::vector<Vec3f> vertices;
	std::vector<Vec2f> uvs;
	std::vector<Vec3f> normals;
	std::vector<unsigned int> indices;

	Vec3f Ka;
	Vec3f Kd;
	Vec3f Ks;
	float Ns;
	float T=1.0;

	s_texture *map_Ka=nullptr;
	s_texture *map_Kd=nullptr;
	s_texture *map_Ks=nullptr;

	DataGL dGL;
};

class Ojm {
public:
	Ojm(const std::string& _fileName);
	Ojm(const std::string & _fileName, const std::string & _pathFile, float multiplier);
	~Ojm();

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	//! charge et initialise un objet OJM
	bool init(float multiplier= 1.0);

	//! dessine l'objet
	void draw(shaderProgram *shader);

	//! pour debugger : print
	void print();

private:
	bool is_ok = false; //say if the model is correctly initialised and operationnal
	//! vérifie si les indices coincident dans l'objet

	bool testIndices();

	//! charge un objet OJM du disque dur
	bool readOJM(const std::string& filename, float multiplier= 1.0);

	//! indices des différents morceaux de l'objet
	std::vector<Shape> shapes;

	//! initialise tous les parametres GL de l'ojm
	void initGLparam();

	//! supprime les paramètres GL de l'ojm
	void delGLparam();

	std::string fileName;
	std::string pathFile;
};


#endif // OJM_HPP_INCLUDED
