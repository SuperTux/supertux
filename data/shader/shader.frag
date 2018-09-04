#version 330 core

uniform sampler2D diffuse_texture;
uniform vec4 diffuse;

varying vec2 texcoord_var;

void main(void)
{
  gl_FragColor = diffuse * texture(diffuse_texture, texcoord_var.st);
}

/* EOF */
