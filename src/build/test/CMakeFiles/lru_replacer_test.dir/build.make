# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.24

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = D:\Python3.10\Lib\site-packages\cmake\data\bin\cmake.exe

# The command to remove a file.
RM = D:\Python3.10\Lib\site-packages\cmake\data\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\VsCodeDocument\Lab\rucbase-lab\src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\VsCodeDocument\Lab\rucbase-lab\src\build

# Include any dependencies generated for this target.
include test/CMakeFiles/lru_replacer_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/lru_replacer_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/lru_replacer_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/lru_replacer_test.dir/flags.make

test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj: test/CMakeFiles/lru_replacer_test.dir/flags.make
test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj: test/CMakeFiles/lru_replacer_test.dir/includes_CXX.rsp
test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj: E:/VsCodeDocument/Lab/rucbase-lab/src/test/storage/lru_replacer_test.cpp
test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj: test/CMakeFiles/lru_replacer_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\VsCodeDocument\Lab\rucbase-lab\src\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj"
	cd /d E:\VsCodeDocument\Lab\rucbase-lab\src\build\test && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj -MF CMakeFiles\lru_replacer_test.dir\storage\lru_replacer_test.obj.d -o CMakeFiles\lru_replacer_test.dir\storage\lru_replacer_test.obj -c E:\VsCodeDocument\Lab\rucbase-lab\src\test\storage\lru_replacer_test.cpp

test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.i"
	cd /d E:\VsCodeDocument\Lab\rucbase-lab\src\build\test && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\VsCodeDocument\Lab\rucbase-lab\src\test\storage\lru_replacer_test.cpp > CMakeFiles\lru_replacer_test.dir\storage\lru_replacer_test.i

test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.s"
	cd /d E:\VsCodeDocument\Lab\rucbase-lab\src\build\test && C:\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\VsCodeDocument\Lab\rucbase-lab\src\test\storage\lru_replacer_test.cpp -o CMakeFiles\lru_replacer_test.dir\storage\lru_replacer_test.s

# Object files for target lru_replacer_test
lru_replacer_test_OBJECTS = \
"CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj"

# External object files for target lru_replacer_test
lru_replacer_test_EXTERNAL_OBJECTS =

test/lru_replacer_test.exe: test/CMakeFiles/lru_replacer_test.dir/storage/lru_replacer_test.obj
test/lru_replacer_test.exe: test/CMakeFiles/lru_replacer_test.dir/build.make
test/lru_replacer_test.exe: replacer/liblru_replacer.a
test/lru_replacer_test.exe: test/CMakeFiles/lru_replacer_test.dir/linklibs.rsp
test/lru_replacer_test.exe: test/CMakeFiles/lru_replacer_test.dir/objects1.rsp
test/lru_replacer_test.exe: test/CMakeFiles/lru_replacer_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\VsCodeDocument\Lab\rucbase-lab\src\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lru_replacer_test.exe"
	cd /d E:\VsCodeDocument\Lab\rucbase-lab\src\build\test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\lru_replacer_test.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/lru_replacer_test.dir/build: test/lru_replacer_test.exe
.PHONY : test/CMakeFiles/lru_replacer_test.dir/build

test/CMakeFiles/lru_replacer_test.dir/clean:
	cd /d E:\VsCodeDocument\Lab\rucbase-lab\src\build\test && $(CMAKE_COMMAND) -P CMakeFiles\lru_replacer_test.dir\cmake_clean.cmake
.PHONY : test/CMakeFiles/lru_replacer_test.dir/clean

test/CMakeFiles/lru_replacer_test.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\VsCodeDocument\Lab\rucbase-lab\src E:\VsCodeDocument\Lab\rucbase-lab\src\test E:\VsCodeDocument\Lab\rucbase-lab\src\build E:\VsCodeDocument\Lab\rucbase-lab\src\build\test E:\VsCodeDocument\Lab\rucbase-lab\src\build\test\CMakeFiles\lru_replacer_test.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/lru_replacer_test.dir/depend

