# Copyright 2023 Justin Hu
#
# This file is part of PROJECT_NAME.
#
# PROJECT_NAME is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# PROJECT_NAME is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# PROJECT_NAME. If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later


# command options
CXX := g++
RM := rm -rf
CP := cp
MKDIR := mkdir -p
DOXYGEN := doxygen
TOUCH := touch
SED := sed
ECHO := echo
SET-E := set -e
CMAKE := cmake


# file options
SRCDIRPREFIX := src
OBJDIRPREFIX := bin
DEPDIRPREFIX := deps
MAINSUFFIX := main
TESTSUFFIX := test
DOCSDIR := docs

# main file options
SRCDIR := $(SRCDIRPREFIX)/$(MAINSUFFIX)
SRCS := $(shell find -O3 $(SRCDIR)/ -type f -name '*.cc')

OBJDIR := $(OBJDIRPREFIX)/$(MAINSUFFIX)
OBJS := $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.o,$(SRCS))

DEPDIR := $(DEPDIRPREFIX)/$(MAINSUFFIX)
DEPS := $(patsubst $(SRCDIR)/%.cc,$(DEPDIR)/%.dep,$(SRCS))

# test file options
TSRCDIR := $(SRCDIRPREFIX)/$(TESTSUFFIX)
TSRCS := $(shell find -O3 $(TSRCDIR)/ -type f -name '*.cc')

TOBJDIR := $(OBJDIRPREFIX)/$(TESTSUFFIX)
TOBJS := $(patsubst $(TSRCDIR)/%.cc,$(TOBJDIR)/%.o,$(TSRCS))

TDEPDIR := $(DEPDIRPREFIX)/$(TESTSUFFIX)
TDEPS := $(patsubst $(TSRCDIR)/%.cc,$(TDEPDIR)/%.dep,$(TSRCS))

# final executable name
EXENAME := PROJECT_NAME
TEXENAME := PROJECT_NAME-test


# compiler options
WARNINGS := -pedantic -pedantic-errors -Wall -Wextra -Wdouble-promotion\
-Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2\
-Wnull-dereference -Wimplicit-fallthrough=4 -Wmissing-include-dirs\
-Wswitch-enum -Wuninitialized -Wunknown-pragmas -Wstrict-overflow=5\
-Wstringop-overflow=4 -Wstringop-truncation -Wmissing-noreturn\
-Wmissing-format-attribute -Wsuggest-final-types -Wsuggest-final-methods\
-Wsuggest-override -Walloc-zero -Walloca -Warray-bounds=2\
-Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wshadow\
-Wplacement-new=2 -Wunused-macros -Wcast-qual -Wcast-align=strict\
-Wconditionally-supported -Wconversion -Wzero-as-null-pointer-constant\
-Wdate-time -Wuseless-cast -Wextra-semi -Wlogical-op -Wggregate-return\
-Wmissing-declarations -Wnormalized -Wopenmp-simd -Wpacked -Wpadded\
-Wredundant-decls -Winline -Winvalid-pch -Wvector-operation-performance\
-Wstack-protector -Whsa -Wabi-tag -Wctor-dtor-privacy -Wnoexcept\
-Wnon-virtual-dtor -Weffc++ -Wstrict-null-sentinel -Wold-style-cast\
-Woverloaded-virtual -Wsign-promo -Wunused -Wdisabled-optimization

OPTIONS := -std=c++20 -D_POSIX_C_SOURCE=202207L -I$(SRCDIR)\
#$(shell pkg-config --cflags )
TOPTIONS := -I$(TSRCDIR) -Ilibs/Catch2/src -Ilibs/Catch2/Build/generated-includes
LIBS := #$(shell pkg-config --libs )
TLIBS := libs/Catch2/Build/src/libCatch2Main.a libs/Catch2/Build/src/libCatch2.a

DEBUGOPTIONS := -Og -ggdb
RELEASEOPTIONS := -O3 -DNDEBUG


.PHONY: debug release docs install clean
.SECONDEXPANSION:
.SUFFIXES:


debug: OPTIONS := $(OPTIONS) $(DEBUGOPTIONS)
debug: $(EXENAME) $(TEXENAME) docs
	@$(ECHO) "Linting source"
	@libs/cpplint/cpplint.py --quiet --recursive src/main
	@$(ECHO) "Running tests"
	@./$(TEXENAME)
	@$(ECHO) "Done building debug!"

release: OPTIONS := $(OPTIONS) $(RELEASEOPTIONS)
release: $(EXENAME) $(TEXENAME)
	@$(ECHO) "Running tests"
	@./$(TEXENAME)
	@$(ECHO) "Done building release!"

docs: $(DOCSDIR)/.timestamp

clean:
	@$(ECHO) "Removing all generated files and folders."
	@$(RM) $(OBJDIRPREFIX) $(DEPDIRPREFIX) $(EXENAME) $(TEXENAME) $(DOCSDIR) libs/Catch2/Build

install:
	@$(ECHO) "Not yet implemented!"
	@exit 1


$(EXENAME): $(OBJS)
	@$(ECHO) "Linking $@"
	@$(CXX) -o $(EXENAME) $(OPTIONS) $(OBJS) $(LIBS)

$(OBJS): $$(patsubst $(OBJDIR)/%.o,$(SRCDIR)/%.cc,$$@) $$(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.dep,$$@) | $$(dir $$@)
	@$(ECHO) "Compiling $@"
	@$(CXX) -o $@ $(OPTIONS) -c $<

$(DEPS): $$(patsubst $(DEPDIR)/%.dep,$(SRCDIR)/%.cc,$$@) | $$(dir $$@)
	@$(SET-E); $(RM) $@; \
	 $(CXX) $(OPTIONS) -MM -MT $(patsubst $(DEPDIR)/%.dep,$(OBJDIR)/%.o,$@) $< > $@.$$$$; \
	 $(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 $(RM) $@.$$$$


$(TEXENAME): libs/Catch2/Build/src/libCatch2Main.a libs/Catch2/Build/src/libCatch2.a $(TOBJS) $(OBJS)
	@$(ECHO) "Linking $@"
	@$(CXX) -o $(TEXENAME) $(OPTIONS) $(TOPTIONS) $(filter-out %main.o,$(OBJS)) $(TOBJS) $(LIBS) $(TLIBS)

$(TOBJS): $$(patsubst $(TOBJDIR)/%.o,$(TSRCDIR)/%.cc,$$@) $$(patsubst $(TOBJDIR)/%.o,$(TDEPDIR)/%.dep,$$@) libs/Catch2/Build/generated-includes/catch2/catch_user_config.hpp | $$(dir $$@)
	@$(ECHO) "Compiling $@"
	@$(CXX) -o $@ $(OPTIONS) $(TOPTIONS) -c $<

$(TDEPS): $$(patsubst $(TDEPDIR)/%.dep,$(TSRCDIR)/%.cc,$$@) libs/Catch2/Build/generated-includes/catch2/catch_user_config.hpp | $$(dir $$@)
	@$(SET-E); $(RM) $@; \
	 $(CXX) $(OPTIONS) $(TOPTIONS) -MM -MT $(patsubst $(TDEPDIR)/%.dep,$(TOBJDIR)/%.o,$@) $< > $@.$$$$; \
	 $(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 $(RM) $@.$$$$

libs/Catch2/Build/src/libCatch2Main.a libs/Catch2/Build/src/libCatch2.a libs/Catch2/Build/generated-includes/catch2/catch_user_config.hpp &:
	@$(ECHO) "Building Catch2"
	@$(CMAKE) -S libs/Catch2 -B libs/Catch2/Build
	@$(MAKE) -C libs/Catch2/Build


$(DOCSDIR)/.timestamp: $(shell find -O3 $(SRCDIR) -type f \( -name '*.cc' -or -name '*.h' \) )
	@$(ECHO) "Generating documentation"
	@$(DOXYGEN)
	@$(TOUCH) $@


%/:
	@$(MKDIR) $@


-include $(DEPS) $(TDEPS)
