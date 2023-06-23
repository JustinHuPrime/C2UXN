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

#ifndef C2UXN_FRONTEND_TOKEN_H_
#define C2UXN_FRONTEND_TOKEN_H_

#include <string>
#include <variant>

#include "frontend/sourceLocation.h"

namespace c2uxn::frontend {
struct Token final {
  enum class Type {
    END_OF_FILE,
    KEYWORD,
    IDENTIFIER,
    INTEGER_CONSTANT,
    CHARACTER_CONSTANT,
    STRING_LITERAL,
    PUNCTUATION,
  };

  Token(SourceLocation const &, Type, std::string const &) noexcept;
  Token(SourceLocation const &, Type, std::string const &, int16_t) noexcept;
  Token(SourceLocation const &, Type, std::string const &, char) noexcept;
  Token(SourceLocation const &, Type, std::string const &,
        std::string const &) noexcept;
  Token(Token const &) noexcept = default;
  Token(Token &&) noexcept = default;

  ~Token() noexcept = default;

  Token &operator=(Token const &) noexcept = default;
  Token &operator=(Token &&) noexcept = default;

  SourceLocation location;
  Type type;
  std::string text;
  std::variant<int16_t, char, std::string> data;
};
}  // namespace c2uxn::frontend

#endif  // C2UXN_FRONTEND_TOKEN_H_
