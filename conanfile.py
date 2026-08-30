from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy

from pathlib import Path

class Game2DConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = ["CMakeToolchain", "CMakeDeps"]
    
    requires = [
        "gtest/1.17.0@",
        "spdlog/1.14.1@",
        "nlohmann_json/3.10.5@",
        "sfml/3.0.2@",
        "taskflow/4.0.0@"
        ]

    default_options = {
        "gtest/*:shared": True,
        "glfw/*:shared": False,
        "fmt/*:shared": False,
        "spdlog/*:header_only": False,
        "spdlog/*:shared": False,
        "stb/*:shared": False,
        "nlohmann_json/*:shared": False,
        "glm/*:shared": False,
        "glslang/*:shared": False,
        "glslang/*:build_executables": False,
        "spirv-headers/*:shared": False,
        "spirv-tools/*:shared": False,
        "vulkan-memory-allocator/*:shared": False
        }

    def requirements(self):
        pass

    def build_requirements(self):
        self.tool_requires("cmake/3.30.5")
        
    def configure(self):
        self.options["spdlog"].header_only = False
        self.options["spdlog"].shared = False
        
    def layout(self):
        cmake_layout(self)
        
        
    def generate(self):
        for dep in self.dependencies.values():
            
            if self.settings.compiler == "msvc":
                bin_path = Path(self.source_folder) / "build" / "runtime" / str(self.settings.build_type)
                lib_ext = "*.dll"
                
            if dep.cpp_info.bindirs:
                copy(self, lib_ext, src=dep.cpp_info.bindirs[0], dst=bin_path, keep_path=False)
                
            if dep.cpp_info.libdirs:
                copy(self, lib_ext, src=dep.cpp_info.libdirs[0], dst=bin_path, keep_path=False)