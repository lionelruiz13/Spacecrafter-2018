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


#include "halo.hpp"
#include "body.hpp"
#include "navigator.hpp"
#include "projector.hpp"
#include "body_color.hpp"
#include "tone_reproductor.hpp"
#include "s_texture.hpp"
#include <iostream>

using namespace std;

DataGL Halo::HaloData;
shaderProgram* Halo::shaderHalo = nullptr;
s_texture * Halo::tex_halo = nullptr;

Halo::Halo(Body * _body)
{
	body = _body;
	//~ flagHalo = true;
}

void Halo::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	computeHalo(nav, prj, eye);

	//~ std::cout << "Draw halo 1 " << body->getEnglishName() << std::endl;

	if (rmag<1.21 && cmag < 0.05)
		return;


	StateGL::BlendFunc(GL_ONE, GL_ONE);
	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());

	StateGL::enable(GL_BLEND);

	shaderHalo->use();
	//~ shaderHalo->setUniform("Color", bodyColor.halo_color);
	shaderHalo->setUniform("Color", body->myColor->getHalo());
	shaderHalo->setUniform("cmag", cmag);

	glBindVertexArray(HaloData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,HaloData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHaloPos.size(),vecHaloPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,HaloData.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHaloTex.size(),vecHaloTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	//~ std::cout << "Draw halo 2 " << body->getEnglishName() << std::endl;

	shaderHalo->unuse();

	vecHaloPos.clear();
	vecHaloTex.clear();
}

void Halo::computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	//~ cout << "compute halo planete " << body->englishName << endl;

	float fov_q = prj->getFov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	rmag = sqrtf(eye->adaptLuminance((expf(-0.92103f*(body->computeMagnitude(nav->getObserverHelioPos()) + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * Body::object_scale;

	if (body->is_satellite)	{
		if (prj->getFov()>60) rmag=rmag/25; // usefull when going there
		else rmag=rmag/5; // usefull when zooming onto planet
	}
	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<1.2f) {
		if (body->computeMagnitude(nav->getObserverHelioPos())>0.) cmag=rmag*rmag/1.44f;
		else cmag=rmag/1.2f;
		rmag=1.2f;
	} else {

		float limit = Body::object_size_limit/1.8;
		if (rmag>limit) {
			rmag = limit + sqrt(rmag-limit)/(limit + 1);

			if (rmag > Body::object_size_limit) {
				rmag = Body::object_size_limit;
			}
		}
	}

	float screen_r = body->getOnScreenSize(prj, nav);
	cmag *= 0.5*rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {

		//~ if(rmag < 10*screen_r ) return;  // No need to draw if body is drawn fully (interferes with orbit lines)

		cmag*=rmag/screen_r;
		rmag = screen_r;
	}

	if (body->is_satellite) {
		//~ cout << "satellite " << englishName << endl;
		Vec3d _planet = body->get_parent()->get_heliocentric_ecliptic_pos();
		Vec3d _satellite = body->get_heliocentric_ecliptic_pos();
		double c = _planet.dot(_satellite);
		double OP = _planet.length();
		double OS = _satellite.length();
		if (c>0 && OP < OS)
			if (fabs(acos(c/(OP*OS)))<atan(body->get_parent()->getRadius()/OP)) {
				//~ cout << "satellite caché " << englishName << endl;
				cmag = 0.0;
			}
	}

	if (rmag<1.21 && cmag < 0.05)
		return;



	Vec2f screenPosF ((float) body->screenPos[0], (float)body->screenPos[1]);

	//~ cout << "halo planete " << body->englishName << " pos " << body->screenPos[0]<< " " << body->screenPos[1] << " " << rmag <<  " " << cmag  << endl;

	vecHaloPos.push_back( screenPosF[0]-rmag );
	vecHaloPos.push_back( screenPosF[1]-rmag );
	vecHaloPos.push_back( screenPosF[0]-rmag );
	vecHaloPos.push_back( screenPosF[1]+rmag );
	vecHaloPos.push_back( screenPosF[0]+rmag );
	vecHaloPos.push_back( screenPosF[1]-rmag );
	vecHaloPos.push_back( screenPosF[0]+rmag );
	vecHaloPos.push_back( screenPosF[1]+rmag );

	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 1 );

}

void Halo::createShader()
{

	shaderHalo = new shaderProgram();
	shaderHalo->init( "body_halo.vert", "body_halo.frag");
	shaderHalo->setUniformLocation("Color");
	shaderHalo->setUniformLocation("cmag");

	glGenVertexArrays(1,&HaloData.vao);
	glBindVertexArray(HaloData.vao);
	glGenBuffers(1,&HaloData.tex);
	glGenBuffers(1,&HaloData.pos);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}

bool Halo::setTexHaloMap(const std::string &texMap)
{
	tex_halo = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT,1);
	if (tex_halo != nullptr)
		return true;
	else
		return false;
}

void Halo::deleteDefaultTexMap()
{
	if(tex_halo != nullptr) {
		delete tex_halo;
		tex_halo = nullptr;
	}
}

void Halo::deleteShader()
{
	glDeleteBuffers(1, &HaloData.tex);
	glDeleteBuffers(1, &HaloData.pos);
	glDeleteVertexArrays(1, &HaloData.vao);

}