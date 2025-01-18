#include "editor_comment.hpp"

#include "editor/editor.hpp"
#include "supertux/info_box_line.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

EditorComment::EditorComment(const ReaderMapping& reader):
  MovingObject(reader),
  m_lines(),
  m_comment("")
{
  float w,h;
  reader.get("x", m_col.m_bbox.get_left(), 0.f);
  reader.get("y", m_col.m_bbox.get_top(), 0.f);
  reader.get("width", w, 32.f * 3);
  reader.get("height", h, 32.f * 3);
  m_col.m_bbox.set_size(w, h);

  reader.get("comment", m_comment);
  refresh_comment();
}

void
EditorComment::draw(DrawingContext& context)
{
  float y = get_y();
  for (std::unique_ptr<InfoBoxLine>& line : m_lines)
  {
    if (y >= get_bbox().get_bottom())
      break;

    line->draw(context, Rectf(get_x(), y, get_width() / 2, y), get_layer() + 1, InfoBoxLine::LEFT);
    y += line->get_height();
  }

  if (Editor::is_active())
    context.color().draw_filled_rect(get_bbox(), Color(0.5f, 0.2f, 0.f, 0.6f), 0.f, get_layer());
}

void
EditorComment::check_state()
{
  refresh_comment();
}

ObjectSettings
EditorComment::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_multiline_text(_("Comment"), &m_comment, "comment");

  return result;
}

void
EditorComment::refresh_comment()
{
  for (auto& line : m_lines)
    line.reset();

  m_lines = InfoBoxLine::split(m_comment, get_width(), true);

  for (const auto& line : m_lines)
    m_lines_height += line->get_height();
}

/* EOF */
