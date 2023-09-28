#ifndef GRANITO_H
#define GRANITO_H

#include "badguy/walking_badguy.hpp"

/** Interactable friendly NPC */
class Granito : public WalkingBadguy
{
public:
  explicit Granito(const ReaderMapping& reader);

  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision_player(Player &player, const CollisionHit &hit) override;

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
  virtual void on_type_change(int old_type) override;

  bool try_wave();
  void wave();

  void activate() override;

private:
  enum Type { WALK, SIT, STAND };
  enum State {
    STATE_STAND,
    STATE_WALK,
    STATE_WAVE
  };

private:
  Timer m_walk_interval;
  State m_state;

  bool m_has_waved;


private:
  Granito(const Granito&) = delete;
  Granito& operator=(const Granito&) = delete;
};

#endif // GRANITO_H
