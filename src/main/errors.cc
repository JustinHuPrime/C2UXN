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

#include "errors.h"

#include <numeric>

using namespace std;
using namespace c2uxn::frontend;

namespace c2uxn {
ErrorReport::operator string() const noexcept {
  if (errors.empty()) {
    return "";
  } else if (errors.size() == 1) {
    return errors[0] + "\n"s;
  } else {
    return accumulate(errors.begin() + 1, errors.end(), errors[0],
                      [](string const &rsf, string const &toAdd) {
                        return rsf + "\n"s + toAdd;
                      }) +
           "\n"s;
  }
}
ErrorReport::operator bool() const noexcept { return !errors.empty(); }
void ErrorReport::add(string const &message) noexcept {
  return errors.push_back(message);
}

Error::Error(string const &content) noexcept : content(content) {}
Error::operator string() const noexcept { return content; }

[[noreturn]] void error(string const &filename, string const &message) {
  throw Error(filename + ": error: "s + message);
}
[[noreturn]] void error(SourceLocation const &location, string const &message) {
  throw Error(static_cast<string>(location) + ": error: "s + message);
}
}  // namespace c2uxn
