# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/cmake-3.9.2/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake-3.9.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/travis/build/Alkud/OTUS_HOMEWORK_13

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/travis/build/Alkud/OTUS_HOMEWORK_13

# Include any dependencies generated for this target.
include async_server/CMakeFiles/async_server.dir/depend.make

# Include the progress variables for this target.
include async_server/CMakeFiles/async_server.dir/progress.make

# Include the compile flags for this target's objects.
include async_server/CMakeFiles/async_server.dir/flags.make

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o: async_server/CMakeFiles/async_server.dir/flags.make
async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o: async_server/async_acceptor.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_13/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/async_server.dir/async_acceptor.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_acceptor.cpp

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/async_server.dir/async_acceptor.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_acceptor.cpp > CMakeFiles/async_server.dir/async_acceptor.cpp.i

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/async_server.dir/async_acceptor.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_acceptor.cpp -o CMakeFiles/async_server.dir/async_acceptor.cpp.s

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.requires:

.PHONY : async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.requires

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.provides: async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.requires
	$(MAKE) -f async_server/CMakeFiles/async_server.dir/build.make async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.provides.build
.PHONY : async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.provides

async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.provides.build: async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o


async_server/CMakeFiles/async_server.dir/async_reader.cpp.o: async_server/CMakeFiles/async_server.dir/flags.make
async_server/CMakeFiles/async_server.dir/async_reader.cpp.o: async_server/async_reader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_13/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object async_server/CMakeFiles/async_server.dir/async_reader.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/async_server.dir/async_reader.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_reader.cpp

async_server/CMakeFiles/async_server.dir/async_reader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/async_server.dir/async_reader.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_reader.cpp > CMakeFiles/async_server.dir/async_reader.cpp.i

async_server/CMakeFiles/async_server.dir/async_reader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/async_server.dir/async_reader.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/async_reader.cpp -o CMakeFiles/async_server.dir/async_reader.cpp.s

async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.requires:

.PHONY : async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.requires

async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.provides: async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.requires
	$(MAKE) -f async_server/CMakeFiles/async_server.dir/build.make async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.provides.build
.PHONY : async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.provides

async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.provides.build: async_server/CMakeFiles/async_server.dir/async_reader.cpp.o


async_server/CMakeFiles/async_server.dir/db_manager.cpp.o: async_server/CMakeFiles/async_server.dir/flags.make
async_server/CMakeFiles/async_server.dir/db_manager.cpp.o: async_server/db_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_13/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object async_server/CMakeFiles/async_server.dir/db_manager.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/async_server.dir/db_manager.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_manager.cpp

async_server/CMakeFiles/async_server.dir/db_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/async_server.dir/db_manager.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_manager.cpp > CMakeFiles/async_server.dir/db_manager.cpp.i

async_server/CMakeFiles/async_server.dir/db_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/async_server.dir/db_manager.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_manager.cpp -o CMakeFiles/async_server.dir/db_manager.cpp.s

async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.requires:

.PHONY : async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.requires

async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.provides: async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.requires
	$(MAKE) -f async_server/CMakeFiles/async_server.dir/build.make async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.provides.build
.PHONY : async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.provides

async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.provides.build: async_server/CMakeFiles/async_server.dir/db_manager.cpp.o


async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o: async_server/CMakeFiles/async_server.dir/flags.make
async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o: async_server/db_command_translator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_13/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/async_server.dir/db_command_translator.cpp.o -c /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_command_translator.cpp

async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/async_server.dir/db_command_translator.cpp.i"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_command_translator.cpp > CMakeFiles/async_server.dir/db_command_translator.cpp.i

async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/async_server.dir/db_command_translator.cpp.s"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/db_command_translator.cpp -o CMakeFiles/async_server.dir/db_command_translator.cpp.s

async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.requires:

.PHONY : async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.requires

async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.provides: async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.requires
	$(MAKE) -f async_server/CMakeFiles/async_server.dir/build.make async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.provides.build
.PHONY : async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.provides

async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.provides.build: async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o


# Object files for target async_server
async_server_OBJECTS = \
"CMakeFiles/async_server.dir/async_acceptor.cpp.o" \
"CMakeFiles/async_server.dir/async_reader.cpp.o" \
"CMakeFiles/async_server.dir/db_manager.cpp.o" \
"CMakeFiles/async_server.dir/db_command_translator.cpp.o"

# External object files for target async_server
async_server_EXTERNAL_OBJECTS =

async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o
async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/async_reader.cpp.o
async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/db_manager.cpp.o
async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o
async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/build.make
async_server/libasync_server.a: async_server/CMakeFiles/async_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/travis/build/Alkud/OTUS_HOMEWORK_13/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libasync_server.a"
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && $(CMAKE_COMMAND) -P CMakeFiles/async_server.dir/cmake_clean_target.cmake
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/async_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
async_server/CMakeFiles/async_server.dir/build: async_server/libasync_server.a

.PHONY : async_server/CMakeFiles/async_server.dir/build

async_server/CMakeFiles/async_server.dir/requires: async_server/CMakeFiles/async_server.dir/async_acceptor.cpp.o.requires
async_server/CMakeFiles/async_server.dir/requires: async_server/CMakeFiles/async_server.dir/async_reader.cpp.o.requires
async_server/CMakeFiles/async_server.dir/requires: async_server/CMakeFiles/async_server.dir/db_manager.cpp.o.requires
async_server/CMakeFiles/async_server.dir/requires: async_server/CMakeFiles/async_server.dir/db_command_translator.cpp.o.requires

.PHONY : async_server/CMakeFiles/async_server.dir/requires

async_server/CMakeFiles/async_server.dir/clean:
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server && $(CMAKE_COMMAND) -P CMakeFiles/async_server.dir/cmake_clean.cmake
.PHONY : async_server/CMakeFiles/async_server.dir/clean

async_server/CMakeFiles/async_server.dir/depend:
	cd /home/travis/build/Alkud/OTUS_HOMEWORK_13 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/travis/build/Alkud/OTUS_HOMEWORK_13 /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server /home/travis/build/Alkud/OTUS_HOMEWORK_13 /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server /home/travis/build/Alkud/OTUS_HOMEWORK_13/async_server/CMakeFiles/async_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : async_server/CMakeFiles/async_server.dir/depend

