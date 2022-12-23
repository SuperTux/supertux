//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/info_box_line.hpp"

#include "supertux/textscroller_screen.hpp"
#include "supertux/resources.hpp"
#include "supertux/colorscheme.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/surface.hpp"

static const float ITEMS_SPACE = 4;

namespace {

FontPtr get_font_by_format_char(char format_char) {
  switch (format_char)
  {
    case ' ':
      return Resources::small_font;
    case '-':
      return Resources::big_font;
    case '\t':
    case '*':
    case '#':
    case '!':
      return Resources::normal_font;
    default:
      return Resources::normal_font;
      //log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
  }
}

Color get_color_by_format_char(char format_char) {
  switch (format_char)
  {
    case ' ':
      return ColorScheme::Text::small_color;
    case '-':
      return ColorScheme::Text::heading_color;
    case '*':
      return ColorScheme::Text::reference_color;
    case '\t':
    case '#':
    case '!':
      return ColorScheme::Text::normal_color;
    default:
      return ColorScheme::Text::normal_color;
      //log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
  }
}

InfoBoxLine::LineType get_linetype_by_format_char(char format_char) {
  switch (format_char)
  {
    case ' ':
      return InfoBoxLine::SMALL;

    case '\t':
      return InfoBoxLine::NORMAL;
    case '-':
      return InfoBoxLine::HEADING;
    case '*':
      return InfoBoxLine::REFERENCE;
    case '#':
      return InfoBoxLine::NORMAL_LEFT;
    case '!':
      return InfoBoxLine::IMAGE;
    default:
      return InfoBoxLine::SMALL;
      //log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
  }
}

} // namespace

InfoBoxLine::InfoBoxLine(char format_char, const std::string& text_) :
  lineType(get_linetype_by_format_char(format_char)),
  font(get_font_by_format_char(format_char)),
  color(get_color_by_format_char(format_char)),
  text(text_),
  image()
{
  if (lineType == IMAGE)
  {
    image = Surface::from_file(text);
  }
}

std::vector<std::unique_ptr<InfoBoxLine> >
InfoBoxLine::split(const std::string& text, float width)
{
  std::vector<std::unique_ptr<InfoBoxLine> > lines;

  std::string::size_type i = 0;
  std::string::size_type l;
  char format_char = '#';
  while (i < text.size()) {
    // take care of empty lines - represent them as blank lines of normal text
    if (text[i] == '\n') {
      lines.emplace_back(new InfoBoxLine('\t', ""));
      i++;
      continue;
    }

    // extract the format_char
    if (is_valid_format_char(text[i]))
    {
      format_char = text[i];
      i++;
    }
    else
    {
      format_char = '#';
    }
    if (i >= text.size()) break;

    // extract one line
    l = text.find('\n', i);
    if (l == std::string::npos) l=text.size();
    std::string s = text.substr(i, l-i);
    i = l+1;

    // if we are dealing with an image, just store the line
    if (format_char == '!') {
      lines.emplace_back(new InfoBoxLine(format_char, s));
      continue;
    }

    // append wrapped parts of line into list
    std::string overflow;
    do {
      FontPtr font = get_font_by_format_char(format_char);
      std::string s2 = s;
      if (font) s2 = font->wrap_to_width(s2, width, &overflow);
      lines.emplace_back(new InfoBoxLine(format_char, s2));
      s = overflow;
    } while (s.length() > 0);
  }

  return lines;
}

void
InfoBoxLine::draw(DrawingContext& context, const Rectf& bbox, int layer, LineAlignment alignment)
{
  Vector position = bbox.p1();
  switch (lineType) {
    case IMAGE:
      context.color().draw_surface(image, Vector(((bbox.get_left() + bbox.get_right() - static_cast<float>(image->get_width())) * 0.5f)
        + (static_cast<float>(image->get_width()) * (alignment == LineAlignment::LEFT ? 0.5f : alignment == LineAlignment::RIGHT ? -0.5f : 0.f)), position.y), layer);
      break;
    case NORMAL_LEFT:
      context.color().draw_text(font, text, Vector(position.x, position.y), ALIGN_LEFT, layer, color);
      break;
    default:
      context.color().draw_text(font, text, Vector((bbox.get_left() + bbox.get_right()) / 2.f, position.y),
        alignment == LineAlignment::LEFT ? ALIGN_LEFT :
        alignment == LineAlignment::RIGHT ? ALIGN_RIGHT : ALIGN_CENTER, layer, color);
      break;
  }
}

float
InfoBoxLine::get_height() const
{
  switch (lineType) {
    case IMAGE:
      return static_cast<float>(image->get_height()) + ITEMS_SPACE;
    case NORMAL_LEFT:
      return font->get_height() + ITEMS_SPACE;
    default:
      return font->get_height() + ITEMS_SPACE;
  }
}

/* EOF */
