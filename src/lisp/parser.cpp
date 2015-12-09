//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include <sstream>
#include <stdexcept>
#include <tinygettext/tinygettext.hpp>
#include <physfs.h>

#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "util/gettext.hpp"
#include "util/obstackpp.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/globals.hpp"

#include "supertux/gameconfig.hpp"

namespace lisp {

Parser::Parser(bool translate_) :
  lexer(0),
  filename(),
  token(),
  obst(),
  translate(translate_)
{
  obstack_init(&obst);
}

Parser::~Parser()
{
  obstack_free(&obst, NULL);
  delete lexer;
}

static std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)
    return "";

  return filename.substr(0, p);
}

const Lisp*
Parser::parse(const std::string& filename_)
{
  IFileStreambuf ins(filename_);
  std::istream in(&ins);

  if(!in.good()) {
    std::stringstream msg;
    msg << "Parser problem: Couldn't open file '" << filename_ << "'.";
    throw std::runtime_error(msg.str());
  }

  if(translate && g_dictionary_manager) {
    std::string rel_dir = dirname (filename_);
    if(rel_dir.empty())
    {
      // Relative dir inside PhysFS search path?
      // Get full path from search path, instead.
      rel_dir = PHYSFS_getRealDir(filename_.c_str());
    }
    g_dictionary_manager->add_directory (rel_dir);
  }

  return parse(in, filename_);
}

const Lisp*
Parser::parse(std::istream& stream, const std::string& sourcename)
{
  delete lexer;
  lexer = new Lexer(stream);

  this->filename = sourcename;
  token = lexer->getNextToken();

  Lisp* result = new(obst) Lisp(Lisp::TYPE_CONS);
  result->v.cons.car = read();
  result->v.cons.cdr = 0;

  delete lexer;
  lexer = 0;

  return result;
}

void
Parser::parse_error(const char* msg) const
{
  std::stringstream emsg;
  emsg << "Parse Error at '" << filename << "' line " << lexer->getLineNumber()
       << ": " << msg;
  throw std::runtime_error(emsg.str());
}

const Lisp*
Parser::read()
{
  Lisp* result;
  switch(token) {
    case Lexer::TOKEN_EOF: {
      parse_error("Unexpected EOF.");
    }
    case Lexer::TOKEN_CLOSE_PAREN: {
      parse_error("Unexpected ')'.");
    }
    case Lexer::TOKEN_OPEN_PAREN: {
      result = new(obst) Lisp(Lisp::TYPE_CONS);

      token = lexer->getNextToken();
      if(token == Lexer::TOKEN_CLOSE_PAREN) {
        result->v.cons.car = 0;
        result->v.cons.cdr = 0;
        break;
      }

      if(token == Lexer::TOKEN_SYMBOL &&
         strcmp(lexer->getString(), "_") == 0) {
        // evaluate translation function (_ str) in place here
        token = lexer->getNextToken();
        if(token != Lexer::TOKEN_STRING)
          parse_error("Expected string after '(_'");

        result = new(obst) Lisp(Lisp::TYPE_STRING);
        if(translate) {
          std::string translation = _(lexer->getString());
          result->v.string = new(obst) char[translation.size()+1];
          memcpy(result->v.string, translation.c_str(), translation.size()+1);
        } else {
          size_t len = strlen(lexer->getString()) + 1;
          result->v.string = new(obst) char[len];
          memcpy(result->v.string, lexer->getString(), len);
        }
        token = lexer->getNextToken();
        if(token != Lexer::TOKEN_CLOSE_PAREN)
          parse_error("Expected ')' after '(_ string'");
        break;
      }

      Lisp* cur = result;
      do {
        cur->v.cons.car = read();
        if(token == Lexer::TOKEN_CLOSE_PAREN) {
          cur->v.cons.cdr = 0;
          break;
        }
        Lisp *newcur = new(obst) Lisp(Lisp::TYPE_CONS);
        cur->v.cons.cdr = newcur;
        cur = newcur;
      } while(1);

      break;
    }
    case Lexer::TOKEN_SYMBOL: {
      result = new(obst) Lisp(Lisp::TYPE_SYMBOL);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new(obst) char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_STRING: {
      result = new(obst) Lisp(Lisp::TYPE_STRING);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new(obst) char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_INTEGER:
      result = new(obst) Lisp(Lisp::TYPE_INTEGER);
      result->v.integer = atoi(lexer->getString());
      break;
    case Lexer::TOKEN_REAL:
      result = new(obst) Lisp(Lisp::TYPE_REAL);
      result->v.real = strtof(lexer->getString(), NULL);
      break;
    case Lexer::TOKEN_TRUE:
      result = new(obst) Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = true;
      break;
    case Lexer::TOKEN_FALSE:
      result = new(obst) Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = false;
      break;

    default:
      // this should never happen
      result = NULL;
      assert(false);
  }

  token = lexer->getNextToken();
  return result;
}

} // end of namespace lisp

/* EOF */
