#pragma once

#include <gtest/gtest.h>
#include <filesystem>
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

  NiceMock<RxMock> _mock;
  zsu::File _zsu;
  zsu::Firmware _fw;
};
