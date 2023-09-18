#ifndef GRANITO_H
#define GRANITO_H

#include "badguy/walking_badguy.hpp"

/** Interactable friendly NPC */
class Granito : public WalkingBadguy
{
public:
  Granito(const ReaderMapping& reader);

  virtual void active_update(float dt_sec) override;

  static std::string class_name() { return "granito"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Granito"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual bool is_snipable() const override { return false; }
  virtual bool is_freezable() const override { return false; }

  virtual void kill_fall() override;

  GameObjectTypes get_types() const override;

protected:
  virtual void initialize() override;

private:
  enum Type {
    SMALL_WALK,
    SMALL_SIT,
    SMALL_STAND
  };

private:
  Granito(const Granito&) = delete;
  Granito& operator=(const Granito&) = delete;


};

#endif // GRANITO_H
