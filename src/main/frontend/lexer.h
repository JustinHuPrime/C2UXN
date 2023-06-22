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

#ifndef C2UXN_FRONTEND_LEXER_H_
#define C2UXN_FRONTEND_LEXER_H_

#include <string>
#include <vector>

#include "frontend/token.h"

namespace c2uxn::frontend {
std::vector<Token> lex(std::string const &text);
}

#endif  // C2UXN_FRONTEND_LEXER_H_
