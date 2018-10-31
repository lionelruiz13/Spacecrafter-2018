/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "object_base.hpp"
#include "object.hpp"
// #include "spacecrafter.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "utility.hpp"
#include "s_texture.hpp"


#define DEG_TO_RAD 3.14159265359/180.0

using namespace std;

void intrusivePtrAddRef(ObjectBase* p)
{
	p->retain();
}

void intrusivePtrRelease(ObjectBase* p)
{
	p->release();
}

ObjectBaseP ObjectBase::getBrightestStarInConstellation(void) const
{
	return ObjectBaseP();
}

s_texture * ObjectBase::pointer_star = nullptr;
s_texture * ObjectBase::pointer_planet = nullptr;
s_texture * ObjectBase::pointer_nebula = nullptr;

int ObjectBase::local_time = 0;

shaderProgram* ObjectBase::shaderPointer = nullptr;
shaderProgram* ObjectBase::shaderStarPointer = nullptr;

//GLuint ObjectBase::Pointer.vao = 0, ObjectBase::Pointer.pos = 0, ObjectBase::Pointer.color = 0, ObjectBase::Pointer.mag = 0, ObjectBase::StarPointer.vao = 0, ObjectBase::StarPointer.pos = 0;

DataGL ObjectBase::Pointer;
DataGL ObjectBase::StarPointer;

void ObjectBase::createShaderPointeur(void)
{
	//INIT OF SHADER POINTER
	shaderPointer = new shaderProgram();
	shaderPointer->init("object_base_pointer.vert","object_base_pointer.geom","object_base_pointer.frag");
	shaderPointer->setUniformLocation("color");

	//INIT OF VAO/VBO
	glGenVertexArrays(1,&Pointer.vao);
	glBindVertexArray(Pointer.vao);

	glGenBuffers(1,&Pointer.pos);
	glGenBuffers(1,&Pointer.mag);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);	
}

void ObjectBase::createShaderStarPointeur(void)
{
	//INIT OF SHADER STARPOINTER
	shaderStarPointer = new shaderProgram();
	shaderStarPointer->init("star_pointer.vert","star_pointer.geom","star_pointer.frag");
	shaderStarPointer->setUniformLocation("radius");
	shaderStarPointer->setUniformLocation("color");
	shaderStarPointer->setUniformLocation("matRotation");

	//INIT OF VAO/VBO
	glGenVertexArrays(1,&StarPointer.vao);
	glBindVertexArray(StarPointer.vao);

	glGenBuffers(1,&StarPointer.pos);
	
	glEnableVertexAttribArray(0);
}

void ObjectBase::deleteShaderPointeur(void)
{
	if(shaderPointer) delete shaderPointer;
	shaderPointer =  nullptr;
	
	glDeleteBuffers(1,&Pointer.mag);
	glDeleteBuffers(1,&Pointer.pos);
	glDeleteVertexArrays(1,&Pointer.vao);
}

void ObjectBase::deleteShaderStarPointeur(void)
{
	if(shaderStarPointer) delete shaderStarPointer;
	shaderStarPointer = nullptr;

	glDeleteBuffers(1,&StarPointer.pos);
	glDeleteVertexArrays(1,&StarPointer.vao);
}

// Draw a nice animated pointer around the object
void ObjectBase::drawPointer(int delta_time, const Projector* prj, const Navigator * nav)
{
	local_time+=delta_time;
	Vec3d pos=getEarthEquPos(nav);
	Vec3d screenposd;
	// Compute 2D pos and return if outside screen
	if (!prj->projectEarthEqu(pos, screenposd)) return;

	Vec2f screenpos = Vec2f((float) screenposd[0], (float) screenposd[1]);

	// If object is large enough, no need for distracting pointer
	if (getType()==OBJECT_NEBULA || getType()==OBJECT_BODY) {
		double size = getOnScreenSize(prj, nav, false);
		if ( size > prj->getViewportRadius()*.1f ) return;
	}

	if (getType()==OBJECT_NEBULA)
		color=Vec3f(0.4f,0.5f,0.8f);
	if (getType()==OBJECT_BODY)
		color=Vec3f(1.0f,0.3f,0.3f);

	float size = getOnScreenSize(prj, nav, false);
	size+=20.f;
	size+=10.f*sin(0.002f * local_time);

	if (getType()==OBJECT_STAR ) {

		Vec3d screenPos;

		Mat4f matRotation = Mat4f::zrotation((float)local_time/750.);

		// Compute 2D pos and return if outside screen
		if (!prj->projectEarthEqu(pos, screenPos)) return;

		Vec3f color = getRGB();
		float radius=13.f;

		glBindVertexArray(StarPointer.vao);

		glBindBuffer (GL_ARRAY_BUFFER, StarPointer.pos);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3,(Vec3f) screenPos,GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

		shaderStarPointer->use();

		glBindTexture (GL_TEXTURE_2D, pointer_star->getID());
		StateGL::enable(GL_BLEND);

		//SET UNIFORM
		shaderStarPointer->setUniform("radius", radius);
		shaderStarPointer->setUniform("matRotation", matRotation);
		shaderStarPointer->setUniform("color", color);

		glDrawArrays(GL_POINTS,0,1);

		shaderStarPointer->unuse();
	}

	if (getType()==OBJECT_NEBULA || getType()==OBJECT_BODY) {
		if (getType()==OBJECT_BODY)
			glBindTexture(GL_TEXTURE_2D, pointer_planet->getID());
		if (getType()==OBJECT_NEBULA)
			glBindTexture(GL_TEXTURE_2D, pointer_nebula->getID());


		posTex.push_back(screenpos[0] -size/2);
		posTex.push_back(screenpos[1] +size/2);
		posIndice.push_back(1.0);

		posTex.push_back(screenpos[0] +size/2);
		posTex.push_back(screenpos[1] +size/2);
		posIndice.push_back(2.0);

		posTex.push_back(screenpos[0] +size/2);
		posTex.push_back(screenpos[1] -size/2);
		posIndice.push_back(3.0);

		posTex.push_back(screenpos[0] -size/2);
		posTex.push_back(screenpos[1] -size/2);
		posIndice.push_back(4.0);

		glBindVertexArray(Pointer.vao);

		glBindBuffer (GL_ARRAY_BUFFER, Pointer.pos);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posTex.size(),posTex.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

		glBindBuffer (GL_ARRAY_BUFFER, Pointer.mag);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posIndice.size(),posIndice.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

		shaderPointer->use();

		StateGL::enable(GL_BLEND);
		shaderPointer->setUniform("color", color);
		glDrawArrays(GL_POINTS,0,4);
		shaderPointer->unuse();

		posTex.clear();
		posIndice.clear();
	}
}

void ObjectBase::initTextures(void)
{
	pointer_star = new s_texture("pointer_star.png");
	pointer_planet = new s_texture("pointer_planet.png");
	pointer_nebula = new s_texture("pointer_nebula.png");
}

void ObjectBase::deleteTextures(void)
{
	delete pointer_star;
	pointer_star = nullptr;
	delete pointer_planet;
	pointer_planet = nullptr;
	delete pointer_nebula;
	pointer_nebula = nullptr;
}
