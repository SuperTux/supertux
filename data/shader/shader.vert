#version 330 core

attribute vec2 texcoord;
attribute vec2 position;

varying vec2 texcoord_var;

uniform mat3 modelviewprojection;

void main(void)
{
  texcoord_var = texcoord;
  diffuse_var = diffuse;
  gl_Position = vec4(vec3(position, 1) * modelviewprojection, 1.0);
}

/* EOF */
