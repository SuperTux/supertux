//  Copyright (c) 2021
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

#ifndef HEADER_SUPERTUX_UTIL_COLORSPACE_OKLAB_HPP
#define HEADER_SUPERTUX_UTIL_COLORSPACE_OKLAB_HPP

class Color;

struct ColorOKLCh final {
  ColorOKLCh(float pL, float pC, float ph) : L(pL), C(pC), h(ph) {}

  // Convert an non-linear sRGB colour to OKLab's LCh
  ColorOKLCh(Color& c);

  // Convert to non-linear sRGB; clip_chroma is applied if required.
  Color to_srgb() const;

  // Find the maximum chroma which is still representable in sRGB while the
  // lightness and hue are preserved
  float get_maximum_chroma() const;

  // Find the maximum chroma which is still representable in sRGB while the
  // hue is preserved
  float get_maximum_chroma_any_l() const;

  // Reduce the chroma so that the colour can be represented in sRGB.
  // Also clamp the lightness if needed.
  void clip_chroma();

  // Change the lightness so that the colour can be represented in sRGB.
  void clip_lightness();

  // Changes both the lightness and chroma so that the colour can be represented
  // in sRGB. The resulting colour should have less visual distance to the true
  // colour than colour produced by clipping only chroma or lightness.
  void clip_adaptive_L0_L_cusp(float alpha=0.05f);

  float L, C, h;
};

#endif

/* EOF */
