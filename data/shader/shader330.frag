#version 330 core

uniform sampler2D diffuse_texture;
uniform sampler2D framebuffer_texture;
uniform mat3 fragcoord2uv;
uniform float backbuffer;
uniform float game_time;
uniform float real_time;

in vec4 diffuse_var;
in vec2 texcoord_var;

out vec4 fragColor;

void main(void)
{
  if (true)
  {
    vec4 color =  diffuse_var * texture(diffuse_texture, texcoord_var.st);
    fragColor = color;
  }
  else
  {
    // water reflection
    vec4 color =  diffuse_var * texture(diffuse_texture, texcoord_var.st);
    vec2 uv = (fragcoord2uv * gl_FragCoord.xyw).xy + vec2(0, 0.05);
    uv.x = uv.x + 0.005 * sin(real_time + uv.y * 100);
    uv = vec2(uv.x, 1.0 - uv.y);
    vec4 back_color = texture(framebuffer_texture, uv);
    if (backbuffer == 0.0)
      fragColor = color;
    else
      if (uv.y > 0.5)
        fragColor = vec4(mix(vec3(0,0,0.75), mix(color.rgb, back_color.rgb, 0.95 * backbuffer), (1.2 - uv.y) * (1.2 - uv.y)), 1.0);
      else
        fragColor = color;
  }
}

/* EOF */
