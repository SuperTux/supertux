#pragma once

#include "badguy/badguy.hpp"

class PogoSnowball final : public BadGuy
{
public:
  PogoSnowball(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void active_update(float dt_sec) override;

  static std::string class_name() { return "pogo_snowball"; }
  static std::string display_name() { return _("Pogo Snowball"); }

  virtual bool is_freezable() const override { return true; }
  virtual bool is_flammable() const override { return false; }

private:
  float m_jump_timer;
};
