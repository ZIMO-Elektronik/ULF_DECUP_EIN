#include "rx_test.hpp"
#include <ranges>
#include <ulf/decup_ein.hpp>

RxTest::RxTest() {}

RxTest::~RxTest() {}

RxTest& RxTest::Zsu(std::filesystem::path path) {
  _zsu = zsu::read(path);
  return *this;
}

RxTest& RxTest::ZsuPreamble(size_t count) {
  for (auto i{0uz}; i < count; ++i)
    _mock.receive(std::to_underlying(count % 2uz ? decup::Command::Preamble0
                                                 : decup::Command::Preamble1));
  return *this;
}

RxTest& RxTest::ZsuDecoderId(uint8_t decoder_id) {
  EXPECT_CALL(_mock, transmit(_, _)).Times(AnyNumber());
  EXPECT_CALL(_mock, transmit(ElementsAre(decoder_id), _))
    .WillOnce(Return(2u))
    .RetiresOnSaturation();
  auto it{std::ranges::find_if(_zsu.firmwares, [this](auto&& fw) {
    return _mock.receive(static_cast<uint8_t>(fw.id)) == ulf::decup_ein::ack;
  })};
  if (it != cend(_zsu.firmwares)) _fw = *it;
  return *this;
}

RxTest& RxTest::ZsuBlockCount() {
  auto block_count{static_cast<uint8_t>(size(_fw.bin) / 256u + 8u - 1u)};
  EXPECT_CALL(_mock, transmit(ElementsAre(block_count), _))
    .WillOnce(Return(1u))
    .RetiresOnSaturation();
  _mock.receive(block_count);
  return *this;
}

RxTest& RxTest::ZsuSecurityByte1() {
  EXPECT_CALL(_mock, transmit(ElementsAre(0x55u), _))
    .WillOnce(Return(1u))
    .RetiresOnSaturation();
  _mock.receive(0x55u);
  return *this;
}

RxTest& RxTest::ZsuSecurityByte2() {
  EXPECT_CALL(_mock, transmit(ElementsAre(0xAAu), _))
    .WillOnce(Return(1u))
    .RetiresOnSaturation();
  _mock.receive(0xAAu);
  return *this;
}

RxTest& RxTest::ZsuBlocks() {
  EXPECT_CALL(_mock, transmit(_, _))
    .Times(Exactly(836))
    .WillRepeatedly(Return(2u));
  uint8_t count{0u};
  for (auto chunk : _fw.bin | std::views::chunk(decup::decoder_id2data_size(
                                static_cast<uint8_t>(_fw.id)))) {
    _mock.receive(count);
    std::ranges::for_each(chunk, [this](uint8_t byte) { _mock.receive(byte); });
    _mock.receive(count++ ^ decup::exor(chunk));
  }
  return *this;
}

RxTest& RxTest::Zpp(std::filesystem::path path) {
  _zpp = zpp::read(path);
  return *this;
}

RxTest& RxTest::ZppPreamble(size_t count) {

  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(std::to_underlying(decup::Command::Preamble0)),
             ulf::decup_ein::times::zpp_preamble))
    .Times(AnyNumber());
  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(std::to_underlying(decup::Command::Preamble1)),
             ulf::decup_ein::times::zpp_preamble))
    .Times(AnyNumber());
  for (auto i{0uz}; i < count; ++i) switch (count % 3uz) {
      case 0u:
        _mock.receive(std::to_underlying(decup::Command::Preamble0));
        break;
      case 1u:
        _mock.receive(std::to_underlying(decup::Command::Preamble0));
        break;
      case 2u:
        _mock.receive(std::to_underlying(decup::Command::Preamble1));
        break;
    }
  return *this;
}

RxTest& RxTest::ZppCvRead(uint16_t cv) {
  EXPECT_CALL(_mock,
              transmit(ElementsAre(std::to_underlying(decup::Command::CvRead),
                                   cv >> 0u,
                                   cv >> 8u),
                       ulf::decup_ein::times::zpp_cv_read))
    .Times(1u);
  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(0xFF), std::to_underlying(decup::Command::CvRead)))
    .Times(sizeof(char) * 8u - 1u);

  _mock.receive(std::to_underlying(decup::Command::CvRead));
  _mock.receive(static_cast<uint8_t>(cv >> 0u));
  _mock.receive(static_cast<uint8_t>(cv >> 8u));
  for (size_t idx{0uz}; idx < (sizeof(char) * 8u) - 1u; idx++) {
    _mock.receive(0xFFu);
  }
  return *this;
}

RxTest& RxTest::ZppCvWrite(uint16_t cv, uint8_t val) {
  _mock.receive(0x6u);
  _mock.receive(0xAAu);
  _mock.receive(static_cast<uint8_t>(cv >> 0u));
  _mock.receive(static_cast<uint8_t>(cv >> 8u));
  _mock.receive(val);
  return *this;
}

RxTest& RxTest::ZppDecoderId() {
  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(std::to_underlying(decup::Command::ReadDecoderType)),
             ulf::decup_ein::times::zpp_decoder_id))
    .Times(1);
  EXPECT_CALL(
    _mock, transmit(ElementsAre(0xFFu), ulf::decup_ein::times::zpp_decoder_id))
    .Times(sizeof(char) * 8u - 1u);
  _mock.receive(std::to_underlying(decup::Command::ReadDecoderType));
  for (size_t idx{0uz}; idx < (sizeof(char) * 8u) - 1u; idx++) {
    _mock.receive(0xFFu);
  }
  return *this;
}

RxTest& RxTest::ZppFlashErase() {
  EXPECT_CALL(
    _mock,
    transmit(
      ElementsAre(
        std::to_underlying(decup::Command::DeleteFlash), 0x55u, 0xFFu, 0xFFu),
      ulf::decup_ein::times::zpp_flash_erase))
    .Times(1);
  _mock.receive(std::to_underlying(decup::Command::DeleteFlash));
  _mock.receive(0x55u);
  _mock.receive(0xFFu);
  _mock.receive(0xFFu);
  return *this;
}

RxTest& RxTest::ZppFlashWrite() {
  uint16_t block{0u};
  for (auto chunk : _fw.bin | std::views::chunk(256u)) {
    EXPECT_CALL(_mock, transmit(_, ulf::decup_ein::times::zpp_flash_write))
      .Times(1);

    _mock.receive(0x05u);
    _mock.receive(0x55u);
    _mock.receive(static_cast<uint8_t>(block >> 0u));
    _mock.receive(static_cast<uint8_t>(block >> 8u));
    std::ranges::for_each(chunk, [this](uint8_t byte) { _mock.receive(byte); });
    _mock.receive(decup::crc8(chunk, 0x55u));
    block++;
  }
  return *this;
}

RxTest& RxTest::ZppCRCorXOR() {
  _mock.receive(std::to_underlying(decup::Command::CRCorXORQuery));
  for (size_t idx{0uz}; idx < (sizeof(char) * 8u) - 1u; idx++) {
    _mock.receive(0xFFu);
  }
  return *this;
}
