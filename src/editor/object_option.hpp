//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_OPTION_HPP

#include <boost/optional.hpp>
#include <string>
#include <vector>

#include <sexp/value.hpp>

#include "gui/menu_action.hpp"
#include "object/path_walker.hpp"
#include "video/color.hpp"

enum ObjectOptionFlag {
  /** Set if the value is a hidden implementation detail that
      shouldn't be exposed to the user */
  OPTION_HIDDEN = (1 << 0),

  /** Set if the text should be saved as translatable */
  OPTION_TRANSLATABLE = (1 << 1)
};

namespace sexp {
class Value;
} // namespace sexp
class Color;
class Menu;
class Path;
class PathObject;
class Rectf;
class TileMap;
class Writer;

class ObjectOption
{
public:
  ObjectOption(const std::string& text, const std::string& key, unsigned int flags);
  virtual ~ObjectOption();

  virtual void save(Writer& write) const = 0;
  virtual std::string to_string() const = 0;
  virtual void add_to_menu(Menu& menu) const = 0;

  const std::string& get_key() const { return m_key; }
  const std::string& get_text() const { return m_text; }
  unsigned int get_flags() const { return m_flags; }

protected:
  const std::string m_text;
  const std::string m_key;
  const unsigned int m_flags;

private:
  ObjectOption(const ObjectOption&) = delete;
  ObjectOption& operator=(const ObjectOption&) = delete;
};

class BoolObjectOption : public ObjectOption
{
public:
  BoolObjectOption(const std::string& text, bool* pointer, const std::string& key,
                   boost::optional<bool> default_value,
                   unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  bool* const m_pointer;
  const boost::optional<bool> m_default_value;

private:
  BoolObjectOption(const BoolObjectOption&) = delete;
  BoolObjectOption& operator=(const BoolObjectOption&) = delete;
};

class IntObjectOption : public ObjectOption
{
public:
  IntObjectOption(const std::string& text, int* pointer, const std::string& key,
                  boost::optional<int> default_value,
                  unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  int* const m_pointer;
  const boost::optional<int> m_default_value;

private:
  IntObjectOption(const IntObjectOption&) = delete;
  IntObjectOption& operator=(const IntObjectOption&) = delete;
};

class LabelObjectOption : public ObjectOption
{
public:
  LabelObjectOption(const std::string& text,
                  unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  LabelObjectOption(const LabelObjectOption&) = delete;
  LabelObjectOption& operator=(const LabelObjectOption&) = delete;
};

class RectfObjectOption : public ObjectOption
{
public:
  RectfObjectOption(const std::string& text, Rectf* pointer, const std::string& key,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  Rectf* const m_pointer;
  float m_width;
  float m_height;

private:
  RectfObjectOption(const RectfObjectOption&) = delete;
  RectfObjectOption& operator=(const RectfObjectOption&) = delete;
};

class FloatObjectOption : public ObjectOption
{
public:
  FloatObjectOption(const std::string& text, float* pointer, const std::string& key,
                    boost::optional<float> default_value,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  float* const m_pointer;
  const boost::optional<float> m_default_value;

private:
  FloatObjectOption(const FloatObjectOption&) = delete;
  FloatObjectOption& operator=(const FloatObjectOption&) = delete;
};

class StringObjectOption : public ObjectOption
{
public:
  StringObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                     boost::optional<std::string> default_value,
                     unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* const m_pointer;
  boost::optional<std::string> m_default_value;

private:
  StringObjectOption(const StringObjectOption&) = delete;
  StringObjectOption& operator=(const StringObjectOption&) = delete;
};

class StringMultilineObjectOption : public ObjectOption
{
public:
  StringMultilineObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                     boost::optional<std::string> default_value,
                     unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* const m_pointer;
  boost::optional<std::string> m_default_value;

private:
  StringMultilineObjectOption(const StringMultilineObjectOption&) = delete;
  StringMultilineObjectOption& operator=(const StringMultilineObjectOption&) = delete;
};

class StringSelectObjectOption : public ObjectOption
{
public:
  StringSelectObjectOption(const std::string& text, int* pointer, const std::vector<std::string>& select,
                           boost::optional<int> default_value,
                           const std::string& key, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  int* const m_pointer;
  const std::vector<std::string> m_select;
  const boost::optional<int> m_default_value;

private:
  StringSelectObjectOption(const StringSelectObjectOption&) = delete;
  StringSelectObjectOption& operator=(const StringSelectObjectOption&) = delete;
};

class EnumObjectOption : public ObjectOption
{
public:
  EnumObjectOption(const std::string& text, int* pointer,
                   const std::vector<std::string>& labels,
                   const std::vector<std::string>& symbols,
                   boost::optional<int> default_value,
                   const std::string& key, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  int* const m_pointer;
  const std::vector<std::string> m_labels;
  const std::vector<std::string> m_symbols;
  const boost::optional<int> m_default_value;

private:
  EnumObjectOption(const EnumObjectOption&) = delete;
  EnumObjectOption& operator=(const EnumObjectOption&) = delete;
};

class ScriptObjectOption : public ObjectOption
{
public:
  ScriptObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                     unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* const m_pointer;

private:
  ScriptObjectOption(const ScriptObjectOption&) = delete;
  ScriptObjectOption& operator=(const ScriptObjectOption&) = delete;
};

class FileObjectOption : public ObjectOption
{
public:
  FileObjectOption(const std::string& text, std::string* pointer,
                   boost::optional<std::string> default_value,
                   const std::string& key,
                   std::vector<std::string> filter,
                   const std::string& basedir,
                   bool path_relative_to_basedir,
                   unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* const m_pointer;
  boost::optional<std::string> m_default_value;
  const std::vector<std::string> m_filter;
  std::string m_basedir;
  bool m_path_relative_to_basedir;

private:
  FileObjectOption(const FileObjectOption&) = delete;
  FileObjectOption& operator=(const FileObjectOption&) = delete;
};

class ColorObjectOption : public ObjectOption
{
public:
  ColorObjectOption(const std::string& text, Color* pointer, const std::string& key,
                    boost::optional<Color> default_value, bool use_alpha,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  Color* const m_pointer;
  const boost::optional<Color> m_default_value;
  bool m_use_alpha;

private:
  ColorObjectOption(const ColorObjectOption&) = delete;
  ColorObjectOption& operator=(const ColorObjectOption&) = delete;
};

class BadGuySelectObjectOption : public ObjectOption
{
public:
  BadGuySelectObjectOption(const std::string& text, std::vector<std::string>* pointer, const std::string& key,
                           unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::vector<std::string>* const m_pointer;

private:
  BadGuySelectObjectOption(const BadGuySelectObjectOption&) = delete;
  BadGuySelectObjectOption& operator=(const BadGuySelectObjectOption&) = delete;
};

class TilesObjectOption : public ObjectOption
{
public:
  TilesObjectOption(const std::string& text, TileMap* tilemap, const std::string& key,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  TileMap* m_tilemap;

private:
  TilesObjectOption(const TilesObjectOption&) = delete;
  TilesObjectOption& operator=(const TilesObjectOption&) = delete;
};

class PathObjectOption : public ObjectOption
{
public:
  PathObjectOption(const std::string& text, Path* path, const std::string& key,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  Path* m_path;

private:
  PathObjectOption(const PathObjectOption&) = delete;
  PathObjectOption& operator=(const PathObjectOption&) = delete;
};

class PathRefObjectOption : public ObjectOption
{
public:
  PathRefObjectOption(const std::string& text, PathObject& target, const std::string& path_ref,
                      const std::string& key, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string m_path_ref;
  PathObject& m_target;

private:
  PathRefObjectOption(const PathRefObjectOption&) = delete;
  PathRefObjectOption& operator=(const PathRefObjectOption&) = delete;
};

class SExpObjectOption : public ObjectOption
{
public:
  SExpObjectOption(const std::string& text, const std::string& key, sexp::Value& value, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  sexp::Value m_sx;

private:
  SExpObjectOption(const SExpObjectOption&) = delete;
  SExpObjectOption& operator=(const SExpObjectOption&) = delete;
};

class PathHandleOption : public ObjectOption
{
public:
  PathHandleOption(const std::string& text, PathWalker::Handle& handle,
                   const std::string& key, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  PathWalker::Handle& m_target;

private:
  PathHandleOption(const PathHandleOption&) = delete;
  PathHandleOption& operator=(const PathHandleOption&) = delete;
};

class RemoveObjectOption : public ObjectOption
{
public:
  RemoveObjectOption();

  virtual void save(Writer& write) const override {}
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  RemoveObjectOption(const RemoveObjectOption&) = delete;
  RemoveObjectOption& operator=(const RemoveObjectOption&) = delete;
};

class TestFromHereOption : public ObjectOption
{
public:
  TestFromHereOption();

  virtual void save(Writer& write) const override {}
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  TestFromHereOption(const TestFromHereOption&) = delete;
  TestFromHereOption& operator=(const TestFromHereOption&) = delete;
};

class ParticleEditorOption : public ObjectOption
{
public:
  ParticleEditorOption();

  virtual void save(Writer& write) const override {}
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  ParticleEditorOption(const ParticleEditorOption&) = delete;
  ParticleEditorOption& operator=(const ParticleEditorOption&) = delete;
};

class ButtonOption : public ObjectOption
{
public:
  ButtonOption(const std::string& text, std::function<void()> callback);

  virtual void save(Writer& write) const override {}
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::function<void()> m_callback;

private:
  ButtonOption(const ButtonOption&) = delete;
  ButtonOption& operator=(const ButtonOption&) = delete;
};

class StringArrayOption : public ObjectOption
{
public:
  StringArrayOption(const std::string& text, const std::string& key, std::vector<std::string>& items);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override { return "text-area"; }
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::vector<std::string>& m_items;

private:
  StringArrayOption(const StringArrayOption&) = delete;
  StringArrayOption& operator=(const StringArrayOption&) = delete;
};

#endif

/* EOF */
