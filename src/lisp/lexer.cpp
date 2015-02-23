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

#include "lisp/lexer.hpp"

#include <string.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>

namespace lisp {

Lexer::Lexer(std::istream& newstream) :
  stream(newstream),
  eof(false),
  linenumber(0),
  bufend(),
  bufpos(),
  c(),
  token_string(),
  token_length()
{
  // trigger a refill of the buffer
  bufpos = NULL;
  bufend = NULL;
  nextChar();
}

Lexer::~Lexer()
{
}

void
Lexer::nextChar()
{
  if(bufpos >= bufend || (bufpos == NULL && bufend == NULL) /* Initial refill trigger */) {
    if(eof) {
      c = EOF;
      return;
    }
    stream.read(buffer, BUFFER_SIZE);
    size_t bytes_read = stream.gcount();

    bufpos = buffer;
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

  if(bufpos == NULL)
    return;

  c = *bufpos++;
  if(c == '\n')
    ++linenumber;
}

void
Lexer::addChar()
{
  if(token_length < MAX_TOKEN_LENGTH)
    token_string[token_length++] = c;
  nextChar();
}

Lexer::TokenType
Lexer::getNextToken()
{
  static const char* delims = "\"();";

  while(isspace(c)) {
    nextChar();
  }

  token_length = 0;

  switch(c) {
    case ';': // comment
      while(c != '\n') {
        nextChar();
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
      while(1) {
        nextChar();
        switch(c) {
          case '"':
            nextChar();
            goto string_finished;
          case '\r':
            continue;
          case '\n':
            break;
          case '\\':
            nextChar();
            switch(c) {
              case 'n':
                c = '\n';
                break;
              case 't':
                c = '\t';
                break;
            }
            break;
          case EOF: {
            std::stringstream msg;
            msg << "Parse error in line " << startline << ": "
                << "EOF while parsing string.";
            throw std::runtime_error(msg.str());
          }
          default:
            break;
        }
        if(token_length < MAX_TOKEN_LENGTH)
          token_string[token_length++] = c;
      }
      string_finished:
      token_string[token_length] = 0;
      return TOKEN_STRING;
    }
    case '#': // constant
      nextChar();

      while(isalnum(c) || c == '_') {
        addChar();
      }
      token_string[token_length] = 0;

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

    case EOF:
      return TOKEN_EOF;

    default:
      if(isdigit(c) || c == '-') {
        bool have_nondigits = false;
        bool have_digits = false;
        int have_floating_point = 0;

        do {
          if(isdigit(c))
            have_digits = true;
          else if(c == '.')
            ++have_floating_point;
          else if(isalnum(c) || c == '_')
            have_nondigits = true;

          addChar();
        } while(!isspace(c) && !strchr(delims, c));

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
          addChar();
        } while(!isspace(c) && !strchr(delims, c));
        token_string[token_length] = 0;

        // no nextChar

        return TOKEN_SYMBOL;
      }
  }
}

} // end of namespace lisp

/* EOF */
