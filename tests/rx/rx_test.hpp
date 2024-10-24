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

  RxTest& Zsu(std::filesystem::path path);
  RxTest& Preamble(size_t count);
  RxTest& Decoder(uint8_t decoder_id);
  RxTest& BlockCount();
  RxTest& SecurityByte1();
  RxTest& SecurityByte2();
  RxTest& Data();

protected:
  NiceMock<RxMock> _mock;
  zsu::File _zsu;
  zsu::Firmware _fw;
};
