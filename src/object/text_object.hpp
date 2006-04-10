#ifndef __TEXTOBJECT_H__
#define __TEXTOBJECT_H__

#include "game_object.hpp"
#include "scripting/text.hpp"
#include "script_interface.hpp"

class Font;

/** A text object intended for scripts that want to tell a story */
class TextObject : public GameObject, public Scripting::Text,
                   public ScriptInterface
{
public:
  TextObject();
  virtual ~TextObject();

  void expose(HSQUIRRELVM vm, int table_idx);
  void unexpose(HSQUIRRELVM vm, int table_idx);

  void set_text(const std::string& text);
  void set_font(const std::string& name);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void set_visible(bool visible);
  void set_centered(bool centered);
  bool is_visible();

  void draw(DrawingContext& context);
  void update(float elapsed_time);

private:
  Font* font;
  std::string text;
  float fading;
  float fadetime;
  bool visible;
  bool centered;
};

#endif

