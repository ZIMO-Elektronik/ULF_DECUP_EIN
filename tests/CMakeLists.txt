include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(ULF_DECUP_EINTests ${SRC})

sanitize(address,undefined)

target_common_warnings(ULF_DECUP_EINTests PRIVATE)
target_common_errors(ULF_DECUP_EINTests PRIVATE -Werror)

cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")
cpmaddpackage("gh:ZIMO-Elektronik/ZSU@0.1.1")
cpmaddpackage("gh:ZIMO-Elektronik/ZPP@0.1.1")

target_link_libraries(
  ULF_DECUP_EINTests PRIVATE GTest::gtest_main GTest::gmock ULF::DECUP_EIN
                             ZSU::ZSU ZPP::ZPP)

gtest_discover_tests(ULF_DECUP_EINTests)
