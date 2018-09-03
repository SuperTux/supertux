#version 330 core

attribute vec3 texcoord;
attribute vec3 position;

varying vec3 texcoord_var;

void main(void)
{
  texcoord_var = texcoord;
  gl_Position = vec4(position, 1.0);
}

/* EOF */
