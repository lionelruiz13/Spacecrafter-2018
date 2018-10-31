
#include "orbit_2d.hpp"
#include "body.hpp"
#include <iostream>
#include "body_color.hpp"

using namespace std;

Orbit2D::Orbit2D(Body* _body, int segments) : OrbitPlot(_body, segments) { }

void Orbit2D::drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{

	////AutoPerfDebug apd(&pd, "Body::drawOrbit_2d$"); //Debug
	if (!orbit_fader.getInterstate())
		return;
	if (!body->visibilityFader.getInterstate())
		return;
	//~ if (body->hidden)
		//~ return;

	Vec3d onscreen;
	if (!body->re.sidereal_period)
		return;

	computeShader();

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StateGL::enable(GL_BLEND);
	//~ Vec4f Color ( bodyColor.orbit_color[0], bodyColor.orbit_color[1], bodyColor.orbit_color[2], (orbit_fader.getInterstate()*visibilityFader.getInterstate()) );
	Vec4f Color ( body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()) );

	shaderOrbit2d->use();
	shaderOrbit2d->setUniform("Mat", mat.convert());
	shaderOrbit2d->setUniform("Color", Color);

	glBindVertexArray(Orbit2dData.vao);
	glBindBuffer(GL_ARRAY_BUFFER,Orbit2dData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecOrbit2dVertex.size(),vecOrbit2dVertex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	//~ glDrawElements(GL_LINES_ADJACENCY, nbOrbit2DIndices, GL_UNSIGNED_INT, (void*)0 );
	glDrawArrays(GL_LINE_STRIP, 0, vecOrbit2dVertex.size()/3);

	shaderOrbit2d->unuse();

	vecOrbit2dVertex.clear();

}

void Orbit2D::computeShader()
{

	for ( int n=0; n<ORBIT_POINTS/2-1; n++) {
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][0]) );
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][1]) );
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][2]) );
		//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[n+1][0]) );
		//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[n+1][1]));
		//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[n+1][2]) );
	}

//-------------------------------------------------------------------------
//
// gestion du point passant par le centre de la planete
//
//-------------------------------------------------------------------------
	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2-1][0]) );
	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2-1][1]) );
	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2-1][2]) );

	//ICI insertion du point passant par le centre de la planete
	body->orbit_position= body->get_ecliptic_pos();
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[0]-body->radius/10)  );
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[1]-body->radius/10) );
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[2]-0*body->radius/10)  );

	//~ vecOrbit2dVertex.push_back( (float) (body->orbit_position[0]+body->radius/10) );
	//~ vecOrbit2dVertex.push_back( (float) (body->orbit_position[1]+body->radius/10) );
	//~ vecOrbit2dVertex.push_back( (float) (body->orbit_position[2]+0*body->radius/10) );

	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2+1][0]) );
	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2+1][1]) );
	//~ vecOrbit2dVertex.push_back( (float)(orbitPoint[ORBIT_POINTS/2+1][2]) );

//-------------------------------------------------------------------------

	for ( int n= ORBIT_POINTS/2+1; n< ORBIT_POINTS-1; n++) {
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][0] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][1] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][2] );

		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[n+1][0] );
		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[n+1][1] );
		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[n+1][2] );
	}
	//fermer la boucle
	if (body->close_orbit) {
		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[ORBIT_POINTS-1][0] );
		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[ORBIT_POINTS-1][1] );
		//~ vecOrbit2dVertex.push_back( (float)orbitPoint[ORBIT_POINTS-1][2] );

		vecOrbit2dVertex.push_back( (float)orbitPoint[0][0] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[0][1] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[0][2] );
	}
}
