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

#include <config.h>

#include <sstream>
#include <stdexcept>
#include <fstream>
#include <cassert>
#include <iostream>

#include "tinygettext/tinygettext.hpp"
#include "physfs/physfs_stream.hpp"
#include "parser.hpp"
#include "lisp.hpp"

namespace lisp
{

Parser::Parser(bool translate)
  : lexer(0), dictionary_manager(0), dictionary(0)
{
  if(translate) {
    dictionary_manager = new TinyGetText::DictionaryManager();
    dictionary_manager->set_charset("UTF-8");
  }
}

Parser::~Parser()
{
  delete lexer;
  delete dictionary_manager;
}

static std::string dirname(std::string filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if(p == std::string::npos)
    return "";

  return filename.substr(0, p+1);
}

Lisp*
Parser::parse(const std::string& filename)
{
  IFileStream in(filename);
  if(!in.good()) {
    std::stringstream msg;
    msg << "Parser problem: Couldn't open file '" << filename << "'.";
    throw std::runtime_error(msg.str());
  }

  if(dictionary_manager) {
    dictionary_manager->add_directory(dirname(filename));
    dictionary = & (dictionary_manager->get_dictionary());
  }
  
  return parse(in);
}

Lisp*
Parser::parse(std::istream& stream)
{
  delete lexer;
  lexer = new Lexer(stream);

  token = lexer->getNextToken();
  Lisp* result = new Lisp(Lisp::TYPE_CONS);
  result->v.cons.car = read();
  result->v.cons.cdr = 0;
  
  delete lexer;
  lexer = 0;

  return result;    
}

Lisp*
Parser::read()
{
  Lisp* result;
  switch(token) {
    case Lexer::TOKEN_EOF: {
      std::stringstream msg;
      msg << "Parse Error at line " << lexer->getLineNumber() << ": "
        << "Unexpected EOF.";
      throw std::runtime_error(msg.str());
    }
    case Lexer::TOKEN_CLOSE_PAREN: {
      std::stringstream msg;
      msg << "Parse Error at line " << lexer->getLineNumber() << ": "
        << "Unexpected ')'.";
      throw std::runtime_error(msg.str());
    }
    case Lexer::TOKEN_OPEN_PAREN: {
      result = new Lisp(Lisp::TYPE_CONS);
      
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
          throw std::runtime_error("Expected string after '(_'");
        
        result = new Lisp(Lisp::TYPE_STRING);
        if(dictionary) {
          std::string translation = dictionary->translate(lexer->getString());
          result->v.string = new char[translation.size()+1];
          memcpy(result->v.string, translation.c_str(), translation.size()+1);
        } else {
          size_t len = strlen(lexer->getString()) + 1;                                
          result->v.string = new char[len];
          memcpy(result->v.string, lexer->getString(), len);
        }
        token = lexer->getNextToken();
        if(token != Lexer::TOKEN_CLOSE_PAREN)
          throw std::runtime_error("Expected ')' after '(_ string'");
        break;
      }

      Lisp* cur = result;
      do {
        cur->v.cons.car = read();
        if(token == Lexer::TOKEN_CLOSE_PAREN) {
          cur->v.cons.cdr = 0;
          break;
        }
        cur->v.cons.cdr = new Lisp(Lisp::TYPE_CONS);
        cur = cur->v.cons.cdr;
      } while(1);

      break;
    }
    case Lexer::TOKEN_SYMBOL: {
      result = new Lisp(Lisp::TYPE_SYMBOL);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_STRING: {
      result = new Lisp(Lisp::TYPE_STRING);
      size_t len = strlen(lexer->getString()) + 1;
      result->v.string = new char[len];
      memcpy(result->v.string, lexer->getString(), len);
      break;
    }
    case Lexer::TOKEN_INTEGER:
      result = new Lisp(Lisp::TYPE_INTEGER);
      sscanf(lexer->getString(), "%d", &result->v.integer);
      break;
    case Lexer::TOKEN_REAL:
      result = new Lisp(Lisp::TYPE_REAL);
      sscanf(lexer->getString(), "%f", &result->v.real);
      break;
    case Lexer::TOKEN_TRUE:
      result = new Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = true;
      break;
    case Lexer::TOKEN_FALSE:
      result = new Lisp(Lisp::TYPE_BOOLEAN);
      result->v.boolean = false;
      break;

    default:
      // this should never happen
      assert(false);
  }

  token = lexer->getNextToken();
  return result;
}

} // end of namespace lisp
