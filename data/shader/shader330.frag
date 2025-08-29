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
uniform int blur;

in vec4 diffuse_var;
in vec2 texcoord_var;

out vec4 fragColor;

void main(void)
{
  if (backbuffer == 0.0 || !is_displacement || blur != 0.0)
  {
    vec4 color = diffuse_var * texture(diffuse_texture, texcoord_var.st + (animate * game_time));
    if (blur != 0.0)
    {
      vec2 uv = (fragcoord2uv * gl_FragCoord.xyw).xy;
      uv.y = 1.0 - uv.y;
      vec2 texel = vec2(fragcoord2uv[0].x, fragcoord2uv[1].y);
      float num = 0.0;
      vec4 sum = vec4(0.0);
      
      for (float y = -blur; y <= blur; ++y)
      {
        for (float x = -blur; x <= blur; ++x)
        {
          vec2 offset = vec2(x, y) * texel;
          sum += texture(framebuffer_texture, uv + offset);
          num++;
        }
      }

      fragColor = vec4(mix(color.rgb, (sum/num).rgb * 1.05, color.a), 1.0);
    }
    else
    {
      fragColor = color;
    }
  }
  else if (is_displacement)
  {
    vec4 pixel = texture(displacement_texture, texcoord_var.st + (displacement_animate * game_time));
    vec2 displacement = (pixel.rg - vec2(0.5, 0.5)) * 255;
    float alpha = pixel.a;

    vec2 uv = (fragcoord2uv * (gl_FragCoord.xyw + vec3(displacement.xy * alpha, 0))).xy;
    uv = vec2(uv.x, 1.0 - uv.y);
    vec4 back_color = texture(framebuffer_texture, uv);

    vec4 color =  diffuse_var * texture(diffuse_texture, texcoord_var.st + (animate * game_time));
    fragColor = vec4(mix(color.rgb, back_color.rgb, alpha), color.a);
  }
}

/* EOF */
