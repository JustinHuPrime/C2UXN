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

#include "frontend/token.h"

using namespace std;

namespace c2uxn::frontend {
Token::Token(SourceLocation const &location, Type type,
             string const &text) noexcept
    : location(location), type(type), text(text), data() {}
Token::Token(SourceLocation const &location, Type type, string const &text,
             int16_t value) noexcept
    : location(location), type(type), text(text), data(value) {}
Token::Token(SourceLocation const &location, Type type, string const &text,
             char value) noexcept
    : location(location), type(type), text(text), data(value) {}
Token::Token(SourceLocation const &location, Type type, string const &text,
             std::string const &value) noexcept
    : location(location), type(type), text(text), data(value) {}
}  // namespace c2uxn::frontend
