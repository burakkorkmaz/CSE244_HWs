# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /home/eksor/Programlar/clion-2017.1/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/eksor/Programlar/clion-2017.1/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/eksor/GitHub/CSE244_HWs/Midterm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/seeWhat.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/seeWhat.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/seeWhat.dir/flags.make

CMakeFiles/seeWhat.dir/seeWhat.c.o: CMakeFiles/seeWhat.dir/flags.make
CMakeFiles/seeWhat.dir/seeWhat.c.o: ../seeWhat.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/seeWhat.dir/seeWhat.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/seeWhat.dir/seeWhat.c.o   -c /home/eksor/GitHub/CSE244_HWs/Midterm/seeWhat.c

CMakeFiles/seeWhat.dir/seeWhat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/seeWhat.dir/seeWhat.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/eksor/GitHub/CSE244_HWs/Midterm/seeWhat.c > CMakeFiles/seeWhat.dir/seeWhat.c.i

CMakeFiles/seeWhat.dir/seeWhat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/seeWhat.dir/seeWhat.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/eksor/GitHub/CSE244_HWs/Midterm/seeWhat.c -o CMakeFiles/seeWhat.dir/seeWhat.c.s

CMakeFiles/seeWhat.dir/seeWhat.c.o.requires:

.PHONY : CMakeFiles/seeWhat.dir/seeWhat.c.o.requires

CMakeFiles/seeWhat.dir/seeWhat.c.o.provides: CMakeFiles/seeWhat.dir/seeWhat.c.o.requires
	$(MAKE) -f CMakeFiles/seeWhat.dir/build.make CMakeFiles/seeWhat.dir/seeWhat.c.o.provides.build
.PHONY : CMakeFiles/seeWhat.dir/seeWhat.c.o.provides

CMakeFiles/seeWhat.dir/seeWhat.c.o.provides.build: CMakeFiles/seeWhat.dir/seeWhat.c.o


# Object files for target seeWhat
seeWhat_OBJECTS = \
"CMakeFiles/seeWhat.dir/seeWhat.c.o"

# External object files for target seeWhat
seeWhat_EXTERNAL_OBJECTS =

seeWhat: CMakeFiles/seeWhat.dir/seeWhat.c.o
seeWhat: CMakeFiles/seeWhat.dir/build.make
seeWhat: CMakeFiles/seeWhat.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable seeWhat"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/seeWhat.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/seeWhat.dir/build: seeWhat

.PHONY : CMakeFiles/seeWhat.dir/build

CMakeFiles/seeWhat.dir/requires: CMakeFiles/seeWhat.dir/seeWhat.c.o.requires

.PHONY : CMakeFiles/seeWhat.dir/requires

CMakeFiles/seeWhat.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/seeWhat.dir/cmake_clean.cmake
.PHONY : CMakeFiles/seeWhat.dir/clean

CMakeFiles/seeWhat.dir/depend:
	cd /home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/eksor/GitHub/CSE244_HWs/Midterm /home/eksor/GitHub/CSE244_HWs/Midterm /home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug /home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug /home/eksor/GitHub/CSE244_HWs/Midterm/cmake-build-debug/CMakeFiles/seeWhat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/seeWhat.dir/depend

