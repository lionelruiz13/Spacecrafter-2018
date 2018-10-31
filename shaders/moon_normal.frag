//
// moon normal
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;
layout (binding=2) uniform sampler2D shadowTexture;
uniform float SunHalfAngle; 
uniform vec3 MoonPosition1; 
uniform float MoonRadius1; 

smooth in vec2 TexCoord;
in float Ambient; 
in vec3 Position;
in float NdotL;
in vec3 Light; 
//~ in vec3 ModelLight;
out vec4 FragColor;

void main(void)
{ 
	float diffuse = max(NdotL, 0.0);

	vec3 color = vec3(texture(mapTexture, TexCoord)).rgb;
	FragColor = vec4(color, 1.0);
	if(diffuse != 0.0) { 
		vec3 moon; 
		float moonHalfAngle;
		vec2 ratio; 
		float distance;
		vec3 lookup;
		if(MoonRadius1 != 0.0) { 
			moon = MoonPosition1 - Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon))); 
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)).rgb; 
			diffuse = diffuse * lookup.r; 
		} 
	}
	FragColor = vec4(color*(min(diffuse+Ambient, 1.0)), 1.0);
}
