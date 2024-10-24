#include "rx_test.hpp"
#include <ranges>

RxTest::RxTest() {}

RxTest::~RxTest() {}

RxTest& RxTest::Zsu(std::filesystem::path path) {
  _zsu = zsu::read(path);
  return *this;
}

RxTest& RxTest::Preamble(size_t count) {
  for (auto i{0uz}; i < count; ++i) _mock.receive(0xEFu);
  return *this;
}

RxTest& RxTest::Decoder(uint8_t decoder_id) {
  EXPECT_CALL(_mock, transmit(_)).Times(AnyNumber());
  EXPECT_CALL(_mock, transmit(Contains(decoder_id)))
    .WillOnce(Return(2u))
    .RetiresOnSaturation();
  auto it{std::ranges::find_if(_zsu.firmwares, [this](auto&& fw) {
    return _mock.receive(fw.id) == ulf::decup_ein::ack;
  })};
  if (it != cend(_zsu.firmwares)) _fw = *it;
  return *this;
}

RxTest& RxTest::BlockCount() {
  auto block_count{static_cast<uint8_t>(size(_fw.bin) / 256u + 8u - 1u)};
  EXPECT_CALL(_mock, transmit(_)).WillOnce(Return(1u)).RetiresOnSaturation();
  _mock.receive(block_count);
  return *this;
}

RxTest& RxTest::SecurityByte1() {
  EXPECT_CALL(_mock, transmit(_)).WillOnce(Return(1u)).RetiresOnSaturation();
  _mock.receive(0x55u);
  return *this;
}

RxTest& RxTest::SecurityByte2() {
  EXPECT_CALL(_mock, transmit(_)).WillOnce(Return(1u)).RetiresOnSaturation();
  _mock.receive(0xAAu);
  return *this;
}

RxTest& RxTest::Data() {
  EXPECT_CALL(_mock, transmit(_)).WillRepeatedly(Return(2u));
  EXPECT_CALL(_mock, done()).Times(Exactly(1));
  auto count{0uz};
  for (auto chunk : _fw.bin | std::views::chunk(decup::decoder_id2data_size(
                                static_cast<uint8_t>(_fw.id)))) {
    _mock.receive(count);
    std::ranges::for_each(chunk, [this](uint8_t byte) { _mock.receive(byte); });
    _mock.receive(count++ ^ decup::exor(chunk));
  }
  return *this;
}