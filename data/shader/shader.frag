#version 330 core

uniform sampler2D diffuse_texture;

varying vec4 diffuse_var;
varying vec2 texcoord_var;

void main(void)
{
  gl_FragColor = diffuse_var * texture(diffuse_texture, texcoord_var.st);
}

/* EOF */
