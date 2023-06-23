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

#include "frontend/preprocessor.h"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <stdexcept>

using namespace std;

namespace c2uxn::frontend {
namespace {
constexpr size_t READ_END = 0;
constexpr size_t WRITE_END = 1;
constexpr size_t BUFFER_SIZE = 4096;

char charToNybble(char c) {
  if (0 <= c <= 9) {
    return c + '0';
  } else {
    return c + 'a';
  }
}

string escapeString(string const &s) {
  string result;
  for_each(s.begin(), s.end(), [&result](char c) {
    if (' ' <= c <= '~') {
      result += c;
    } else {
      result += "\\";
      result += charToNybble((c & 0xf0) >> 4);
      result += charToNybble((c & 0x0f) >> 0);
    }
  });
  return result;
}
}  // namespace

string preprocess(ErrorReport &errorReport, string const &filename,
                  string const &text) {
  try {
    int toPreprocessorPipe[2];
    int fromPreprocessorPipe[2];

    if (pipe2(toPreprocessorPipe, O_CLOEXEC)) {
      throw runtime_error("could not create pipe");
    }
    if (pipe2(fromPreprocessorPipe, O_CLOEXEC)) {
      throw runtime_error("could not create pipe");
    }

    if (pid_t retval = fork(); retval == -1) {
      throw runtime_error("could not run preprocessor");
    } else if (retval == 0) {
      // I'm the child
      close(toPreprocessorPipe[WRITE_END]);  // won't be writing to myself
      dup2(toPreprocessorPipe[READ_END],
           STDIN_FILENO);  // make stdin the read end the pipe

      close(fromPreprocessorPipe[READ_END]);  // won't be reading from myself
      dup2(fromPreprocessorPipe[WRITE_END],
           STDOUT_FILENO);  // make stdout the write end of the pipe

      execlp("cpp", "cpp", "-std=c17", "-ffreestanding", "-undef", nullptr);
      raise(SIGUSR1);
      // can't get here (would be terminated by SIGUSR1)
      abort();
    } else {
      // I'm the parent
      close(toPreprocessorPipe[READ_END]);  // won't be reading from myself
      unique_ptr<FILE, decltype(&fclose)> toPreprocessor =
          unique_ptr<FILE, decltype(&fclose)>(
              fdopen(toPreprocessorPipe[WRITE_END], "w"), fclose);

      close(fromPreprocessorPipe[WRITE_END]);  // won't be writing to myself
      unique_ptr<FILE, decltype(&fclose)> fromPreprocessor =
          unique_ptr<FILE, decltype(&fclose)>(
              fdopen(fromPreprocessorPipe[READ_END], "r"), fclose);

      // write to child
      // override filename
      string escaped = escapeString(filename);
      if (int written = fprintf(toPreprocessor.get(), "#line 1 \"%s\"\n",
                                escaped.c_str());
          written != escaped.size() + 11) {
        throw runtime_error("could not set filename"s);
      }
      if (fwrite(text.data(), sizeof(char), text.size(),
                 toPreprocessor.get()) != text.size()) {
        throw runtime_error("could not send file to preprocessor");
      }
      toPreprocessor.reset();

      // read output from child
      unique_ptr<char[]> buffer = make_unique<char[]>(BUFFER_SIZE);
      string output = "";
      while (feof(fromPreprocessor.get()) == 0) {
        size_t readCount = fread(buffer.get(), sizeof(char), BUFFER_SIZE,
                                 fromPreprocessor.get());
        copy(buffer.get(), buffer.get() + readCount, back_inserter(output));
      }

      // did it finish successfully?
      int exitStatus;
      waitpid(retval, &exitStatus, 0);

      if (WIFSIGNALED(exitStatus) && WTERMSIG(exitStatus) == SIGUSR1) {
        throw runtime_error("could not run preprocessor");
      }

      if (WEXITSTATUS(exitStatus) != 0) {
        error(filename, "preprocessor error");
      }

      return output;
    }
  } catch (Error const &e) {
    errorReport.add(e);
    return "";
  }
}
}  // namespace c2uxn::frontend
