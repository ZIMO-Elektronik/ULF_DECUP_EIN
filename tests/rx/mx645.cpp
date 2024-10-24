#include "../utility.hpp"
#include "rx_test.hpp"

TEST_F(RxTest, mx645) {
  Zsu(source_location_parent_path() / "../../data/DS240307.zsu")
    .Preamble(100uz)
    .Decoder(221u)
    .BlockCount()
    .SecurityByte1()
    .SecurityByte2()
    .Data();
}