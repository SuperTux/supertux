#version 330 core

// uniform sampler2D diffuse_texture;
// uniform vec4 diffuse;

varying vec3 texcoord_var;

void main(void)
{
  //gl_FragColor = diffuse * texture(diffuse_texture, texcoord_var.st);
  gl_FragColor = vec4(1, 1, 1, 1);
  // gl_FragColor = vec4(texcoord_var, 1);
}

/* EOF */
