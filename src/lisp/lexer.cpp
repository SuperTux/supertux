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
#include <iostream>

#include "lexer.hpp"

namespace lisp
{

class EOFException
{
};

Lexer::Lexer(std::istream& newstream)
    : stream(newstream), eof(false), linenumber(0)
{
  try {
    // trigger a refill of the buffer
    c = 0;
    bufend = 0;
    nextChar();
  } catch(EOFException& ) {
  }
}

Lexer::~Lexer()
{
}

void
Lexer::nextChar()
{
  ++c;
  if(c >= bufend) {
    if(eof)
      throw EOFException();
    stream.read(buffer, BUFFER_SIZE);
    size_t bytes_read = stream.gcount();

    c = buffer;
    bufend = buffer + bytes_read;

    // the following is a hack that appends an additional ' ' at the end of
    // the file to avoid problems when parsing symbols/elements and a sudden
    // EOF. This is faster than relying on unget and IMO also nicer.
    if(bytes_read == 0 || stream.eof()) {
      eof = true;
      *bufend = ' ';
      ++bufend;
    }
  }
}

Lexer::TokenType
Lexer::getNextToken()
{
  static const char* delims = "\"();";

  try {
    while(isspace(*c)) {
      if(*c == '\n')
        ++linenumber;
      nextChar();
    };

    token_length = 0;

    switch(*c) {
      case ';': // comment
        while(true) {
          nextChar();
          if(*c == '\n') {
            ++linenumber;
            break;
          }
        }
        return getNextToken(); // and again
      case '(':
        nextChar();
        return TOKEN_OPEN_PAREN;
      case ')':
        nextChar();
        return TOKEN_CLOSE_PAREN;
      case '"': {  // string
        int startline = linenumber;
        try {
          while(1) {
            nextChar();
            if(*c == '"')
              break;
            else if (*c == '\r') // XXX this breaks with pure \r EOL
              continue;
            else if(*c == '\n')
              linenumber++;
            else if(*c == '\\') {
              nextChar();
              switch(*c) {
                case 'n':
                  *c = '\n';
                  break;
                case 't':
                  *c = '\t';
                  break;
              }
            }
            if(token_length < MAX_TOKEN_LENGTH)
              token_string[token_length++] = *c;
          }
          token_string[token_length] = 0;
        } catch(EOFException& ) {
          std::stringstream msg;
          msg << "Parse error in line " << startline << ": "
            << "EOF while parsing string.";
          throw std::runtime_error(msg.str());
        }
        nextChar();
        return TOKEN_STRING;
      }
      case '#': // constant
        try {
          nextChar();

          while(isalnum(*c) || *c == '_') {
            if(token_length < MAX_TOKEN_LENGTH)
              token_string[token_length++] = *c;
            nextChar();
          }
          token_string[token_length] = 0;
        } catch(EOFException& ) {
          std::stringstream msg;
          msg << "Parse Error in line " << linenumber << ": "
            << "EOF while parsing constant.";
          throw std::runtime_error(msg.str());
        }

        if(strcmp(token_string, "t") == 0)
          return TOKEN_TRUE;
        if(strcmp(token_string, "f") == 0)
          return TOKEN_FALSE;

        // we only handle #t and #f constants at the moment...

        {
          std::stringstream msg;
          msg << "Parse Error in line " << linenumber << ": "
            << "Unknown constant '" << token_string << "'.";
          throw std::runtime_error(msg.str());
        }

      default:
        if(isdigit(*c) || *c == '-') {
          bool have_nondigits = false;
          bool have_digits = false;
          int have_floating_point = 0;

          do {
            if(isdigit(*c))
              have_digits = true;
            else if(*c == '.')
              ++have_floating_point;
            else if(isalnum(*c) || *c == '_')
              have_nondigits = true;

            if(token_length < MAX_TOKEN_LENGTH)
              token_string[token_length++] = *c;

            nextChar();
          } while(!isspace(*c) && !strchr(delims, *c));

          token_string[token_length] = 0;

          // no nextChar

          if(have_nondigits || !have_digits || have_floating_point > 1)
            return TOKEN_SYMBOL;
          else if(have_floating_point == 1)
            return TOKEN_REAL;
          else
            return TOKEN_INTEGER;
        } else {
          do {
            if(token_length < MAX_TOKEN_LENGTH)
              token_string[token_length++] = *c;
            nextChar();
          } while(!isspace(*c) && !strchr(delims, *c));
          token_string[token_length] = 0;

          // no nextChar

          return TOKEN_SYMBOL;
        }
    }
  } catch(EOFException& ) {
    return TOKEN_EOF;
  }
}

} // end of namespace lisp
