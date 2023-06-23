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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

#include "errors.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/preprocessor.h"

using namespace std;
using namespace c2uxn;
using namespace c2uxn::frontend;

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  ErrorReport errorReport;

  try {
    string filename = string(argv[1]);
    ifstream fin = ifstream(filename);
    if (!fin) {
      error(filename, "could not open file");
    }
    string contents =
        string(istreambuf_iterator<char>(fin), istreambuf_iterator<char>());

    string preprocessed = preprocess(errorReport, filename, contents);
    if (errorReport) {
      cout << static_cast<string>(errorReport);
      return EXIT_FAILURE;
    }

    vector<Token> tokens = lex(errorReport, filename, preprocessed);
    if (errorReport) {
      cout << static_cast<string>(errorReport);
      return EXIT_FAILURE;
    }

    unique_ptr<ASTNode> ast = parse(errorReport, filename, tokens);
    if (errorReport) {
      cout << static_cast<string>(errorReport);
      return EXIT_FAILURE;
    }

  } catch (Error const &e) {
    errorReport.add(e);
  }

  if (errorReport) {
    cout << static_cast<string>(errorReport);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
