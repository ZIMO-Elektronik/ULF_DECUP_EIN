#include <ranges>
#include <string_view>
#include "../utility.hpp"
#include "rx_test.hpp"

constexpr std::string_view path{"../../data/test.zpp"};

using testing::InSequence;

TEST_F(RxTest, zpp_preamble) {
  Zpp(source_location_parent_path() / path);

  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(std::to_underlying(decup::Command::Preamble0)),
             decup::Timeouts::zpp_preamble))
    .Times(Exactly(100 * 2));
  EXPECT_CALL(
    _mock,
    transmit(ElementsAre(std::to_underlying(decup::Command::Preamble1)),
             decup::Timeouts::zpp_preamble))
    .Times(Exactly(100));

  ZppPreamble(300uz);
}

TEST_F(RxTest, zpp_cv_read) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);
  uint16_t const cv{8u - 1u};

  {
    InSequence s;
    EXPECT_CALL(_mock,
                transmit(ElementsAre(std::to_underlying(decup::Command::CvRead),
                                     cv >> 0u,
                                     cv >> 8u),
                         decup::Timeouts::zpp_cv_read))
      .Times(Exactly(1));
    EXPECT_CALL(_mock,
                transmit(ElementsAre(0xFF), decup::Timeouts::zpp_cv_read))
      .Times(Exactly(sizeof(char) * 8u - 1u));
  }

  ZppCvRead(cv);
}

TEST_F(RxTest, zpp_cv_write) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);
  uint16_t const cv{3u - 1u};
  uint8_t const val{128u};

  {
    InSequence s;
    EXPECT_CALL(_mock,
                transmit(ElementsAre(0x6u, 0xAAu, cv >> 0u, cv >> 8u, _, val),
                         decup::Timeouts::zpp_cv_write))
      .Times(Exactly(1));
  };

  ZppCvWrite(cv, val);
}

TEST_F(RxTest, zpp_decoder_id) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);

  {
    InSequence s;
    EXPECT_CALL(
      _mock,
      transmit(ElementsAre(std::to_underlying(decup::Command::ReadDecoderType)),
               decup::Timeouts::zpp_decoder_id))
      .Times(Exactly(1));
    EXPECT_CALL(_mock,
                transmit(ElementsAre(0xFFu), decup::Timeouts::zpp_decoder_id))
      .Times(Exactly(sizeof(char) * 8u - 1u));
  }

  ZppDecoderId();
}

TEST_F(RxTest, zpp_flash_erase) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);

  {
    InSequence s;
    EXPECT_CALL(
      _mock,
      transmit(
        ElementsAre(
          std::to_underlying(decup::Command::DeleteFlash), 0x55u, 0xFFu, 0xFFu),
        decup::Timeouts::zpp_flash_erase))
      .Times(Exactly(1));
  }

  ZppFlashErase();
}

TEST_F(RxTest, zpp_flash_write) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);

  auto const chunks{std::views::chunk(_zpp.flash, 256u)};

  {
    InSequence s;
    EXPECT_CALL(_mock, transmit(_, decup::Timeouts::zpp_flash_write))
      .Times(Exactly(static_cast<int>(chunks.size())));
  }

  ZppFlashWrite();
}

TEST_F(RxTest, zpp_crc_or_xor) {
  Zpp(source_location_parent_path() / path);
  ZppPreamble(100uz);

  {
    InSequence s;
    EXPECT_CALL(
      _mock,
      transmit(ElementsAre(std::to_underlying(decup::Command::CRCorXORQuery)),
               decup::Timeouts::zpp_crc_or_xor))
      .Times(Exactly(1));
    EXPECT_CALL(_mock,
                transmit(ElementsAre(0xFFu), decup::Timeouts::zpp_crc_or_xor))
      .Times(Exactly(sizeof(char) * 8u - 1u));
  }

  ZppCRCorXOR();
}
