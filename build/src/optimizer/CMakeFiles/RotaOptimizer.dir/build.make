# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kayms/SquadMaprotaGenerator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kayms/SquadMaprotaGenerator/build

# Include any dependencies generated for this target.
include src/optimizer/CMakeFiles/RotaOptimizer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/optimizer/CMakeFiles/RotaOptimizer.dir/compiler_depend.make

# Include the progress variables for this target.
include src/optimizer/CMakeFiles/RotaOptimizer.dir/progress.make

# Include the compile flags for this target's objects.
include src/optimizer/CMakeFiles/RotaOptimizer.dir/flags.make

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/flags.make
src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o: ../src/optimizer/OptimizerMain.cpp
src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kayms/SquadMaprotaGenerator/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o -MF CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o.d -o CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o -c /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerMain.cpp

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.i"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerMain.cpp > CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.i

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.s"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerMain.cpp -o CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.s

src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/flags.make
src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o: ../src/optimizer/RotaOptimizer.cpp
src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kayms/SquadMaprotaGenerator/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o -MF CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o.d -o CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o -c /home/kayms/SquadMaprotaGenerator/src/optimizer/RotaOptimizer.cpp

src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.i"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kayms/SquadMaprotaGenerator/src/optimizer/RotaOptimizer.cpp > CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.i

src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.s"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kayms/SquadMaprotaGenerator/src/optimizer/RotaOptimizer.cpp -o CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.s

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/flags.make
src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o: ../src/optimizer/OptimizerConfig.cpp
src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o: src/optimizer/CMakeFiles/RotaOptimizer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kayms/SquadMaprotaGenerator/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o -MF CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o.d -o CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o -c /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerConfig.cpp

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.i"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerConfig.cpp > CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.i

src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.s"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kayms/SquadMaprotaGenerator/src/optimizer/OptimizerConfig.cpp -o CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.s

# Object files for target RotaOptimizer
RotaOptimizer_OBJECTS = \
"CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o" \
"CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o" \
"CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o"

# External object files for target RotaOptimizer
RotaOptimizer_EXTERNAL_OBJECTS =

src/optimizer/RotaOptimizer: src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerMain.cpp.o
src/optimizer/RotaOptimizer: src/optimizer/CMakeFiles/RotaOptimizer.dir/RotaOptimizer.cpp.o
src/optimizer/RotaOptimizer: src/optimizer/CMakeFiles/RotaOptimizer.dir/OptimizerConfig.cpp.o
src/optimizer/RotaOptimizer: src/optimizer/CMakeFiles/RotaOptimizer.dir/build.make
src/optimizer/RotaOptimizer: src/optimizer/CMakeFiles/RotaOptimizer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kayms/SquadMaprotaGenerator/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable RotaOptimizer"
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RotaOptimizer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/optimizer/CMakeFiles/RotaOptimizer.dir/build: src/optimizer/RotaOptimizer
.PHONY : src/optimizer/CMakeFiles/RotaOptimizer.dir/build

src/optimizer/CMakeFiles/RotaOptimizer.dir/clean:
	cd /home/kayms/SquadMaprotaGenerator/build/src/optimizer && $(CMAKE_COMMAND) -P CMakeFiles/RotaOptimizer.dir/cmake_clean.cmake
.PHONY : src/optimizer/CMakeFiles/RotaOptimizer.dir/clean

src/optimizer/CMakeFiles/RotaOptimizer.dir/depend:
	cd /home/kayms/SquadMaprotaGenerator/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kayms/SquadMaprotaGenerator /home/kayms/SquadMaprotaGenerator/src/optimizer /home/kayms/SquadMaprotaGenerator/build /home/kayms/SquadMaprotaGenerator/build/src/optimizer /home/kayms/SquadMaprotaGenerator/build/src/optimizer/CMakeFiles/RotaOptimizer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/optimizer/CMakeFiles/RotaOptimizer.dir/depend

