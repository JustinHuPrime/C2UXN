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

#ifndef C2UXN_ERRORS_H_
#define C2UXN_ERRORS_H_

#include <string>
#include <vector>

#include "frontend/sourceLocation.h"

namespace c2uxn {
class ErrorReport {
 public:
  ErrorReport() noexcept = default;
  ErrorReport(ErrorReport const &) noexcept = default;
  ErrorReport(ErrorReport &&) noexcept = default;

  ~ErrorReport() noexcept = default;

  ErrorReport &operator=(ErrorReport const &) noexcept = default;
  ErrorReport &operator=(ErrorReport &&) noexcept = default;

  operator std::string() const noexcept;
  operator bool() const noexcept;

  void add(std::string const &) noexcept;

 private:
  std::vector<std::string> errors;
};

class Error {
 public:
  explicit Error(std::string const &) noexcept;
  Error(Error const &) noexcept = default;
  Error(Error &&) noexcept = default;

  ~Error() noexcept = default;

  Error &operator=(Error const &) noexcept = default;
  Error &operator=(Error &&) noexcept = default;

  operator std::string() const noexcept;

 private:
  std::string content;
};

void error(std::string const &filename, std::string const &message);
void error(frontend::SourceLocation const &location,
           std::string const &message);
}  // namespace c2uxn

#endif  // C2UXN_ERRORS_ERRORS_H_
