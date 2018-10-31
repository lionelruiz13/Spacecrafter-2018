
#include "model3D.hpp"



namespace GPU {

//! load_obj: load an OBJ file.
void Model3D::load_OBJ(const char*filename,const char*texname)
{}

Model3D::~Model3D()
{
}

//! This function intends to feed two layouts called position and texcoord
//! This is in the case you care about the program object and uniforms.
//! @param mode must be GL_TRIANGLES or GL_PATCHES if the used program contains a tessellation evaluation shader.
void Model3D::bindBuffersAndDraw(GLenum mode)
{
}

}// namespace GPU
