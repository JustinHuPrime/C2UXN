// Copyright 2023 Justin Hu
//
// This file is part of C2UXN.
//
// C2UXN is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// C2UXN is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// C2UXN. If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frontend/lexer.h"

#include <algorithm>
#include <cinttypes>
#include <limits>
#include <regex>
#include <unordered_set>

using namespace std;

namespace c2uxn::frontend {
namespace {
regex const SOURCE_INFO = regex(
    R"STR(# ([0-9]+) "((?:[^"\n\\]|\\['"?\\abfnrtv]|\\[0-8]{1,3}|\\x[0-9a-fA-F]+)+)"(?: 1)?(?: 2)?(?: 3)?(?: 4)?\n)STR",
    regex_constants::ECMAScript | regex_constants::optimize);
regex const PREPROCESSING_DIRECTIVE =
    regex(R"(#.*\n)", regex_constants::ECMAScript | regex_constants::optimize);
regex const WHITESPACE = regex(
    R"([\n\t ]+)", regex_constants::ECMAScript | regex_constants::optimize);
regex const ID_OR_KEYWORD =
    regex(R"([a-zA-Z_][a-zA-Z0-9_]*)",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const DECIMAL_CONSTANT =
    regex(R"(([1-9][0-9]*)([uU][lL]{0,2}|[lL]{0,2}[uU]?))",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const OCTAL_CONSTANT =
    regex(R"((0[0-7]*)([uU][lL]{0,2}|[lL]{0,2}[uU]?))",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const HEXADECIMAL_CONSTANT =
    regex(R"((0[xX][0-9a-fA-F]+)([uU][lL]{0,2}|[lL]{0,2}[uU]?))",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const FLOATING_CONSTANT = regex(
    R"((?:(?:[0-9]*\.[0-9]+|[0-9]+\.)(?:[eE][+-]?[0-9]+)?|0[xX](?:[0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.)(?:[pP][+-]?[0-9]+)?)[flFL]?)",
    regex_constants::ECMAScript | regex_constants::optimize);
regex const CHARACTER_CONSTANT = regex(
    R"([LuU]?'((?:[^'\n\\]|\\['"?\\abfnrtv]|\\[0-8]{1,3}|\\x[0-9a-fA-F]+)+)')",
    regex_constants::ECMAScript | regex_constants::optimize);
regex const SINGLE_ESCAPED_CHAR_CHARACTER =
    regex(R"([^'\n\\]|\\['"?\\abfnrtv]|\\[0-8]{1,3}|\\x[0-9a-fA-F]+)",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const STRING_CONSTANT = regex(
    R"STR((?:[LuU]|u8)?"((?:[^"\n\\]|\\['"?\\abfnrtv]|\\[0-8]{1,3}|\\x[0-9a-fA-F]+)*)")STR",
    regex_constants::ECMAScript | regex_constants::optimize);
regex const SINGLE_ESCAPED_STRING_CHARACTER =
    regex(R"([^"\n\\]|\\['"?\\abfnrtv]|\\[0-8]{1,3}|\\x[0-9a-fA-F]+)",
          regex_constants::ECMAScript | regex_constants::optimize);
regex const PUNCTUATION = regex(
    R"(%:%:|>>=|<<=|\.\.\.|\|\||\|=|\^=|>>|>=|==|<=|<<|<:|<%|:>|\/=|->|-=|--|\+=|\+\+|\*=|&=|&&|%>|%=|%:|!=|[~}|{^\][?>=<;:\/.\-,+*)(&%!])",
    regex_constants::ECMAScript | regex_constants::optimize);

unordered_set<string> const keywords = {
    "auto",       "break",     "case",           "char",
    "const",      "continue",  "default",        "do",
    "double",     "else",      "enum",           "extern",
    "float",      "for",       "goto",           "if",
    "inline",     "int",       "long",           "register",
    "restrict",   "return",    "short",          "signed",
    "sizeof",     "static",    "struct",         "switch",
    "typedef",    "union",     "unsigned",       "void",
    "volatile",   "while",     "_Alignas",       "_Alignof",
    "_Atomic",    "_Bool",     "_Complex",       "_Generic",
    "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local",
};

void handleConstant(smatch const &results, string const &currFile,
                    size_t const &line, size_t &character,
                    string::const_iterator &curr, vector<Token> &tokens,
                    int base) {
  string content = results[0].str();

  if (find(results[2].first, results[2].second, 'u') != results[2].second ||
      find(results[2].first, results[2].second, 'U') != results[2].second) {
    curr = results[0].second;
    character += content.size();
    error(SourceLocation(currFile, line, character - content.size(), line,
                         character),
          "unsupported unsigned integer encountered"s);
  }

  char *endPtr;
  uintmax_t value = strtoumax(&*results[1].first, &endPtr, base);
  if (endPtr != &*results[1].second || value > numeric_limits<int16_t>::max()) {
    curr = results[0].second;
    character += content.size();
    error(SourceLocation(currFile, line, character - content.size(), line,
                         character),
          "integer literal overflows"s);
  }

  tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                     character + content.size()),
                      Token::Type::INTEGER_CONSTANT, content,
                      static_cast<int16_t>(value));
  curr = results[0].second;
  character += content.size();
}

char nybbleToChar(char hex) {
  if ('a' <= hex && hex <= 'F') {
    return hex - 'a' + 10;
  } else if ('A' <= hex && hex <= 'F') {
    return hex - 'A' + 10;
  } else {
    return hex - '0';
  }
}

char unescapeChar(string const &escaped, smatch const &results,
                  string const &content, string const &currFile,
                  size_t const &line, size_t &character,
                  string::const_iterator &curr) {
  if (escaped[0] != '\\') {
    return escaped[0];
  } else {
    switch (escaped[1]) {
      case '\'': {
        return '\'';
      }
      case '"': {
        return '"';
      }
      case '?': {
        return '?';
      }
      case '\\': {
        return '\\';
      }
      case 'a': {
        return '\a';
      }
      case 'b': {
        return '\b';
      }
      case 'f': {
        return '\f';
      }
      case 'n': {
        return '\n';
      }
      case 'r': {
        return '\r';
      }
      case 't': {
        return '\t';
      }
      case 'v': {
        return '\v';
      }
      case 'x': {
        if (escaped.size() > 4) {
          curr = results[0].second;
          character += content.size();
          error(SourceLocation(currFile, line, character - content.size(), line,
                               character),
                "multibyte character encountered"s);
        } else if (escaped.size() == 3) {
          return nybbleToChar(escaped[2]);
        } else {
          return (nybbleToChar(escaped[2]) << 4) |
                 (nybbleToChar(escaped[3]) << 0);
        }
        break;
      }
      default: {
        // octal
        if (escaped.size() == 2) {
          return escaped[1] - '0';
        } else if (escaped.size() == 3) {
          return ((escaped[1] - '0') << 3) | ((escaped[2] - '0') << 0);
        } else {
          if (escaped[1] > '1') {
            curr = results[0].second;
            character += content.size();
            error(SourceLocation(currFile, line, character - content.size(),
                                 line, character),
                  "multibyte character encountered"s);
          }

          return ((escaped[1] - '0') << 6) | ((escaped[2] - '0') << 3) |
                 ((escaped[3] - '0') << 0);
        }
        break;
      }
    }
  }
}

string unescapeString(smatch const &results, size_t idx, string const &content,
                      string const &currFile, size_t const &line,
                      size_t &character, string::const_iterator &curr) {
  string value;
  smatch characterResults;
  string::const_iterator currCharacter = results[idx].first;
  while (regex_search(currCharacter, results[idx].second, characterResults,
                      SINGLE_ESCAPED_STRING_CHARACTER)) {
    string escaped = characterResults[0].str();
    char unescaped = unescapeChar(escaped, results, content, currFile, line,
                                  character, curr);
    currCharacter = characterResults[0].second;
    value += unescaped;
  }

  return value;
}
}  // namespace

vector<Token> lex(ErrorReport &errorReport, string const &filename,
                  string const &text) {
  vector<Token> tokens;

  string currFile = "<unknown>";
  size_t line = 1;
  size_t character = 1;
  string::const_iterator curr = text.begin();
  while (true) {
    try {
      // discard whitespace
      smatch results;
      if (regex_search(curr, text.end(), results, WHITESPACE,
                       regex_constants::match_continuous)) {
        for_each(results[0].first, results[0].second, [&](char c) {
          switch (c) {
            case '\n': {
              ++line;
              character = 1;
              break;
            }
            case '\t':
            case ' ': {
              ++character;
              break;
            }
          }
        });
        curr = results[0].second;
      }

      if (curr == text.end()) {
        tokens.emplace_back(
            SourceLocation(currFile, line, character, line, character),
            Token::Type::END_OF_FILE, "");
        break;
      } else if (character == 1 &&
                 regex_search(curr, text.end(), results, SOURCE_INFO,
                              regex_constants::match_continuous)) {
        try {
          string content = results[0].str();

          curr = results[0].second;
          currFile = unescapeString(results, 2, content, currFile, line,
                                    character, curr);
          if (currFile == "<stdin>") {
            currFile = filename;
          }
          line = strtoumax(&*results[1].first, nullptr, 10);
          character = 1;
        } catch (Error const &e) {
          throw runtime_error("preprocessed output contained bad string");
        }
      } else if (character == 1 &&
                 regex_search(curr, text.end(), results,
                              PREPROCESSING_DIRECTIVE,
                              regex_constants::match_continuous)) {
        curr = results[0].second;
        ++line;
        error(SourceLocation(currFile, line - 1, 1, line, 1),
              "unsupported preprocessing directive");
      } else if (regex_search(curr, text.end(), results, ID_OR_KEYWORD,
                              regex_constants::match_continuous)) {
        string content = results[0].str();
        if (keywords.find(content) != keywords.end()) {
          tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                             character + content.size()),
                              Token::Type::KEYWORD, content);
        } else {
          tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                             character + content.size()),
                              Token::Type::IDENTIFIER, content);
        }
        curr = results[0].second;
        character += content.size();
      } else if (regex_search(curr, text.end(), results, FLOATING_CONSTANT,
                              regex_constants::match_continuous)) {
        string content = results[0].str();

        curr = results[0].second;
        character += content.size();
        error(SourceLocation(currFile, line, character - content.size(), line,
                             character),
              "unsupported floating point number encountered"s);
      } else if (regex_search(curr, text.end(), results, DECIMAL_CONSTANT,
                              regex_constants::match_continuous)) {
        handleConstant(results, currFile, line, character, curr, tokens, 10);
      } else if (regex_search(curr, text.end(), results, OCTAL_CONSTANT,
                              regex_constants::match_continuous)) {
        handleConstant(results, currFile, line, character, curr, tokens, 8);
      } else if (regex_search(curr, text.end(), results, HEXADECIMAL_CONSTANT,
                              regex_constants::match_continuous)) {
        handleConstant(results, currFile, line, character, curr, tokens, 16);
      } else if (regex_search(curr, text.end(), results, CHARACTER_CONSTANT,
                              regex_constants::match_continuous)) {
        string content = results[0].str();

        smatch characterResults;
        regex_search(results[1].first, results[1].second, characterResults,
                     SINGLE_ESCAPED_CHAR_CHARACTER);
        if (characterResults.suffix().length() != 0) {
          curr = results[0].second;
          character += content.size();
          error(SourceLocation(currFile, line, character - content.size(), line,
                               character),
                "multibyte character constant encountered"s);
        }

        string escaped = characterResults[0].str();
        char value = unescapeChar(escaped, results, content, currFile, line,
                                  character, curr);
        tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                           character + content.size()),
                            Token::Type::CHARACTER_CONSTANT, content, value);
        curr = results[0].second;
        character += content.size();
      } else if (regex_search(curr, text.end(), results, STRING_CONSTANT,
                              regex_constants::match_continuous)) {
        string content = results[0].str();

        string value = unescapeString(results, 1, content, currFile, line,
                                      character, curr);
        tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                           character + content.size()),
                            Token::Type::STRING_LITERAL, content, value);
        curr = results[0].second;
        character += content.size();
      } else if (regex_search(curr, text.end(), results, PUNCTUATION,
                              regex_constants::match_continuous)) {
        string content = results[0].str();
        tokens.emplace_back(SourceLocation(currFile, line, character, line,
                                           character + content.size()),
                            Token::Type::PUNCTUATION, content);
        curr = results[0].second;
        character += content.size();
      } else {
        // unexpected character
        curr += 1;
        character += 1;
        error(SourceLocation(currFile, line, character - 1, line, character),
              "unexpected character '"s + *curr + "' encountered"s);
      }
    } catch (Error const &e) {
      errorReport.add(e);
    }
  }

  return tokens;
}
}  // namespace c2uxn::frontend
