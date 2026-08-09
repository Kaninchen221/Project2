@echo off

echo Remove temp files
if exist CMakeUserPresets.json (del CMakeUserPresets.json && echo Remove CMakeUserPresets.json)
if exist build (rmdir /s /q build && echo Remove build folder)

echo ----------------------------------------------
echo --- Debug ---
echo ----------------------------------------------
conan install . --build=missing --profile:build=conan_profile_windows_msvc_2026_debug.txt --profile:host=conan_profile_windows_msvc_2026_debug.txt
cmake -S . -B build -G "Visual Studio 18 2026" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=build/generators