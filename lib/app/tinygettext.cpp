//  $Id$
// 
//  TinyGetText - A small flexible gettext() replacement
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include <sys/types.h>
#include <iconv.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <errno.h>
#include "tinygettext.h"

//#define TRANSLATION_DEBUG

namespace TinyGetText {

/** Convert \a which is in \a from_charset to \a to_charset and return it */
std::string convert(const std::string& text,
                    const std::string& from_charset,
                    const std::string& to_charset)           
{
  if (from_charset == to_charset)
    return text;

  iconv_t cd = iconv_open(to_charset.c_str(), from_charset.c_str());
  
  size_t in_len = text.length();
  size_t out_len = text.length()*2;

  char*  out_orig = new char[out_len]; // FIXME: cross fingers that this is enough
  char*  in_orig  = new char[in_len+1];
  strcpy(in_orig, text.c_str());

  char* out = out_orig;
  char* in  = in_orig;

  //std::cout << "IN: " << (int)in << " " << in_len << " " << (int)out << " " << out_len << std::endl;
  int retval = iconv(cd, &in, &in_len, &out, &out_len);
  //std::cout << "OUT: " << (int)in << " " << in_len << " " << (int)out << " " << out_len << std::endl;

  if (retval != 0)
    {
      std::cerr << strerror(errno) << std::endl;
      std::cerr << "Error: conversion from " << from_charset
                << " to " << to_charset << " went wrong: " << retval << std::endl;
    }
  iconv_close(cd);

  std::string ret(out_orig, out_len);
  delete[] out_orig;
  delete[] in_orig;
  return ret;
}

bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(lhs.length() - rhs.length(), rhs.length(), rhs) == 0;
}

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

int plural1(int )     { return 0; }
int plural2_1(int n)  { return (n != 1); }
int plural2_2(int n)  { return (n > 1); }
int plural3_lv(int n) { return (n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2); }
int plural3_ga(int n) { return n==1 ? 0 : n==2 ? 1 : 2; }
int plural3_lt(int n) { return (n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_1(int n)  { return (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_sk(int n) { return (n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2; }
int plural3_pl(int n) { return (n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_sl(int n) { return (n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3); }

/** Language Definitions */
//*{
LanguageDef lang_hu("hu", "Hungarian",         1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_ja("ja", "Japanese",          1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_ko("ko", "Korean",            1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_tr("tr", "Turkish",           1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_da("da", "Danish",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nl("nl", "Dutch",             2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_en("en", "English",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fo("fo", "Faroese",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_de("de", "German",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nb("nb", "Norwegian Bokmal",  2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_no("no", "Norwegian",         2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nn("nn", "Norwegian Nynorsk", 2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_sv("sv", "Swedish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_et("et", "Estonian",          2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fi("fi", "Finnish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_el("el", "Greek",             2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_he("he", "Hebrew",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_it("it", "Italian",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_pt("pt", "Portuguese",        2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_es("es", "Spanish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_eo("eo", "Esperanto",         2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fr("fr", "French",            2, plural2_2); // "nplurals=2; plural=(n > 1);"
LanguageDef lang_pt_BR("pt_BR", "Brazilian",   2, plural2_2); // "nplurals=2; plural=(n > 1);"
LanguageDef lang_lv("lv", "Latvian",           3, plural3_lv); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2);"
LanguageDef lang_ga("ga", "Irish",             3, plural3_ga); // "nplurals=3; plural=n==1 ? 0 : n==2 ? 1 : 2;"
LanguageDef lang_lt("lt", "Lithuanian",        3, plural3_lt); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_hr("hr", "Croatian",          3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_cs("cs", "Czech",             3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_ru("ru", "Russian",           3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_uk("uk", "Ukrainian",         3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_sk("sk", "Slovak",            3, plural3_sk); // "nplurals=3; plural=(n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2;"
LanguageDef lang_pl("pl", "Polish",            3, plural3_pl); // "nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
LanguageDef lang_sl("sl", "Slovenian",         3, plural3_sl); // "nplurals=4; plural=(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3);"
//*}

LanguageDef&
get_language_def(const std::string& name)
{
  if (name == "hu") return lang_hu;
  else if (name == "ja") return lang_ja;
  else if (name == "ko") return lang_ko;
  else if (name == "tr") return lang_tr;
  else if (name == "da") return lang_da;
  else if (name == "nl") return lang_nl;
  else if (name == "en") return lang_en;
  else if (name == "fo") return lang_fo;
  else if (name == "de") return lang_de;
  else if (name == "nb") return lang_nb;
  else if (name == "no") return lang_no;
  else if (name == "nn") return lang_nn;
  else if (name == "sv") return lang_sv;
  else if (name == "et") return lang_et;
  else if (name == "fi") return lang_fi;
  else if (name == "el") return lang_el;
  else if (name == "he") return lang_he;
  else if (name == "it") return lang_it;
  else if (name == "pt") return lang_pt;
  else if (name == "es") return lang_es;
  else if (name == "eo") return lang_eo;
  else if (name == "fr") return lang_fr;
  else if (name == "pt_BR") return lang_pt_BR;
  else if (name == "lv") return lang_lv;
  else if (name == "ga") return lang_ga;
  else if (name == "lt") return lang_lt;
  else if (name == "hr") return lang_hr;
  else if (name == "cs") return lang_cs;
  else if (name == "ru") return lang_ru;
  else if (name == "uk") return lang_uk;
  else if (name == "sk") return lang_sk;
  else if (name == "pl") return lang_pl;
  else if (name == "sl") return lang_sl;
  else return lang_en; 
}

DictionaryManager::DictionaryManager()
  : current_dict(&empty_dict)
{
  parseLocaleAliases();
  // setup language from environment vars
  const char* lang = getenv("LC_ALL");
  if(!lang)
    lang = getenv("LC_MESSAGES");
  if(!lang)
    lang = getenv("LANG");
  
  if(lang)
    set_language(lang);
}

void
DictionaryManager::parseLocaleAliases()
{
  // try to parse language alias list
  std::ifstream in("/usr/share/locale/locale.alias");
  
  char c = ' ';
  while(in.good() && !in.eof()) {
    while(isspace(c) && !in.eof())
      in.get(c);
    
    if(c == '#') { // skip comments
      while(c != '\n' && !in.eof())
        in.get(c);
      continue;
    }
    
    std::string alias;
    while(!isspace(c) && !in.eof()) {
      alias += c;
      in.get(c);
    }
    while(isspace(c) && !in.eof())
      in.get(c);
    std::string language;
    while(!isspace(c) && !in.eof()) {
      language += c;
      in.get(c);
    }

    if(in.eof())
      break;
    set_language_alias(alias, language);
  }
}
  
Dictionary&
DictionaryManager::get_dictionary(const std::string& spec)
{
  std::string lang = get_language_from_spec(spec);
  Dictionaries::iterator i = dictionaries.find(get_language_from_spec(lang));
  if (i != dictionaries.end())
    {
      return i->second;
    }
  else // Dictionary for languages lang isn't loaded, so we load it
    {
      //std::cout << "get_dictionary: " << lang << std::endl;
      Dictionary& dict = dictionaries[lang];

      dict.set_language(get_language_def(lang));
      if(charset != "")
        dict.set_charset(charset);

      for (SearchPath::iterator p = search_path.begin(); p != search_path.end(); ++p)
        {
          DIR* dir = opendir(p->c_str());
          if (!dir)
            {
              std::cerr << "Error: opendir() failed on " << *p << std::endl;
            }
          else
            {
              struct dirent* ent;
              while((ent = readdir(dir)))
                {
                  if (std::string(ent->d_name) == lang + ".po")
                    {
                      std::string pofile = *p + "/" + ent->d_name;
                      std::ifstream in(pofile.c_str());
                      if (!in)
                        {
                          std::cerr << "Error: Failure file opening: " << pofile << std::endl;
                        }
                      else
                        {
                          read_po_file(dict, in);
                        }
                    }
                }
              closedir(dir);
            }
        }

      return dict;
    }
}

std::set<std::string>
DictionaryManager::get_languages()
{
  std::set<std::string> languages;

  for (SearchPath::iterator p = search_path.begin(); p != search_path.end(); ++p)
    {
      DIR* dir = opendir(p->c_str());
      if (!dir)
        {
          std::cerr << "Error: opendir() failed on " << *p << std::endl;
        }
      else
        {
          struct dirent* ent;
          while((ent = readdir(dir)))
            {
              if (has_suffix(ent->d_name, ".po"))
                {
                  std::string filename = ent->d_name;
                  languages.insert(filename.substr(0, filename.length()-3));
                }
            }
          closedir(dir);
        }
    }  
  return languages;
}

void
DictionaryManager::set_language(const std::string& lang)
{
  language = get_language_from_spec(lang);
  current_dict = & (get_dictionary(language));
}

void
DictionaryManager::set_charset(const std::string& charset)
{
  dictionaries.clear(); // changing charset invalidates cache
  this->charset = charset;
  set_language(language);
}

void
DictionaryManager::set_language_alias(const std::string& alias,
    const std::string& language)
{
  language_aliases.insert(std::make_pair(alias, language));
}

std::string
DictionaryManager::get_language_from_spec(const std::string& spec)
{
  std::string lang = spec;
  Aliases::iterator i = language_aliases.find(lang);
  if(i != language_aliases.end()) {
    lang = i->second;
  }
  
  std::string::size_type s = lang.find_first_of("_.");
  if(s == std::string::npos)
    return lang;

  return std::string(lang, 0, s);  
}

void
DictionaryManager::add_directory(const std::string& pathname)
{
  dictionaries.clear(); // adding directories invalidates cache
  search_path.push_back(pathname);
  set_language(language);
}

//---------------------------------------------------------------------------

Dictionary::Dictionary(const LanguageDef& language_, const std::string& charset_)
  : language(language_), charset(charset_)
{
}

Dictionary::Dictionary()
  : language(lang_en)
{
}

std::string
Dictionary::get_charset() const
{
  return charset;
}

void
Dictionary::set_charset(const std::string& charset_)
{
  charset = charset_;
}

void
Dictionary::set_language(const LanguageDef& lang)
{
  language = lang;
}

std::string
Dictionary::translate(const std::string& msgid, const std::string& msgid2, int num) 
{
  PluralEntries::iterator i = plural_entries.find(msgid);
  std::map<int, std::string>& msgstrs = i->second;

  if (i != plural_entries.end() && !msgstrs.empty())
    {
      int g = language.plural(num);
      std::map<int, std::string>::iterator j = msgstrs.find(g);
      if (j != msgstrs.end())
        {
          return j->second;
        }
      else
        {
          // Return the first translation, in case we can't translate the specific number
          return msgstrs.begin()->second;
        }
    }
  else
    {
#ifdef TRANSLATION_DEBUG
      std::cerr << "Warning: Couldn't translate: " << msgid << std::endl;
      std::cerr << "Candidates: " << std::endl;
      for (PluralEntries::iterator i = plural_entries.begin(); i != plural_entries.end(); ++i)
        std::cout << "'" << i->first << "'" << std::endl;
#endif

      if (plural2_1(num)) // default to english rules
        return msgid2;
      else
        return msgid;
    }
}

std::string
Dictionary::translate(const std::string& msgid) 
{
  Entries::iterator i = entries.find(msgid);
  if (i != entries.end() && !i->second.empty())
    {
      return i->second;
    }
  else
    {
#ifdef TRANSLATION_DBEUG
      std::cout << "Error: Couldn't translate: " << msgid << std::endl;
#endif
      return msgid;
    }
}
  
void
Dictionary::add_translation(const std::string& msgid, const std::string& ,
                            const std::map<int, std::string>& msgstrs)
{
  // Do we need msgid2 for anything? its after all supplied to the
  // translate call, so we just throw it away
  plural_entries[msgid] = msgstrs;
}

void 
Dictionary::add_translation(const std::string& msgid, const std::string& msgstr) 
{
  entries[msgid] = msgstr;
}

class POFileReader
{
private:
  struct Token
  {
    std::string keyword;
    std::string content;
  };

  Dictionary& dict;

  std::string from_charset;
  std::string to_charset;

  std::string current_msgid;
  std::string current_msgid_plural;
  std::map<int, std::string> msgstr_plural;

  int line_num;

  enum { WANT_MSGID, WANT_MSGSTR, WANT_MSGSTR_PLURAL, WANT_MSGID_PLURAL } state;

public:
  POFileReader(std::istream& in, Dictionary& dict_)
    : dict(dict_)
  {
    state = WANT_MSGID;
    line_num = 0;
    tokenize_po(in);
  }

  void parse_header(const std::string& header)
  {
    // Seperate the header in lines
    typedef std::vector<std::string> Lines;
    Lines lines;
    
    std::string::size_type start = 0;
    for(std::string::size_type i = 0; i < header.length(); ++i)
      {
        if (header[i] == '\n')
          {
            lines.push_back(header.substr(start, i - start));
            start = i+1;
          }
      }

    for(Lines::iterator i = lines.begin(); i != lines.end(); ++i)
      {
        if (has_prefix(*i, "Content-Type: text/plain; charset=")) {
          from_charset = i->substr(strlen("Content-Type: text/plain; charset="));
        }
      }

    if (from_charset.empty() || from_charset == "CHARSET")
      {
        std::cerr << "Error: Charset not specified for .po, fallback to ISO-8859-1" << std::endl;
        from_charset = "ISO-8859-1";
      }

    to_charset = dict.get_charset();
    if (to_charset.empty())
      { // No charset requested from the dict, so we use the one from the .po 
        to_charset = from_charset;
        dict.set_charset(from_charset);
      }
  }

  void add_token(const Token& token)
  {
    switch(state) 
      {
      case WANT_MSGID:
        if (token.keyword == "msgid") 
          {
            current_msgid = token.content;
            state = WANT_MSGID_PLURAL;
          }
        else if (token.keyword.empty())
          {
            //std::cerr << "Got EOF, everything looks ok." << std::endl;
          }
        else
          {
            std::cerr << "tinygettext: expected 'msgid' keyword, got " << token.keyword 
                      << " at line " << line_num << std::endl;
          }
        break;
    
      case WANT_MSGID_PLURAL:
        if (token.keyword == "msgid_plural") 
          {
            current_msgid_plural = token.content;
            state = WANT_MSGSTR_PLURAL;
          } 
        else
          {
            state = WANT_MSGSTR;
            add_token(token);
          }
        break;

      case WANT_MSGSTR:
        if (token.keyword == "msgstr") 
          {
            if (current_msgid == "") 
              { // .po Header is hidden in the msgid with the empty string
                parse_header(token.content);
              }
            else
              {
                dict.add_translation(current_msgid, convert(token.content, from_charset, to_charset));
              }
            state = WANT_MSGID;
          } 
        else
          {
            std::cerr << "tinygettext: expected 'msgstr' keyword, got " << token.keyword 
                      << " at line " << line_num << std::endl;
          }
        break;

      case WANT_MSGSTR_PLURAL:
        if (has_prefix(token.keyword, "msgstr[")) 
          {
            int num;
            if (sscanf(token.keyword.c_str(), "msgstr[%d]", &num) != 1) 
              {
                std::cerr << "Error: Couldn't parse: " << token.keyword << std::endl;
              } 
            else 
              {
                msgstr_plural[num] = convert(token.content, from_charset, to_charset);
              }
          }
        else 
          {
            dict.add_translation(current_msgid, current_msgid_plural, msgstr_plural);

            state = WANT_MSGID;
            add_token(token);
          }
        break;
      }
  }
  
  inline int getchar(std::istream& in) 
  {
    int c = in.get();
    if (c == '\n')
      line_num += 1;
    return c;
  }
  
  void tokenize_po(std::istream& in)
  {
    enum State { READ_KEYWORD, 
                 READ_CONTENT,
                 READ_CONTENT_IN_STRING,
                 SKIP_COMMENT };

    State state = READ_KEYWORD;
    int c;
    Token token;

    while((c = getchar(in)) != EOF)
      {
        //std::cout << "Lexing char: " << char(c) << " " << state << std::endl;
        switch(state)
          {
          case READ_KEYWORD:
            if (c == '#')
              {
                state = SKIP_COMMENT;
              }
            else
              {
                // Read a new token
                token = Token();
                
                do { // Read keyword 
                  token.keyword += c;
                } while((c = getchar(in)) != EOF && !isspace(c));
                in.unget();

                state = READ_CONTENT;
              }
            break;

          case READ_CONTENT:
            while((c = getchar(in)) != EOF)
              {
                if (c == '"') { 
                  // Found start of content
                  state = READ_CONTENT_IN_STRING;
                  break;
                } else if (isspace(c)) {
                  // skip
                } else { // Read something that may be a keyword
                  in.unget();
                  state = READ_KEYWORD;
                  add_token(token);
                  break;
                }
              }
            break;

          case READ_CONTENT_IN_STRING:
            if (c == '\\') {
              c = getchar(in);
              if (c != EOF)
                {
                  if (c == 'n') token.content += '\n';
                  else if (c == 't') token.content += '\t';
                  else if (c == 'r') token.content += '\r';
                  else if (c == '"') token.content += '"';
                  else
                    {
                      std::cout << "Unhandled escape character: " << char(c) << std::endl;
                    }
                }
              else
                {
                  std::cout << "Unterminated string" << std::endl;
                }
            } else if (c == '"') { // Content string is terminated
              state = READ_CONTENT;
            } else {
              token.content += c;
            }
            break;

          case SKIP_COMMENT:
            if (c == '\n')
              state = READ_KEYWORD;
            break;
          }
      }
    add_token(token);
  }
};

void read_po_file(Dictionary& dict_, std::istream& in) 
{
  POFileReader reader(in, dict_);
}

} // namespace TinyGetText

/* EOF */
