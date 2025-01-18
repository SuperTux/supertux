#ifndef EDITOR_COMMENT_HPP
#define EDITOR_COMMENT_HPP

#include "supertux/moving_object.hpp"
#include "video/layer.hpp"

class ReaderMapping;
class InfoBoxLine;

class EditorComment : public MovingObject
{
public:
  EditorComment(const ReaderMapping& reader);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override { MovingObject::update(dt_sec); }
  virtual HitResponse collision(MovingObject&, const CollisionHit&) override { return ABORT_MOVE; }
  virtual void check_state() override;

  virtual bool has_variable_size() const override { return true; }
  static std::string class_name() { return "editor-comment"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Editor Comment"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return MovingObject::get_class_types().add(typeid(EditorComment)); }

  virtual ObjectSettings get_settings() override;

  virtual int get_layer() const override { return LAYER_FOREGROUND1 - 5; }

private:
  void refresh_comment();

private:
  std::string m_comment;
  std::vector<std::unique_ptr<InfoBoxLine>> m_lines;
  float m_lines_height;
};

#endif // EDITOR_COMMENT_HPP
