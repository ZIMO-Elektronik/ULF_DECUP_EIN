#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ulf/decup_ein.hpp>

struct RxMock : ulf::decup_ein::rx::Base {
  MOCK_METHOD(uint8_t,
              transmit,
              (std::span<uint8_t const>, uint32_t),
              (override));
};
