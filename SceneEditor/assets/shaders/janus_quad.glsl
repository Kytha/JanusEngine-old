#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	vec4 position = vec4(a_Position.xy, 1.0, 1.0);
	v_TexCoord = a_TexCoord;

	gl_Position = position;
}

#type fragment
#version 430

out vec4 finalColor;

uniform sampler2D u_Texture;

in vec2 v_TexCoord;

void main()
{
	finalColor = vec4(0.0,0.0,1.0,1.0); 
    //vec4(texture(u_Texture, v_TexCoord).rgb, 1.0);
}