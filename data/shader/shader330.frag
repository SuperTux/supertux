#version 330 core

uniform sampler2D diffuse_texture;
uniform sampler2D displacement_texture;
uniform sampler2D framebuffer_texture;
uniform mat3 fragcoord2uv;
uniform float backbuffer;
uniform float game_time;
uniform vec2 animate;
uniform vec2 displacement_animate;
uniform bool is_displacement;

in vec4 diffuse_var;
in vec2 texcoord_var;
flat in uint attrs_var;

out vec4 fragColor;

const uint TATTR_SOLID    = 0x0001u;
const uint TATTR_UNISOLID = 0x0002u;
const uint TATTR_SHADED   = 0x0004u;
const uint TATTR_SLOPE    = 0x0010u;
const uint TATTR_FIF      = 0x0100u;
const uint TATTR_ICE      = 0x0100u;
const uint TATTR_WATER    = 0x0200u;
const uint TATTR_HURTS    = 0x0400u;
const uint TATTR_FIRE     = 0x0800u;
const uint TATTR_WALLJUMP = 0x1000u;

void main(void)
{
  vec4 color = diffuse_var * texture(diffuse_texture, texcoord_var.st + (animate * game_time));
  
  if (is_displacement)
  {
    vec4 pixel = texture(displacement_texture, texcoord_var.st + (displacement_animate * game_time));
    vec2 displacement = (pixel.rg - vec2(0.5, 0.5)) * 255;
    float alpha = pixel.a;

    vec2 uv = (fragcoord2uv * (gl_FragCoord.xyw + vec3(displacement.xy * alpha, 0))).xy;
    uv = vec2(uv.x, 1.0 - uv.y);
    vec4 back_color = texture(framebuffer_texture, uv);

    vec4 newcolor = diffuse_var * texture(diffuse_texture, texcoord_var.st + (animate * game_time));
    color = vec4(mix(newcolor.rgb, back_color.rgb, alpha), newcolor.a);
  }
  
  if ((attrs_var & TATTR_SHADED) == TATTR_SHADED &&
      (attrs_var & TATTR_WATER)  == TATTR_WATER &&
      (attrs_var & TATTR_FIRE)   != TATTR_FIRE) // Water (not lava)
  {
    vec2 uv = (fragcoord2uv * gl_FragCoord.xyw).xy;
    uv.x = uv.x + 0.0006 * (sin(game_time + uv.y * (80)) + cos(game_time + uv.y * 30));
    uv.y = 1.0 - uv.y + 0.003 * (cos(game_time + uv.y * 140));
    vec4 back_color = texture(framebuffer_texture, uv);

    if (backbuffer == 0.0)
      fragColor = color;
    else
      fragColor = vec4(max(color.rgb, back_color.rgb), 1.0);
  }
  else
  {
    if (backbuffer == 0.0 || !is_displacement)
    {
      fragColor = color;
    }
  }
}

/* EOF */
