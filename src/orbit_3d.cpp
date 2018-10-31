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

#include "orbit_3d.hpp"
#include "body.hpp"
#include <iostream>
#include "projector.hpp"
#include "body_color.hpp"

using namespace std;

Orbit3D::Orbit3D(Body* _body, int segments) : OrbitPlot(_body, segments) { }

void Orbit3D::drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	//~ if (body->getEnglishName()=="Moon")
		//~ std::cout << "Draw orbit " << body->getEnglishName() << std::endl;
	////AutoPerfDebug apd(&pd, "Body::drawOrbit_3d$"); //Debug
	if (!orbit_fader.getInterstate())
		return;
	if (!body->visibilityFader.getInterstate())
		return;
	//~ if (body->hidden)
		//~ return;
	// si l'orbite tout entière est trop petite à l'écran
	//~ if (body->get_on_screen_bounding_size(prj, nav)<5) {
		// std::cout << "Draw orbit trop petite, donc non tracée pour " << body->getEnglishName() << " orbit " <<  body->get_on_screen_bounding_size(prj, nav) << std::endl;
		//~ return;
	//~ }

	if (!body->re.sidereal_period)
		return; // TODO change name to visualization_period

	StateGL::enable(GL_DEPTH_TEST);

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateGL::enable(GL_BLEND);
	StateGL::disable(GL_CULL_FACE);

	//~ Vec4f Color ( bodyColor.orbit_color[0], bodyColor.orbit_color[1], bodyColor.orbit_color[2], (orbit_fader.getInterstate()*visibilityFader.getInterstate()) );
	Vec4f Color ( body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()) );

	shaderOrbit3d->use();
	shaderOrbit3d->setUniform("Color", Color);

	//paramétrage des matrices pour opengl4
	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	shaderOrbit3d->setUniform("ModelViewProjectionMatrix",proj*matrix);
	shaderOrbit3d->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	shaderOrbit3d->setUniform("ModelViewMatrix",matrix);
	shaderOrbit3d->setUniform("clipping_fov",prj->getClippingFov());

	setPrjMat(prj,mat);
	computeShader();

	//~ for(unsigned int i = 0 ; i < orbitSegments.size() ; i++) {

		//~ int nbPoints = (orbitSegments[i].size())/3;
		//~ if (nbPoints >= 2) {
			glBindVertexArray(Orbit3dData.vao);
			glBindBuffer(GL_ARRAY_BUFFER,Orbit3dData.pos);
			//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*orbitSegments[i].size(),orbitSegments[i].data(),GL_DYNAMIC_DRAW);
			glBufferData(GL_ARRAY_BUFFER,sizeof(float)*orbitSegments.size(),orbitSegments.data(),GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

			glDrawArrays(GL_LINE_STRIP,0,orbitSegments.size()/3);
			//~ if (body->getEnglishName()=="Moon")
				//~ std::cout << "Draw orbit this " << orbitSegments.size()/3 << " points from " << body->getEnglishName() << std::endl;
		//~ }
		orbitSegments.clear();
	//~ }

	//~ orbitSegments.clear();

	StateGL::disable(GL_DEPTH_TEST);

}

void Orbit3D::setPrjMat(const Projector* _prj, const Mat4d &_mat)
{
	prj = _prj;
	mat = _mat;
}

void Orbit3D::computeShader()
{
	// only draw moon orbits as zoom in
	Vec3d onscreen;

	//~ int j = 0;
	int i = 0;

	while(i < ORBIT_POINTS) {

		//~ vector<float> segment;
		//~ orbitSegments.push_back(segment);

		//~ while(prj->projectCustom(orbitPoint[i], onscreen, mat) && i < ORBIT_POINTS) {
			//~ orbitSegments[j].push_back( (float) orbitPoint[i][0] );
			//~ orbitSegments[j].push_back( (float) orbitPoint[i][1] );
			//~ orbitSegments[j].push_back( (float) orbitPoint[i][2] );
			orbitSegments.push_back( (float) orbitPoint[i][0] );
			orbitSegments.push_back( (float) orbitPoint[i][1] );
			orbitSegments.push_back( (float) orbitPoint[i][2] );
			//~ if (body->getEnglishName()=="Moon")
				//~ std::cout <<orbitPoint[i][0] << " " << orbitPoint[i][1] << " " << orbitPoint[i][2] << std::endl;
			i++;
		}
		//~ i++;
		//~ if( i < ORBIT_POINTS)
			//~ j++;
	//~ }

	if (body->close_orbit) {
		//~ if (prj->projectCustom(orbitPoint[0], onscreen, mat)) {
			//~ orbitSegments[j].push_back( (float) orbitPoint[0][0] );
			//~ orbitSegments[j].push_back( (float) orbitPoint[0][1] );
			//~ orbitSegments[j].push_back( (float) orbitPoint[0][2] );
			orbitSegments.push_back( (float) orbitPoint[0][0] );
			orbitSegments.push_back( (float) orbitPoint[0][1] );
			orbitSegments.push_back( (float) orbitPoint[0][2] );
		//~ }
	}
}

