#version 330 core

uniform sampler2D diffuse_texture;

in vec4 diffuse_var;
in vec2 texcoord_var;

out vec4 fragColor;

void main(void)
{
  fragColor = diffuse_var * texture(diffuse_texture, texcoord_var.st);
}

/* EOF */
