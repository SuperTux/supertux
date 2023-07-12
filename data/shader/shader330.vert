#version 330 core

in vec2 texcoord;
in vec4 texcoord_repeat;
in vec2 position;
in vec4 diffuse;

out vec4 diffuse_var;
out vec4 texcoord_repeat_var;
out vec2 texcoord_var;

uniform mat3 modelviewprojection;

void main(void)
{
  texcoord_var = texcoord;
  texcoord_repeat_var = texcoord_repeat;
  diffuse_var = diffuse;
  gl_Position = vec4(vec3(position, 1) * modelviewprojection, 1.0);
}

/* EOF */
