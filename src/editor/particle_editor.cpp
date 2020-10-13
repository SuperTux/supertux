//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "editor/particle_editor.hpp"

#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_filesystem.hpp"
#include "gui/mousecursor.hpp"
#include "math/easing.hpp"
#include "object/custom_particle_system.hpp"
#include "physfs/util.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/particle_editor_save_as.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/compositor.hpp"

#include <physfs.h>
#include <boost/algorithm/string/predicate.hpp>

bool
ParticleEditor::is_active()
{
  auto* self = ParticleEditor::current();
  return self && true;
}

ParticleEditor::ParticleEditor() :
  m_enabled(true),
  m_quit_request(false),
  m_controls(),
  m_undo_stack(),
  m_redo_stack(),
  m_saved_version(),
  m_particles(),
  m_filename("")
{
  reload();
}

void
ParticleEditor::reload()
{
  // TODO: Use a std::unique_ptr here
  if (m_particles)
    delete m_particles;

  auto doc = ReaderDocument::from_file((m_filename == "") ? "/particles/default.stcp" : m_filename);
  auto root = doc.get_root();
  auto mapping = root.get_mapping();

  if (root.get_name() != "supertux-custom-particle")
    throw std::runtime_error("file is not a supertux-custom-particle file.");

  m_particles = new CustomParticleSystem(mapping);

  m_controls.clear();

  // TODO: Use the addButton() command
  // Texture button start
  auto texture_btn = std::make_unique<ControlButton>("Change texture...");
  texture_btn.get()->m_on_change = new std::function<void()>([this](){
    const std::vector<std::string>& filter = {".jpg", ".png", ".surface"};
    MenuManager::instance().push_menu(std::make_unique<FileSystemMenu>(
      &(m_particles->m_particle_main_texture),
      filter,
      "/",
      [this](std::string new_filename) { m_particles->reinit_textures(); }
    ));
  });
  float tmp_height = 0.f;
  for (auto& control : m_controls) {
    tmp_height = std::max(tmp_height, control->get_rect().get_bottom() + 5.f);
  }
  texture_btn.get()->set_rect(Rectf(25.f, tmp_height, 325.f, tmp_height + 20.f));
  m_controls.push_back(std::move(texture_btn));
  // Texture button end

  addTextboxInt(_("Max amount"), &(m_particles->m_max_amount), [](ControlTextboxInt* ctrl, int i)
    {
      if (i < 0) {
        ctrl->set_value(0);
        return false;
      } else if (i > 500) {
        ctrl->set_value(500);
        return false;
      } else {
        return true;
      }
    }
  );
  addTextboxFloat(_("Delay"), &(m_particles->m_delay));
  addCheckbox(_("Spawn anywhere"), &(m_particles->m_cover_screen));
  addTextboxFloatWithImprecision(_("Life duration"),
                                 &(m_particles->m_particle_lifetime),
                                 &(m_particles->m_particle_lifetime_variation),
                                 [](ControlTextboxFloat* ctrl, float f){ return f > 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloatWithImprecision(_("Birth duration"),
                                 &(m_particles->m_particle_birth_time),
                                 &(m_particles->m_particle_birth_time_variation),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloatWithImprecision(_("Death duration"),
                                 &(m_particles->m_particle_death_time),
                                 &(m_particles->m_particle_death_time_variation),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });

  auto birth_mode = std::make_unique<ControlEnum<CustomParticleSystem::FadeMode> >();
  birth_mode.get()->add_option(CustomParticleSystem::FadeMode::Shrink, _("Grow"));
  birth_mode.get()->add_option(CustomParticleSystem::FadeMode::Fade, _("Fade in"));
  birth_mode.get()->add_option(CustomParticleSystem::FadeMode::None, _("None"));
  birth_mode.get()->bind_value(&(m_particles->m_particle_birth_mode));
  addControl(_("Birth mode"), std::move(birth_mode));
  auto death_mode = std::make_unique<ControlEnum<CustomParticleSystem::FadeMode> >();
  death_mode.get()->add_option(CustomParticleSystem::FadeMode::Shrink, _("Shrink"));
  death_mode.get()->add_option(CustomParticleSystem::FadeMode::Fade, _("Fade out"));
  death_mode.get()->add_option(CustomParticleSystem::FadeMode::None, _("None"));
  death_mode.get()->bind_value(&(m_particles->m_particle_death_mode));
  addControl(_("Death mode"), std::move(death_mode));

  addEasingEnum(_("Birth easing"), &(m_particles->m_particle_birth_easing));
  addEasingEnum(_("Death easing"), &(m_particles->m_particle_death_easing));

  addTextboxFloatWithImprecision(_("Horizontal speed"),
                                 &(m_particles->m_particle_speed_x),
                                 &(m_particles->m_particle_speed_variation_x),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloatWithImprecision(_("Vertical speed"),
                                 &(m_particles->m_particle_speed_y),
                                 &(m_particles->m_particle_speed_variation_y),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloat(_("Horizontal acceleration"), &(m_particles->m_particle_acceleration_x));
  addTextboxFloat(_("Vertical acceleration"), &(m_particles->m_particle_acceleration_y));
  addTextboxFloat(_("Horizontal friction"), &(m_particles->m_particle_friction_x));
  addTextboxFloat(_("Vertical friction"), &(m_particles->m_particle_friction_y));
  addTextboxFloat(_("Feather factor"), &(m_particles->m_particle_feather_factor));
  addTextboxFloatWithImprecision(_("Initial rotation"),
                                 &(m_particles->m_particle_rotation),
                                 &(m_particles->m_particle_rotation_variation),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloatWithImprecision(_("Rotation speed"),
                                 &(m_particles->m_particle_rotation_speed),
                                 &(m_particles->m_particle_rotation_speed_variation),
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; },
                                 [](ControlTextboxFloat* ctrl, float f){ return f >= 0.f; });
  addTextboxFloat(_("Rotation acceleration"), &(m_particles->m_particle_rotation_acceleration));
  addTextboxFloat(_("Rotation friction/decceleration"), &(m_particles->m_particle_rotation_decceleration));

  auto rotation_mode = std::make_unique<ControlEnum<CustomParticleSystem::RotationMode> >();
  rotation_mode.get()->add_option(CustomParticleSystem::RotationMode::Wiggling, _("Wiggling"));
  rotation_mode.get()->add_option(CustomParticleSystem::RotationMode::Facing, _("Facing"));
  rotation_mode.get()->add_option(CustomParticleSystem::RotationMode::Fixed, _("Fixed"));
  rotation_mode.get()->bind_value(&(m_particles->m_particle_rotation_mode));
  addControl(_("Rotation mode"), std::move(rotation_mode));

  auto collision_mode = std::make_unique<ControlEnum<CustomParticleSystem::CollisionMode> >();
  collision_mode.get()->add_option(CustomParticleSystem::CollisionMode::Destroy, _("Destroy"));
  collision_mode.get()->add_option(CustomParticleSystem::CollisionMode::BounceLight, _("Bounce (light)"));
  collision_mode.get()->add_option(CustomParticleSystem::CollisionMode::BounceHeavy, _("Bounce (heavy)"));
  collision_mode.get()->add_option(CustomParticleSystem::CollisionMode::Stick, _("Stick to surface"));
  collision_mode.get()->add_option(CustomParticleSystem::CollisionMode::Ignore, _("No collision"));
  collision_mode.get()->bind_value(&(m_particles->m_particle_collision_mode));
  addControl(_("Collision mode"), std::move(collision_mode));

  auto offscreen_mode = std::make_unique<ControlEnum<CustomParticleSystem::OffscreenMode> >();
  offscreen_mode.get()->add_option(CustomParticleSystem::OffscreenMode::Always, _("Always destroy"));
  offscreen_mode.get()->add_option(CustomParticleSystem::OffscreenMode::OnlyOnExit, _("Only on exit"));
  offscreen_mode.get()->add_option(CustomParticleSystem::OffscreenMode::Never, _("Never"));
  offscreen_mode.get()->bind_value(&(m_particles->m_particle_offscreen_mode));
  addControl(_("Offscreen mode"), std::move(offscreen_mode));

  // FIXME: add some ParticleEditor::addButton() function so that I don't have to put all that in here
  auto clear_btn = std::make_unique<ControlButton>("Clear");
  clear_btn.get()->m_on_change = new std::function<void()>([this](){ m_particles->clear(); });
  float height = 0.f;
  for (auto& control : m_controls) {
    height = std::max(height, control->get_rect().get_bottom() + 5.f);
  }
  clear_btn.get()->set_rect(Rectf(25.f, height, 325.f, height + 20.f));
  m_controls.push_back(std::move(clear_btn));

  m_undo_stack.clear();
  m_saved_version = m_particles->get_props();
  m_undo_stack.push_back(m_saved_version);
}

ParticleEditor::~ParticleEditor()
{
}

void
ParticleEditor::addTextboxFloat(std::string name, float* bind, bool (*float_validator)(ControlTextboxFloat*, float))
{
  auto float_control = std::make_unique<ControlTextboxFloat>();
  float_control.get()->bind_value(bind);
  float_control.get()->m_validate_float = float_validator;
  addControl(name, std::move(float_control));
}

void
ParticleEditor::addTextboxFloatWithImprecision(std::string name, float* bind,
                                      float* imprecision_bind,
                                      bool (*float_validator)(ControlTextboxFloat*, float),
                                      bool (*imp_validator)(ControlTextboxFloat*, float))
{
  auto float_control = std::make_unique<ControlTextboxFloat>();
  float_control.get()->bind_value(bind);
  float_control.get()->m_validate_float = float_validator;

  auto imp_control = std::make_unique<ControlTextboxFloat>();
  imp_control.get()->bind_value(imprecision_bind);
  imp_control.get()->m_validate_float = imp_validator;

  // Can't use addControl() because this is a special case
  float height = 0.f;
  for (auto& control : m_controls) {
    height = std::max(height, control->get_rect().get_bottom() + 5.f);
  }

  float_control.get()->set_rect(Rectf(150.f, height, 235.f, height + 20.f));
  imp_control.get()->set_rect(Rectf(265.f, height, 350.f, height + 20.f));

  float_control.get()->m_label = new InterfaceLabel(Rectf(5.f, height, 145.f, height + 20.f), name);
  imp_control.get()->m_label = new InterfaceLabel(Rectf(240.f, height, 260.f, height + 20.f), "±");

  float_control.get()->m_on_change = new std::function<void()>([this](){ this->push_version(); });
  imp_control.get()->m_on_change = new std::function<void()>([this](){ this->push_version(); });

  m_controls.push_back(std::move(float_control));
  m_controls.push_back(std::move(imp_control));
}

void
ParticleEditor::addTextboxInt(std::string name, int* bind, bool (*int_validator)(ControlTextboxInt*, int))
{
  auto int_control = std::make_unique<ControlTextboxInt>();
  int_control.get()->bind_value(bind);
  int_control.get()->m_validate_int = int_validator;
  addControl(name, std::move(int_control));
}

void
ParticleEditor::addCheckbox(std::string name, bool* bind)
{
  auto bool_control = std::make_unique<ControlCheckbox>();
  bool_control.get()->bind_value(bind);
  bool_control.get()->set_rect(Rectf(240.f, 0.f, 260.f, 20.f));
  addControl(name, std::move(bool_control));
}

void
ParticleEditor::addEasingEnum(std::string name, EasingMode* bind)
{
  // FIXME: They don't render in the same order as in here
  auto ease_ctrl = std::make_unique<ControlEnum<EasingMode> >();
  ease_ctrl.get()->add_option(EaseNone, _(getEasingName(EaseNone)));
  ease_ctrl.get()->add_option(EaseQuadIn, _(getEasingName(EaseQuadIn)));
  ease_ctrl.get()->add_option(EaseQuadOut, _(getEasingName(EaseQuadOut)));
  ease_ctrl.get()->add_option(EaseQuadInOut, _(getEasingName(EaseQuadInOut)));
  ease_ctrl.get()->add_option(EaseCubicIn, _(getEasingName(EaseCubicIn)));
  ease_ctrl.get()->add_option(EaseCubicOut, _(getEasingName(EaseCubicOut)));
  ease_ctrl.get()->add_option(EaseCubicInOut, _(getEasingName(EaseCubicInOut)));
  ease_ctrl.get()->add_option(EaseQuartIn, _(getEasingName(EaseQuartIn)));
  ease_ctrl.get()->add_option(EaseQuartOut, _(getEasingName(EaseQuartOut)));
  ease_ctrl.get()->add_option(EaseQuartInOut, _(getEasingName(EaseQuartInOut)));
  ease_ctrl.get()->add_option(EaseQuintIn, _(getEasingName(EaseQuintIn)));
  ease_ctrl.get()->add_option(EaseQuintOut, _(getEasingName(EaseQuintOut)));
  ease_ctrl.get()->add_option(EaseQuintInOut, _(getEasingName(EaseQuintInOut)));
  ease_ctrl.get()->add_option(EaseSineIn, _(getEasingName(EaseSineIn)));
  ease_ctrl.get()->add_option(EaseSineOut, _(getEasingName(EaseSineOut)));
  ease_ctrl.get()->add_option(EaseSineInOut, _(getEasingName(EaseSineInOut)));
  ease_ctrl.get()->add_option(EaseCircularIn, _(getEasingName(EaseCircularIn)));
  ease_ctrl.get()->add_option(EaseCircularOut, _(getEasingName(EaseCircularOut)));
  ease_ctrl.get()->add_option(EaseCircularInOut, _(getEasingName(EaseCircularInOut)));
  ease_ctrl.get()->add_option(EaseExponentialIn, _(getEasingName(EaseExponentialIn)));
  ease_ctrl.get()->add_option(EaseExponentialOut, _(getEasingName(EaseExponentialOut)));
  ease_ctrl.get()->add_option(EaseExponentialInOut, _(getEasingName(EaseExponentialInOut)));
  ease_ctrl.get()->add_option(EaseElasticIn, _(getEasingName(EaseElasticIn)));
  ease_ctrl.get()->add_option(EaseElasticOut, _(getEasingName(EaseElasticOut)));
  ease_ctrl.get()->add_option(EaseElasticInOut, _(getEasingName(EaseElasticInOut)));
  ease_ctrl.get()->add_option(EaseBackIn, _(getEasingName(EaseBackIn)));
  ease_ctrl.get()->add_option(EaseBackOut, _(getEasingName(EaseBackOut)));
  ease_ctrl.get()->add_option(EaseBackInOut, _(getEasingName(EaseBackInOut)));
  ease_ctrl.get()->add_option(EaseBounceIn, _(getEasingName(EaseBounceIn)));
  ease_ctrl.get()->add_option(EaseBounceOut, _(getEasingName(EaseBounceOut)));
  ease_ctrl.get()->add_option(EaseBounceInOut, _(getEasingName(EaseBounceInOut)));
  ease_ctrl.get()->bind_value(bind);
  addControl(name, std::move(ease_ctrl));
}

void
ParticleEditor::addControl(std::string name, std::unique_ptr<InterfaceControl> new_control)
{
  float height = 0.f;
  for (auto& control : m_controls) {
    height = std::max(height, control->get_rect().get_bottom() + 5.f);
  }

  if (new_control.get()->get_rect().get_width() == 0.f || new_control.get()->get_rect().get_height() == 0.f) {
    new_control.get()->set_rect(Rectf(150.f, height, 350.f, height + 20.f));
  } else {
    new_control.get()->set_rect(Rectf(new_control.get()->get_rect().get_left(),
                                      height,
                                      new_control.get()->get_rect().get_right(),
                                      height + new_control.get()->get_rect().get_height()));
  }

  new_control.get()->m_label = new InterfaceLabel(Rectf(5.f, height, 135.f, height + 20.f), name);
  new_control.get()->m_on_change = new std::function<void()>([this](){ this->push_version(); });
  m_controls.push_back(std::move(new_control));
}


void
ParticleEditor::save(const std::string& filepath_, bool retry)
{
  std::string filepath = filepath_;
  if (!boost::algorithm::ends_with(filepath, ".stcp"))
    filepath += ".stcp";

  //FIXME: It tests for directory in supertux/data, but saves into .supertux2.
  try {
    { // make sure the level directory exists
      std::string dirname = FileSystem::dirname(filepath);
      if (!PHYSFS_exists(dirname.c_str()))
      {
        if (!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for particle config '"
              << dirname << "': " <<PHYSFS_getLastErrorCode();
          throw std::runtime_error(msg.str());
        }
      }

      if (!physfsutil::is_directory(dirname))
      {
        std::ostringstream msg;
        msg << "Particle config path '" << dirname << "' is not a directory";
        throw std::runtime_error(msg.str());
      }
    }

    Writer writer(filepath);
    save(writer);
    m_filename = filepath;
    m_saved_version = m_undo_stack.back();
    log_warning << "Particle config saved as " << filepath << "." << std::endl;
  } catch(std::exception& e) {
    if (retry) {
      std::stringstream msg;
      msg << "Problem when saving particle config '" << filepath << "': " << e.what();
      throw std::runtime_error(msg.str());
    } else {
      log_warning << "Failed to save the particle configuration, retrying..." << std::endl;
      { // create the level directory again
        std::string dirname = FileSystem::dirname(filepath);
        if (!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for particle config '"
              << dirname << "': " <<PHYSFS_getLastErrorCode();
          throw std::runtime_error(msg.str());
        }
      }
      save(filepath, true);
    }
  }
}

void
ParticleEditor::save(Writer& writer)
{
  writer.start_list("supertux-custom-particle");
  // Starts writing to supertux particle file. Keep this at the very beginning.

  m_particles->save(writer);

  // Ends writing to supertux particle file. Keep this at the very end.
  writer.end_list("supertux-custom-particle");
}

void
ParticleEditor::request_save(bool is_save_as, std::function<void(bool)> callback)
{
  if (is_save_as || m_filename == "")
  {
    m_enabled = false;
    MenuManager::instance().set_menu(std::make_unique<ParticleEditorSaveAs>(callback));
  }
  else
  {
    save(m_filename);
    callback(true);
  }
}

void
ParticleEditor::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  m_particles->draw(context);

  /*context.color().draw_filled_rect(Rectf(0.f, 0.f, 255.f, context.get_height()),
                                   Color(),
                                   LAYER_GUI - 1);*/
  context.color().draw_gradient(Color(0.05f, 0.1f, 0.1f, 1.f),
                                Color(0.1f, 0.15f, 0.15f, 1.f),
                                LAYER_GUI - 1,
                                GradientDirection::HORIZONTAL,
                                Rectf(0.f, 0.f, 355.f, float(context.get_height())));

  for(const auto& control : m_controls) {
    control->draw(context);
  }

  MouseCursor::current()->draw(context);
}

void
ParticleEditor::update(float dt_sec, const Controller& controller)
{

  update_keyboard(controller);

  if (m_quit_request) {
    quit_editor();
  }

  for(const auto& control : m_controls) {
    control->update(dt_sec, controller);
  }

  // Uncomment to make the particles stop updating on pause
  //if (!m_enabled)
  //  return;

  m_particles->update(dt_sec);
}

void
ParticleEditor::update_keyboard(const Controller& controller)
{
  if (!m_enabled)
    return;

  if (controller.pressed(Control::ESCAPE)) {
    m_enabled = false;
    MenuManager::instance().set_menu(MenuStorage::PARTICLE_EDITOR_MENU);
    return;
  }
}

void
ParticleEditor::quit_editor()
{
  m_quit_request = false;

  auto quit = [] ()
  {
    //Quit particle editor
    /*m_world = nullptr;
    m_levelfile = "";
    m_levelloaded = false;
    m_enabled = false;
    Tile::draw_editor_images = false;*/
    ScreenManager::current()->pop_screen();
    if (Editor::current()) {
      Editor::current()->m_reactivate_request = true;
    }
  };

  check_unsaved_changes([quit] {
    quit();
  });
}

void
ParticleEditor::leave()
{
  m_enabled = false;
}

void
ParticleEditor::setup()
{
  m_enabled = true;
}

void
ParticleEditor::check_unsaved_changes(const std::function<void ()>& action)
{
  if (m_undo_stack.back() != m_saved_version)
  {
    m_enabled = false;
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(_("This particle configuration contains unsaved changes, do you want to save?"));
    dialog->add_button(_("Save"), [this, action] {
      request_save(false, [this, action] (bool was_saved) {
        m_enabled = true;
        if (was_saved)
          action();
      });
    });
    dialog->add_default_button(_("Save as"), [this, action] {
      request_save(true, [this, action] (bool was_saved) {
        m_enabled = true;
        if (was_saved)
          action();
      });
    });
    dialog->add_button(_("No"), [this, action] {
      action();
      m_enabled = true;
    });
    dialog->add_button(_("Cancel"), [this] {
      m_enabled = true;
    });
    MenuManager::instance().set_dialog(std::move(dialog));
  }
  else
  {
    action();
  }
}

void
ParticleEditor::reactivate()
{
  m_enabled = true;
}

void
ParticleEditor::open_particle_directory()
{
  auto path = FileSystem::join(PHYSFS_getWriteDir(), "/particles/custom/");
  FileSystem::open_path(path);
}

void
ParticleEditor::event(const SDL_Event& ev)
{
  if (!m_enabled) return;

  for(const auto& control : m_controls) {
    if (control->event(ev))
      break;
  }

  if (ev.type == SDL_KEYDOWN &&
      ev.key.keysym.sym == SDLK_z &&
      ev.key.keysym.mod & KMOD_CTRL) {
    undo();
  }

  if (ev.type == SDL_KEYDOWN &&
      ev.key.keysym.sym == SDLK_y &&
     ev.key.keysym.mod & KMOD_CTRL) {
    redo();
  }

  if (ev.type == SDL_KEYDOWN &&
      ev.key.keysym.sym == SDLK_s &&
      ev.key.keysym.mod & KMOD_CTRL) {
    request_save(ev.key.keysym.mod & KMOD_SHIFT);
  }

  if (ev.type == SDL_KEYDOWN &&
      ev.key.keysym.sym == SDLK_o &&
     ev.key.keysym.mod & KMOD_CTRL) {
    MenuManager::instance().set_menu(MenuStorage::PARTICLE_EDITOR_OPEN);
  }

/*
  if (!m_enabled) return;

  try
  {
	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_t &&
        ev.key.keysym.mod & KMOD_CTRL) {
		test_level(boost::none);
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_s &&
        ev.key.keysym.mod & KMOD_CTRL) {
		save_level();
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_z &&
        ev.key.keysym.mod & KMOD_CTRL) {
		undo();
		}

	if (ev.type == SDL_KEYDOWN &&
        ev.key.keysym.sym == SDLK_y &&
        ev.key.keysym.mod & KMOD_CTRL) {
		redo();
		}

    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F6) {
      Compositor::s_render_lighting = !Compositor::s_render_lighting;
      return;
    }

    m_ignore_sector_change = false;

    BIND_SECTOR(*m_sector);

    for(const auto& widget : m_controls) {
      if (widget->event(ev))
        break;
    }

    // unreliable heuristic to snapshot the current state for future undo
    if (((ev.type == SDL_KEYUP && ev.key.repeat == 0 &&
         ev.key.keysym.sym != SDLK_LSHIFT &&
         ev.key.keysym.sym != SDLK_RSHIFT &&
         ev.key.keysym.sym != SDLK_LCTRL &&
         ev.key.keysym.sym != SDLK_RCTRL) ||
         ev.type == SDL_MOUSEBUTTONUP))
    {
      if (!m_ignore_sector_change) {
        if (m_level) {
          m_undo_manager->try_snapshot(*m_level);
        }
      }
    }

    // Scroll with mouse wheel, if the mouse is not over the toolbox.
    // The toolbox does scrolling independently from the main area.
    if (ev.type == SDL_MOUSEWHEEL && !m_toolbox_widget->has_mouse_focus() && !m_layers_widget->has_mouse_focus()) {
      float scroll_x = static_cast<float>(ev.wheel.x * -32);
      float scroll_y = static_cast<float>(ev.wheel.y * -32);
      scroll({scroll_x, scroll_y});
    }
  }
  catch(const std::exception& err)
  {
    log_warning << "error while processing ParticleEditor::event(): " << err.what() << std::endl;
  }*/
}

void
ParticleEditor::push_version()
{
  m_redo_stack.clear();
  m_undo_stack.push_back(m_particles->get_props());
}

void
ParticleEditor::undo()
{
  if (m_undo_stack.size() < 2)
    return;

  m_redo_stack.push_back(m_undo_stack.back());
  m_undo_stack.pop_back();
  m_particles->set_props(m_undo_stack.back());
}

void
ParticleEditor::redo()
{
  if (m_redo_stack.size() < 1)
    return;

  m_undo_stack.push_back(m_redo_stack.back());
  m_particles->set_props(m_redo_stack.back());
  m_redo_stack.pop_back();
}

/* EOF */
