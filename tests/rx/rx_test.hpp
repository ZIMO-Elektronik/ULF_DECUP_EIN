#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <zpp/zpp.hpp>
#include <zsu/zsu.hpp>
#include "rx_mock.hpp"

using namespace ::testing;

// Receive test fixture
struct RxTest : ::testing::Test {
  RxTest();
  virtual ~RxTest();

  RxTest* Zsu(std::filesystem::path path);
  RxTest* ZsuPreamble(size_t count);
  RxTest* ZsuDecoderId(uint8_t decoder_id);
  RxTest* ZsuBlockCount();
  RxTest* ZsuSecurityByte1();
  RxTest* ZsuSecurityByte2();
  RxTest* ZsuBlocks();

  RxTest* Zpp(std::filesystem::path path);
  RxTest* ZppPreamble(size_t count);
  RxTest* ZppDecoderId();
  RxTest* ZppCvRead(uint16_t cv);
  RxTest* ZppCvWrite(uint16_t cv, uint8_t val);
  RxTest* ZppFlashErase();
  RxTest* ZppFlashWrite();
  RxTest* ZppCRCorXOR();

  NiceMock<RxMock> _mock;
  zsu::File _zsu;
  zsu::Firmware _fw;

  zpp::File _zpp;
};
