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
  
  if ((attrs_var & 0x0004u) == 0x0004u &&
      (attrs_var & 0x0200u) == 0x0200u && (attrs_var & 0x0800u) != 0x0800u) // Water (not lava)
  {
    vec2 uv = (fragcoord2uv * gl_FragCoord.xyw).xy;
    uv.x = uv.x + 0.001 * (sin(game_time + uv.y * (80)) + cos(game_time + uv.y * 30));
    // Disabled until clamping issue gets fixed
    uv.y = 1.0 - uv.y + 0.002 * (cos(game_time + uv.y * 140));
    vec4 back_color = texture(framebuffer_texture, uv);

    if (backbuffer == 0.0)
      fragColor = color;
    else
      fragColor = vec4(mix(color.rgb, back_color.rgb, 1.0 * backbuffer), 1.0);
  }
  else
  {
    if (backbuffer == 0.0)
    {
      fragColor = color;
    }
  }
}

/* EOF */
