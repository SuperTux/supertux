//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_TINYGETTEXT_H
#define HEADER_TINYGETTEXT_H

#include <map>
#include <vector>
#include <set>
#include <string>

namespace TinyGetText {

typedef int (*PluralFunc)(int n);

struct LanguageDef {
  const char* code;
  const char* name;
  int         nplural;
  PluralFunc  plural;

  LanguageDef(const char* code_, const char* name_,  int nplural_, PluralFunc plural_)
    : code(code_), name(name_), nplural(nplural_), plural(plural_)
  {}
};

/** A simple dictionary class that mimics gettext() behaviour. Each
    Dictionary only works for a single language, for managing multiple
    languages and .po files at once use the DictionaryManager. */
class Dictionary
{
private:
  typedef std::map<std::string, std::string> Entries;
  Entries entries;

  typedef std::map<std::string, std::map<int, std::string> > PluralEntries;
  PluralEntries plural_entries;

  LanguageDef language;
  std::string charset;
public:
  /** */
  Dictionary(const LanguageDef& language_, const std::string& charset = "");

  Dictionary();

  /** Return the charset used for this dictionary */
  std::string get_charset() const;

  /** Set a charset for this dictionary, this will NOT convert stuff,
      it is for information only, you have to convert stuff yourself
      when you add it with \a add_translation() */
  void set_charset(const std::string& charset);

  /** Set the language that is used for this dictionary, this is
      mainly needed to evaluate plural forms */
  void set_language(const LanguageDef& lang);

  /** Translate the string \a msgid to its correct plural form, based
      on the number of items given by \a num. \a msgid2 is \a msgid in
      plural form. */
  std::string translate(const std::string& msgid, const std::string& msgid2, int num);

  /** Translate the string \a msgid. */
  std::string translate(const std::string& msgid);
  /** Translate the string \a msgid. */
  const char* translate(const char* msgid);

  /** Add a translation from \a msgid to \a msgstr to the dictionary,
      where \a msgid is the singular form of the message, msgid2 the
      plural form and msgstrs a table of translations. The right
      translation will be calculated based on the \a num argument to
      translate(). */
  void add_translation(const std::string& msgid, const std::string& msgid2,
                       const std::map<int, std::string>& msgstrs);

  /** Add a translation from \a msgid to \a msgstr to the
      dictionary */
  void add_translation(const std::string& msgid, const std::string& msgstr);
};

/** Manager class for dictionaries, you give it a bunch of directories
    with .po files and it will then automatically load the right file
    on demand depending on which language was set. */
class DictionaryManager
{
private:
  typedef std::map<std::string, Dictionary> Dictionaries;
  Dictionaries dictionaries;
  typedef std::vector<std::string> SearchPath;
  SearchPath search_path;
  typedef std::map<std::string, std::string> Aliases;
  Aliases language_aliases;
  std::string charset;
  std::string language;
  Dictionary* current_dict;
  Dictionary empty_dict;

public:
  DictionaryManager();

  /** Return the currently active dictionary, if none is set, an empty
      dictionary is returned. */
  Dictionary& get_dictionary()
  { return *current_dict; }

  /** Get dictionary for lang */
  Dictionary& get_dictionary(const std::string& langspec);

  /** Set a language based on a four? letter country code */
  void set_language(const std::string& langspec);

  /** returns the (normalized) country code of the currently used language */
  const std::string& get_language() const;

  /** Set a charset that will be set on the returned dictionaries */
  void set_charset(const std::string& charset);

  /** Define an alias for a language */
  void set_language_alias(const std::string& alias, const std::string& lang);

  /** Add a directory to the search path for dictionaries */
  void add_directory(const std::string& pathname);

  /** Return a set of the available languages in their country code */
  std::set<std::string> get_languages();

private:
  void parseLocaleAliases();
  /// returns the language part in a language spec (like de_DE.UTF-8 -> de)
  std::string get_language_from_spec(const std::string& spec);
};

/** Read the content of the .po file given as \a in into the
    dictionary given as \a dict */
void read_po_file(Dictionary& dict, std::istream& in);
LanguageDef& get_language_def(const std::string& name);

} // namespace TinyGetText

#endif

/* EOF */
