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
CMAKE_SOURCE_DIR = /home/eksor/GitHub/CSE244_HWs/HW04

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/HW4.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/HW4.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/HW4.dir/flags.make

CMakeFiles/HW4.dir/hw4.c.o: CMakeFiles/HW4.dir/flags.make
CMakeFiles/HW4.dir/hw4.c.o: ../hw4.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/HW4.dir/hw4.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/HW4.dir/hw4.c.o   -c /home/eksor/GitHub/CSE244_HWs/HW04/hw4.c

CMakeFiles/HW4.dir/hw4.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/HW4.dir/hw4.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/eksor/GitHub/CSE244_HWs/HW04/hw4.c > CMakeFiles/HW4.dir/hw4.c.i

CMakeFiles/HW4.dir/hw4.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/HW4.dir/hw4.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/eksor/GitHub/CSE244_HWs/HW04/hw4.c -o CMakeFiles/HW4.dir/hw4.c.s

CMakeFiles/HW4.dir/hw4.c.o.requires:

.PHONY : CMakeFiles/HW4.dir/hw4.c.o.requires

CMakeFiles/HW4.dir/hw4.c.o.provides: CMakeFiles/HW4.dir/hw4.c.o.requires
	$(MAKE) -f CMakeFiles/HW4.dir/build.make CMakeFiles/HW4.dir/hw4.c.o.provides.build
.PHONY : CMakeFiles/HW4.dir/hw4.c.o.provides

CMakeFiles/HW4.dir/hw4.c.o.provides.build: CMakeFiles/HW4.dir/hw4.c.o


# Object files for target HW4
HW4_OBJECTS = \
"CMakeFiles/HW4.dir/hw4.c.o"

# External object files for target HW4
HW4_EXTERNAL_OBJECTS =

HW4: CMakeFiles/HW4.dir/hw4.c.o
HW4: CMakeFiles/HW4.dir/build.make
HW4: CMakeFiles/HW4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable HW4"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/HW4.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/HW4.dir/build: HW4

.PHONY : CMakeFiles/HW4.dir/build

CMakeFiles/HW4.dir/requires: CMakeFiles/HW4.dir/hw4.c.o.requires

.PHONY : CMakeFiles/HW4.dir/requires

CMakeFiles/HW4.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/HW4.dir/cmake_clean.cmake
.PHONY : CMakeFiles/HW4.dir/clean

CMakeFiles/HW4.dir/depend:
	cd /home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/eksor/GitHub/CSE244_HWs/HW04 /home/eksor/GitHub/CSE244_HWs/HW04 /home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug /home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug /home/eksor/GitHub/CSE244_HWs/HW04/cmake-build-debug/CMakeFiles/HW4.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/HW4.dir/depend

