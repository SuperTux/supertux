// The OKLab colourspace code is licensed under MIT
// Most code is taken from
// https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
// and
// https://bottosson.github.io/posts/gamutclipping/
//
//  Copyright (c) 2021 Björn Ottosson
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to
//  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//  of the Software, and to permit persons to whom the Software is furnished to do
//  so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.


#include "util/colorspace_oklab.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>

#include "math/util.hpp"
#include "util/log.hpp"
#include "video/color.hpp"


namespace {

struct ColorRGB {
  float r, g, b;
  bool is_valid() const;
};

bool
ColorRGB::is_valid() const
{
  return r >= 0.0f && r <= 1.0f && g >= 0.0f && g <= 1.0f
    && b >= 0.0f && b <= 1.0f;
}

struct ColorOKLab {
  float L, a, b;
};

ColorRGB srgb_to_linear_srgb(const Color& c)
{
  auto to_linear = [&](float channel) -> float {
    if (channel <= 0.04045f)
      return channel / 12.92f;
    else
      return powf((channel + 0.055f) / (1.0f + 0.055f), 2.4f);
  };
  return {to_linear(c.red), to_linear(c.green), to_linear(c.blue)};
}

Color linear_srgb_to_srgb(const ColorRGB& c)
{
  auto make_nonlinear = [&](float channel) -> float {
    if (channel <= 0.0031308f)
      return 12.92f * channel;
    else
      return (1.0f + 0.055f) * powf(channel, 1.0f / 2.4f) - 0.055f;
  };
  float r = make_nonlinear(c.r);
  float g = make_nonlinear(c.g);
  float b = make_nonlinear(c.b);
  // The clamping here is only for safety against numerical precision errors.
  // r, g and b should already be in [0,1] (at least approximately)
  // since they were clipped in the OKLab colourspace.
  return Color(math::clamp(r, 0.0f, 1.0f), math::clamp(g, 0.0f, 1.0f),
    math::clamp(b, 0.0f, 1.0f));
}

ColorOKLab linear_srgb_to_oklab(const ColorRGB& c)
{
  float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
  float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
  float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

  float l_ = cbrtf(l);
  float m_ = cbrtf(m);
  float s_ = cbrtf(s);

  return {
    0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
    1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
    0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
  };
}

ColorRGB oklab_to_linear_srgb(const ColorOKLab& c)
{
  float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
  float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
  float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

  float l = l_*l_*l_;
  float m = m_*m_*m_;
  float s = s_*s_*s_;

  return {
    +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
    -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
    -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
  };
}

ColorOKLCh lab_to_lch(const ColorOKLab& c)
{
  return ColorOKLCh{c.L, sqrtf(c.a * c.a + c.b * c.b), atan2f(c.b, c.a)};
}

ColorOKLab lch_to_lab(const ColorOKLCh& c)
{
  return {c.L, c.C * cosf(c.h), c.C * sinf(c.h)};
}

// Finds the maximum saturation possible for a given hue that fits in sRGB
// Saturation here is defined as S = C/L
// a and b must be normalized so a^2 + b^2 == 1
float compute_max_saturation(float a, float b)
{
    // Max saturation will be when one of r, g or b goes below zero.

  // Select different coefficients depending on which component goes below zero first
  float k0, k1, k2, k3, k4, wl, wm, ws;

  if (-1.88170328f * a - 0.80936493f * b > 1)
  {
    // Red component
    k0 = +1.19086277f; k1 = +1.76576728f; k2 = +0.59662641f; k3 = +0.75515197f; k4 = +0.56771245f;
    wl = +4.0767416621f; wm = -3.3077115913f; ws = +0.2309699292f;
  }
  else if (1.81444104f * a - 1.19445276f * b > 1)
  {
    // Green component
    k0 = +0.73956515f; k1 = -0.45954404f; k2 = +0.08285427f; k3 = +0.12541070f; k4 = +0.14503204f;
    wl = -1.2681437731f; wm = +2.6097574011f; ws = -0.3413193965f;
  }
  else
  {
    // Blue component
    k0 = +1.35733652f; k1 = -0.00915799f; k2 = -1.15130210f; k3 = -0.50559606f; k4 = +0.00692167f;
    wl = -0.0041960863f; wm = -0.7034186147f; ws = +1.7076147010f;
  }

  // Approximate max saturation using a polynomial:
  float S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b;

  // Do one step Halley's method to get closer
  // this gives an error less than 10e6, except for some blue hues where the dS/dh is close to infinite
  // this should be sufficient for most applications, otherwise do two/three steps

  float k_l = +0.3963377774f * a + 0.2158037573f * b;
  float k_m = -0.1055613458f * a - 0.0638541728f * b;
  float k_s = -0.0894841775f * a - 1.2914855480f * b;

  {
    float l_ = 1.f + S * k_l;
    float m_ = 1.f + S * k_m;
    float s_ = 1.f + S * k_s;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    float l_dS = 3.f * k_l * l_ * l_;
    float m_dS = 3.f * k_m * m_ * m_;
    float s_dS = 3.f * k_s * s_ * s_;

    float l_dS2 = 6.f * k_l * k_l * l_;
    float m_dS2 = 6.f * k_m * k_m * m_;
    float s_dS2 = 6.f * k_s * k_s * s_;

    float f  = wl * l     + wm * m     + ws * s;
    float f1 = wl * l_dS  + wm * m_dS  + ws * s_dS;
    float f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2;

    S = S - f * f1 / (f1*f1 - 0.5f * f * f2);
  }

  return S;
}

// finds L_cusp and C_cusp for a given hue
// a and b must be normalized so a^2 + b^2 == 1
struct OKLabCusp {
  float L, C;
};
OKLabCusp find_cusp(float a, float b)
{
  // First, find the maximum saturation (saturation S = C/L)
  float S_cusp = compute_max_saturation(a, b);

  // Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
  ColorOKLab c = {1, S_cusp * a, S_cusp * b};
  ColorRGB rgb_at_max = oklab_to_linear_srgb(c);
  float L_cusp = cbrtf(1.f / std::max(std::max(rgb_at_max.r, rgb_at_max.g),
    rgb_at_max.b));
  float C_cusp = L_cusp * S_cusp;

  return {L_cusp , C_cusp};
}

// Finds intersection of the line defined by
// L = L0 * (1 - t) + t * L1;
// C = t * C1;
// a and b must be normalized so a^2 + b^2 == 1
float find_gamut_intersection(float a, float b, float L1, float C1, float L0)
{
  // Find the cusp of the gamut triangle
  OKLabCusp cusp = find_cusp(a, b);

  // Find the intersection for upper and lower half seprately
  float t;
  if (((L1 - L0) * cusp.C - (cusp.L - L0) * C1) <= 0.f)
  {
    // Lower half

    t = cusp.C * L0 / (C1 * cusp.L + cusp.C * (L0 - L1));
  }
  else
  {
    // Upper half

    // First intersect with triangle
    t = cusp.C * (L0 - 1.f) / (C1 * (cusp.L - 1.f) + cusp.C * (L0 - L1));

    // Then one step Halley's method
    {
      float dL = L1 - L0;
      float dC = C1;

      float k_l = +0.3963377774f * a + 0.2158037573f * b;
      float k_m = -0.1055613458f * a - 0.0638541728f * b;
      float k_s = -0.0894841775f * a - 1.2914855480f * b;

      float l_dt = dL + dC * k_l;
      float m_dt = dL + dC * k_m;
      float s_dt = dL + dC * k_s;


      // If higher accuracy is required, 2 or 3 iterations of the following block can be used:
      {
        float L = L0 * (1.f - t) + t * L1;
        float C = t * C1;

        float l_ = L + C * k_l;
        float m_ = L + C * k_m;
        float s_ = L + C * k_s;

        float l = l_ * l_ * l_;
        float m = m_ * m_ * m_;
        float s = s_ * s_ * s_;

        float ldt = 3 * l_dt * l_ * l_;
        float mdt = 3 * m_dt * m_ * m_;
        float sdt = 3 * s_dt * s_ * s_;

        float ldt2 = 6 * l_dt * l_dt * l_;
        float mdt2 = 6 * m_dt * m_dt * m_;
        float sdt2 = 6 * s_dt * s_dt * s_;

        float r = 4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s - 1;
        float r1 = 4.0767416621f * ldt - 3.3077115913f * mdt + 0.2309699292f * sdt;
        float r2 = 4.0767416621f * ldt2 - 3.3077115913f * mdt2 + 0.2309699292f * sdt2;

        float u_r = r1 / (r1 * r1 - 0.5f * r * r2);
        float t_r = -r * u_r;

        float g = -1.2681437731f * l + 2.6097574011f * m - 0.3413193965f * s - 1;
        float g1 = -1.2681437731f * ldt + 2.6097574011f * mdt - 0.3413193965f * sdt;
        float g2 = -1.2681437731f * ldt2 + 2.6097574011f * mdt2 - 0.3413193965f * sdt2;

        float u_g = g1 / (g1 * g1 - 0.5f * g * g2);
        float t_g = -g * u_g;

        b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s - 1;
        float b1 = -0.0041960863f * ldt - 0.7034186147f * mdt + 1.7076147010f * sdt;
        float b2 = -0.0041960863f * ldt2 - 0.7034186147f * mdt2 + 1.7076147010f * sdt2;

        float u_b = b1 / (b1 * b1 - 0.5f * b * b2);
        float t_b = -b * u_b;

        t_r = u_r >= 0.f ? t_r : FLT_MAX;
        t_g = u_g >= 0.f ? t_g : FLT_MAX;
        t_b = u_b >= 0.f ? t_b : FLT_MAX;

        t += std::min(t_r, std::min(t_g, t_b));
      }
    }
  }

  return t;
}

} // namespace


ColorOKLCh::ColorOKLCh(Color& c) :
  L(0.0f),
  C(0.0f),
  h(0.0f)
{
  ColorRGB rgb = srgb_to_linear_srgb(c);
  ColorOKLab lab = linear_srgb_to_oklab(rgb);
  *this = lab_to_lch(lab);
  if (C < 0.00001f)
    // Deterministic behaviour when increasing chroma of greyscale colours
    h = 0.0f;
}

Color
ColorOKLCh::to_srgb() const
{
  ColorOKLCh c = *this;
  ColorOKLab lab = lch_to_lab(c);
  ColorRGB rgb = oklab_to_linear_srgb(lab);
  if (!rgb.is_valid()) {
    c.clip_chroma();
    // Gamut clipping; reduce chroma when needed
    lab = lch_to_lab(c);
    rgb = oklab_to_linear_srgb(lab);
  }
  if (!(rgb.r > -0.001f && rgb.r < 1.001f && rgb.g > -0.001f && rgb.g < 1.001f
      && rgb.b > -0.001f && rgb.b < 1.001f)) {
    log_warning << "Colour out of bounds (after clipping): (" << rgb.r <<
      ", " << rgb.g << ", " << rgb.b << ")" << std::endl;
  }
  return linear_srgb_to_srgb(rgb);
}

float
ColorOKLCh::get_maximum_chroma() const
{
  if (C <= 0.0f || L <= 0.0f || L >= 1.0f)
    return 0.0f;
  return find_gamut_intersection(cosf(h), sinf(h), L, 1.0f, L);
}

float
ColorOKLCh::get_maximum_chroma_any_l() const
{
  OKLabCusp cusp = find_cusp(cosf(h), sinf(h));
  return cusp.C;
}

void
ColorOKLCh::clip_chroma()
{
  // Avoid numerical problems for certain hues of blue
  if (-1.67462f < h && h < -1.67460f)
    h = -1.67462f;

  L = math::clamp(L, 0.0f, 1.0f);
  C = math::clamp(C, 0.0f, get_maximum_chroma());
}

void
ColorOKLCh::clip_lightness()
{
  // Avoid numerical problems for certain hues of blue
  if (-1.67462f < h && h < -1.67460f)
    h = -1.67462f;

  L = math::clamp(L, 0.0f, 1.0f);
  ColorOKLab lab = lch_to_lab(*this);
  ColorRGB rgb = oklab_to_linear_srgb(lab);
  if (rgb.is_valid())
    return;

  OKLabCusp cusp = find_cusp(lab.a / C, lab.b / C);
  if (C >= cusp.C) {
    // The cusp is the most colourful point for the given hue
    C = cusp.C;
    L = cusp.L;
    return;
  }
  // Select a point inside the triangle defined by (L,C) in {(0,0), (1,0), cusp}
  // and then move it further if it's not in the sRGB gamut
  if (L > cusp.L) {
    // Reduce L so that it's on the triangle
    L = std::min<float>(L, 1.0f + C * (cusp.L - 1.0f) / cusp.C);
    // Reduce L so that it's in the sRGB gamut
    float L0 = -100.0f;
    float t = find_gamut_intersection(lab.a / C, lab.b / C, L, C, L0);
    L = (1.0f - t) * L0 + t * L;
    C *= t;
  } else {
    // Here the triangle is accurate
    L = std::max<float>(L, C * cusp.L / cusp.C);
  }
}

void
ColorOKLCh::clip_adaptive_L0_L_cusp(float alpha)
{
  // Avoid numerical problems for certain hues of blue
  if (-1.67462f < h && h < -1.67460f)
    h = -1.67462f;

  ColorOKLab lab = lch_to_lab(*this);
  ColorRGB rgb = oklab_to_linear_srgb(lab);
  if (rgb.is_valid())
    return;

  float a_ = lab.a / C;
  float b_ = lab.b / C;
  OKLabCusp cusp = find_cusp(a_, b_);

  float Ld = L - cusp.L;
  float k = 2.f * (Ld > 0 ? 1.f - cusp.L : cusp.L);

  float e1 = 0.5f * k + fabsf(Ld) + alpha * C / k;
  float sgn = Ld < 0.0f ? -1.0f : 1.0f;
  float L0 = cusp.L + 0.5f * (sgn * (e1 - sqrtf(
    std::max<float>(e1 * e1 - 2.f * k * fabsf(Ld), 0.0f))));

  float t = find_gamut_intersection(a_, b_, L, C, L0);
  L = (1.0f - t) * L0 + t * L;
  C *= t;
}

/* EOF */
