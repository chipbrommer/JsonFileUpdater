cmake_minimum_required(VERSION 3.11)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("JsonFileUpdater")

# Add source to this project's executable.
add_executable(JsonFileUpdater 
    "json_updater.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JsonFileUpdater PROPERTY CXX_STANDARD 20)
endif()

# Download nlohmann_json and cxxopts
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(nlohmann_json cxxopts)

# Link the fetched libraries to the executable
target_link_libraries(JsonFileUpdater PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts)

# Set the output directory to the project root and executable name
set_target_properties(JsonFileUpdater PROPERTIES
    OUTPUT_NAME "json_updater"
)
