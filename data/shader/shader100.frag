#version 100

uniform sampler2D diffuse_texture;

varying mediump vec2 texcoord_var;
varying lowp vec4 diffuse_var;

void main(void)
{
  gl_FragColor = diffuse_var * texture2D(diffuse_texture, texcoord_var.st);
}

/* EOF */
