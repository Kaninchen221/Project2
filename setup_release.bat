@echo off

echo Remove temp files
if exist CMakeUserPresets.json (del CMakeUserPresets.json && echo Remove CMakeUserPresets.json)
if exist build (rmdir /s /q build && echo Remove build folder)

echo ----------------------------------------------
echo --- Release ---
echo ----------------------------------------------
conan install . --build=missing -pr=conan_profile_windows_msvc_release.txt
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=build/generators