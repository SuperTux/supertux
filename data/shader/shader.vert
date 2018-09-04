#version 330 core

attribute vec2 texcoord;
attribute vec3 position;

varying vec2 texcoord_var;

void main(void)
{
  texcoord_var = texcoord;
  gl_Position = vec4(position * 0.01, 1.0);
}

/* EOF */
