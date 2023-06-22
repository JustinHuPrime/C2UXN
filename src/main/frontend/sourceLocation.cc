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

#include "frontend/sourceLocation.h"

using namespace std;

namespace c2uxn::frontend {
SourceLocation::SourceLocation(string const &file, size_t startLine,
                               size_t startChar, size_t endLine,
                               size_t endChar) noexcept
    : file(file),
      startLine(startLine),
      startChar(startChar),
      endLine(endLine),
      endChar(endChar) {}

SourceLocation::operator string() const noexcept {
  return file + ":"s + to_string(startLine) + ":"s + to_string(startChar);
}
}  // namespace c2uxn::frontend
