#ifndef __TEXTOBJECT_H__
#define __TEXTOBJECT_H__

#include "game_object.h"
#include "scripting/text.h"

class Font;

/** A text object intended for scripts that want to tell a story */
class TextObject : public GameObject, public Scripting::Text
{
public:
  TextObject();
  virtual ~TextObject();

  void set_text(const std::string& text);
  void set_font(const std::string& name);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void set_visible(bool visible);
  bool is_visible();

  void draw(DrawingContext& context);
  void action(float elapsed_time);

private:
  Font* font;
  std::string text;
  float fading;
  float fadetime;
  bool visible;
};

#endif

