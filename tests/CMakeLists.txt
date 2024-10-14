include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(ULF_DECUP_EINTests ${SRC})

sanitize(address,undefined)

target_common_warnings(ULF_DECUP_EINTests PRIVATE)

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  "google/googletest"
  GIT_TAG
  main
  SYSTEM
  YES
  OPTIONS
  "INSTALL_GTEST OFF")

target_link_libraries(ULF_DECUP_EINTests PRIVATE GTest::gtest_main GTest::gmock)

gtest_discover_tests(ULF_DECUP_EINTests)
