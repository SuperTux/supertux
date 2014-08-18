#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Font Generator
# Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import argparse
import cairo
import codecs
import math
import re

class FontGenerator:

    def __init__(self):
        self.font_size = 24
        self.letters = u"""!"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"""
        self.font_family = "WenQuanYi Micro Hei"
        self.columns = 20
        self.glyph_width = 20
        self.glyph_height = 20
        self.draw_outline = False
        self.draw_shadow = False

    def generate_image(self):
        rows = (len(self.letters) + self.columns - 1) // self.columns
        width = self.glyph_width * self.columns
        height = self.glyph_height * rows

        surface = cairo.ImageSurface (cairo.FORMAT_ARGB32, width, height)
        cr = cairo.Context (surface)
        # cr.translate(0.5, 0.5)
        self.draw_glyphs(cr, self.columns, rows)
        return surface

    def draw_glyphs(self, cr, cols, rows):
        if False:
            cr.set_source_rgb(255, 255, 255)
            cr.fill()
        elif self.checkerboard:
            for row in range(rows):
                for col in range(cols):
                    idx = row * cols + col

                    cr.rectangle(col * self.glyph_width,
                                 row * self.glyph_height,
                                 self.glyph_width,
                                 self.glyph_height)
                    if (col + row % 2) % 2 == 0 :
                        cr.set_source_rgb(0.5, 0.5, 0.5)
                    else:
                        cr.set_source_rgb(0.75, 0.75, 0.75)
                    cr.fill()

        cr.select_font_face(self.font_family, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        cr.set_font_size(self.font_size)

        fascent, fdescent, fheight, fxadvance, fyadvance = cr.font_extents()

        print self.glyph_height, fheight

        for row in range(rows):
            for col in range(cols):
                idx = row * cols + col

                if idx < len(self.letters):
                    letter = self.letters[idx]
                    xbearing, ybearing, width, height, xadvance, yadvance = cr.text_extents(letter)

                    #cr.move_to(cx + 0.5 - xbearing - width / 2,
                    #           0.5 - fdescent + fheight / 2)
                    gx = col * self.glyph_width + self.glyph_width/2.0 - xbearing - width/2.0
                    gy = row * self.glyph_height + self.glyph_height/2.0 - fdescent + fheight/2

                    if self.draw_outline or self.draw_shadow:
                        cr.move_to(gx, gy)
                        cr.set_line_width(2.0)
                        cr.set_line_join(cairo.LINE_JOIN_ROUND)
                        cr.set_source_rgb(0, 0, 0)
                        cr.text_path(letter)
                        cr.stroke_preserve()

                        if self.draw_shadow:
                            cr.set_source_rgb(0, 0, 0)
                        else:
                            cr.set_source_rgb(255, 255, 255)
                        cr.fill()
                    else:
                        cr.set_source_rgb(255, 255, 255)
                        cr.move_to(gx, gy)
                        cr.show_text(letter)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Font Generator')
    parser.add_argument('-o', '--output', metavar='FILE', type=str, required=True,
                        help="output file")
    parser.add_argument('--font-size', metavar='INT', type=int,
                        help="font size")
    parser.add_argument('--font', metavar='NAME', type=str, help="font")
    parser.add_argument('--columns', metavar='INT', type=int, help="font")
    parser.add_argument('--letters', metavar='NAME', type=str, help="letters")
    parser.add_argument('--letters-from-file', metavar='FILE', type=str, help="letters from file")
    parser.add_argument('--outline', action="store_true", help="draw outline")
    parser.add_argument('--shadow', action="store_true", help="draw shadow")
    parser.add_argument('--glyph-width', type=int, help="glyph width")
    parser.add_argument('--glyph-height', type=int, help="glyph height")
    parser.add_argument('--checkerboard', action="store_true", help="draw checkerboard background")
    args = parser.parse_args()

    generator = FontGenerator()

    if args.font_size is not None:
        generator.font_size = args.font_size

    if args.letters is not None:
        generator.letters = args.letters

    if args.letters_from_file is not None:
        with codecs.open(args.letters_from_file, encoding='utf-8') as fin:
            generator.letters = re.sub(r"\s", "", fin.read())

    if args.font is not None:
        generator.font_family = args.font

    if args.letters is not None:
        generator.letters = args.letters.decode('utf8')

    if args.columns is not None:
        generator.columns = args.columns

    if args.outline is not None:
        generator.draw_outline = args.outline

    if args.shadow is not None:
        generator.draw_shadow = args.shadow

    if args.checkerboard is not None:
        generator.checkerboard = args.checkerboard

    if args.glyph_width is not None:
        generator.glyph_width = args.glyph_width

    if args.glyph_height is not None:
        generator.glyph_height = args.glyph_height

    img = generator.generate_image()

    img.write_to_png(args.output)

# EOF #
