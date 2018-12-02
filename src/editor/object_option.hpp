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

#include <string>
#include <vector>

#include "gui/menu_action.hpp"

enum ObjectOptionFlag {
  OPTION_HIDDEN = (1 << 0)
};

class Color;
class Menu;
class Writer;

class ObjectOption
{
public:
  ObjectOption(MenuItemKind m_type, const std::string& text, const std::string& key, unsigned int flags);
  virtual ~ObjectOption();

  virtual void save(Writer& write) const = 0;
  virtual std::string to_string() const = 0;
  virtual void add_to_menu(Menu& menu) const = 0;

public:
  MenuItemKind m_type;
  std::string m_text;
  std::string m_key;
  int m_flags;

private:
  ObjectOption(const ObjectOption&) = delete;
  ObjectOption& operator=(const ObjectOption&) = delete;
};

class BoolObjectOption : public ObjectOption
{
public:
  BoolObjectOption(const std::string& text, bool* pointer, const std::string& key,
                   unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  bool* m_pointer;

private:
  BoolObjectOption(const BoolObjectOption&) = delete;
  BoolObjectOption& operator=(const BoolObjectOption&) = delete;
};

class IntObjectOption : public ObjectOption
{
public:
  IntObjectOption(const std::string& text, int* pointer, const std::string& key,
                  unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  int* m_pointer;

private:
  IntObjectOption(const IntObjectOption&) = delete;
  IntObjectOption& operator=(const IntObjectOption&) = delete;
};

class FloatObjectOption : public ObjectOption
{
public:
  FloatObjectOption(const std::string& text, float* pointer, const std::string& key,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  float* m_pointer;

private:
  FloatObjectOption(const FloatObjectOption&) = delete;
  FloatObjectOption& operator=(const FloatObjectOption&) = delete;
};

class StringObjectOption : public ObjectOption
{
public:
  StringObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                     unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* m_pointer;

private:
  StringObjectOption(const StringObjectOption&) = delete;
  StringObjectOption& operator=(const StringObjectOption&) = delete;
};

class StringSelectObjectOption : public ObjectOption
{
public:
  StringSelectObjectOption(const std::string& text, int* pointer, const std::vector<std::string>& select,
                           const std::string& key, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  int*  m_pointer;
  std::vector<std::string> m_select;

private:
  StringSelectObjectOption(const StringSelectObjectOption&) = delete;
  StringSelectObjectOption& operator=(const StringSelectObjectOption&) = delete;
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
  std::string* m_pointer;

private:
  ScriptObjectOption(const ScriptObjectOption&) = delete;
  ScriptObjectOption& operator=(const ScriptObjectOption&) = delete;
};

class FileObjectOption : public ObjectOption
{
public:
  FileObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                   std::vector<std::string> filter, unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  std::string* m_pointer;
  std::vector<std::string> m_filter;

private:
  FileObjectOption(const FileObjectOption&) = delete;
  FileObjectOption& operator=(const FileObjectOption&) = delete;
};

class ColorObjectOption : public ObjectOption
{
public:
  ColorObjectOption(const std::string& text, Color* pointer, const std::string& key,
                    unsigned int flags);

  virtual void save(Writer& write) const override;
  virtual std::string to_string() const override;
  virtual void add_to_menu(Menu& menu) const override;

private:
  Color* m_pointer;

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
  std::vector<std::string>* m_pointer;

private:
  BadGuySelectObjectOption(const BadGuySelectObjectOption&) = delete;
  BadGuySelectObjectOption& operator=(const BadGuySelectObjectOption&) = delete;
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

#endif

/* EOF */
