//nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

#define M_PI   3.14159265358979323846

layout (triangles) in;
layout (triangle_strip , max_vertices = 3) out;

uniform mat4 Mat;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


vec4 custom_project(vec4 invec)
{
	float zNear=main_clipping_fov[0];
	float zFar=main_clipping_fov[1];
	float fov=main_clipping_fov[2];

	float fisheye_scale_factor = 1.0/fov*180.0/M_PI*2.0;
	float viewport_center_x=viewport_center[0];
	float viewport_center_y=viewport_center[1];
	float viewport_radius=viewport_center[2];

	vec4 win = invec;
    win = Mat * win;
    win.w = 0.0;

	float depth = length(win);

    float rq1 = win.x*win.x+win.y*win.y;

	if (rq1 <= 0.0 ) {
		if (win.z < 0.0) {
			win.x = viewport_center_x;
			win.y = viewport_center_y;
			win.z = 1.0;
			win.w =-1.0;
			return win;
		}
		win.x = viewport_center_x;
		win.y = viewport_center_y;
		win.z = -1e30;
		win.w = -1.0;
		return win;
	}
	else{
        float oneoverh = 1.0/sqrt(rq1);
        float a = M_PI/2.0 + atan(win.z*oneoverh);
        float f = a * fisheye_scale_factor;

        f *= viewport_radius * oneoverh;

        win.x = viewport_center_x + win.x * f;
        win.y = viewport_center_y + win.y * f;

        win.z = (abs(depth) - zNear) / (zFar-zNear);
        if (a<0.9*M_PI) 
			win.w = 1.0;
        else
			win.w = -1.0;
        return win;
	}
}


in VInterpolators
{
	vec2 texCoord;
} dataVertex[];


out FInterpolators
{
	vec2 texCoord;
} dataFrag;


void main(void)
{
	vec4 pos1, pos2, pos3;

    pos1 = custom_project(gl_in[0].gl_Position);
    pos2 = custom_project(gl_in[1].gl_Position);
    pos3 = custom_project(gl_in[2].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {
			pos1.z = 0.0;
			pos2.z = 0.0;
			pos3.z = 0.0;

			dataFrag.texCoord = dataVertex[0].texCoord;
			gl_Position = MVP2D * pos1;
			EmitVertex();

			dataFrag.texCoord = dataVertex[1].texCoord;
			gl_Position = MVP2D * pos2;
			EmitVertex();

			dataFrag.texCoord = dataVertex[2].texCoord;
			gl_Position = MVP2D * pos3;
			EmitVertex();
		}
    EndPrimitive();
}