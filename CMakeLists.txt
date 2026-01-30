cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.12.2
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  ULF_DECUP_EIN
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

file(GLOB_RECURSE SRC src/*.cpp)
add_library(ULF_DECUP_EIN STATIC ${SRC})
add_library(ULF::DECUP_EIN ALIAS ULF_DECUP_EIN)

target_compile_features(ULF_DECUP_EIN INTERFACE cxx_std_23)

# https://github.com/espressif/esp-idf/issues/17773
if(PROJECT_IS_TOP_LEVEL AND NOT ESP_PLATFORM)
  target_include_directories(
    ULF_DECUP_EIN
    INTERFACE include
    PRIVATE include/ulf/decup_ein)
  target_common_warnings(ULF_DECUP_EIN PRIVATE)
  target_common_errors(ULF_DECUP_EIN PRIVATE)
else()
  target_include_directories(
    ULF_DECUP_EIN SYSTEM
    INTERFACE include
    PRIVATE include/ulf/decup_ein)
endif()

if(NOT TARGET DECUP::DECUP)
  cpmaddpackage("gh:ZIMO-Elektronik/DECUP@0.2.0")
endif()

if(NOT TARGET Microsoft.GSL::GSL)
  cpmaddpackage("gh:microsoft/GSL@4.2.1")
endif()

target_link_libraries(ULF_DECUP_EIN PUBLIC DECUP::DECUP Microsoft.GSL::GSL)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  # add_subdirectory(examples)
  file(
    DOWNLOAD
    "https://github.com/ZIMO-Elektronik/.github/raw/master/data/.clang-format"
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC include/*.*pp tests/*.*pp)
  add_clang_format_target(ULF_DECUP_EINFormat OPTIONS -i FILES ${SRC})
  add_include_what_you_must_target(ULF_DECUP_EINIncludeWhatYouMust TARGET
                                   ULF_DECUP_EIN)
endif()

if(BUILD_TESTING
   AND PROJECT_IS_TOP_LEVEL
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
