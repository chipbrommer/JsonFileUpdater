# CMakeList.txt : CMake project for ConfigurationFileUpdater, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ConfigurationFileUpdater")

# Add source to this project's executable.
add_executable (ConfigurationFileUpdater 
    "main.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ConfigurationFileUpdater PROPERTY CXX_STANDARD 20)
endif()

# Include nlohmanns json
target_include_directories(ConfigurationFileUpdater PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)

# Download nlohmann json if it doesn't exist
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.10.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Set the desired name for the executable with an extension based on platform
if (WIN32)
    set_target_properties(ConfigurationFileUpdater PROPERTIES OUTPUT_NAME "FileUpdater.exe")
elseif (APPLE)
    set_target_properties(ConfigurationFileUpdater PROPERTIES OUTPUT_NAME "FileUpdater")
elseif (UNIX)
    set_target_properties(ConfigurationFileUpdater PROPERTIES OUTPUT_NAME "FileUpdater.out")
endif()

