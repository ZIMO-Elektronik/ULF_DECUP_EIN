#include "../utility.hpp"
#include "rx_test.hpp"

TEST_F(RxTest, mx645) {
  Zsu(source_location_parent_path() / "../../data/DS240307.zsu")
    .ZsuPreamble(100uz)
    .ZsuDecoderId(221u)
    .ZsuBlockCount()
    .ZsuSecurityByte1()
    .ZsuSecurityByte2()
    .ZsuBlocks();
}

TEST_F(RxTest, zpp) {
  Zpp(source_location_parent_path() / "../../data/test.zpp")
    .ZppPreamble(100uz)
    .ZppCvRead(7u)
    .ZppCvRead(7u)
    .ZppDecoderId()
    .ZppCvRead(6u)
    .ZppCvRead(6u)
    .ZppFlashErase()
    .ZppFlashWrite()
    .ZppCvWrite(0u, 0u);
}
