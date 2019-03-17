#include "panini.hpp"

#include <gtest/gtest.h>

using namespace std::literals::string_view_literals;
using String = std::string_view;

TEST(panini, ReadsEmptyFiles)
{
    panini::parse("", [](panini::State, String, String, String) { FAIL(); });
}

TEST(panini, ReadsPracticallyEmptyFiles)
{
    panini::parse(  //
        R"(
; This file contains nothing

[a]
; Section a

[b]
  [c]
)",
        [](panini::State, String, String, String) { FAIL(); });
}

TEST(panini, ReadsRootKeyValues)
{
    panini::parse(  //
        R"(panini = bread)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            ASSERT_EQ(section, ""sv);
            ASSERT_EQ(key, "panini"sv);
            ASSERT_EQ(value, "bread"sv);
        });
}

TEST(panini, ReadsSectionedKeyValues)
{
    panini::parse(  //
        R"(
[primary]
panini = 🥪

[secondary]
panini = 🍞
)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            if (section == "primary"sv) {
                ASSERT_EQ(key, "panini"sv);
                ASSERT_EQ(value, "🥪"sv);
            } else if (section == "secondary"sv) {
                ASSERT_EQ(key, "panini"sv);
                ASSERT_EQ(value, "🍞"sv);
            } else {
                FAIL();
            }
        });
}

TEST(panini, IgnoresLeadingAndTrailingWhitespace)
{
    panini::parse(  //
        R"(
[ a section ]
  my key = my value
  this is another key = yet another value
)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            ASSERT_EQ(section, "a section"sv);
            if (key == "my key"sv) {
                ASSERT_EQ(value, "my value"sv);
            } else if (key == "this is another key"sv) {
                ASSERT_EQ(value, "yet another value"sv);
            } else {
                FAIL();
            }
        });
}

TEST(panini, AllowsEmptyValues)
{
    panini::parse(  //
        R"(
bread =
sandwich = 🥪
)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            ASSERT_EQ(section, ""sv);
            if (key == "bread"sv) {
                ASSERT_EQ(value, ""sv);
            } else if (key == "sandwich"sv) {
                ASSERT_EQ(value, "🥪"sv);
            } else {
                FAIL();
            }
        });

    panini::parse(  //
        R"(bread =)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            ASSERT_EQ(section, ""sv);
            ASSERT_EQ(key, "bread"sv);
            ASSERT_EQ(value, ""sv);
        });
}

TEST(panini, AllowsTrailingComments)
{
    panini::parse(  //
        R"(
[panini ;🥪] ; sandwich section
panini = 🥪 ; sandwich
)",
        [](panini::State state, String section, String key, String value) {
            ASSERT_EQ(state, panini::State::Value);
            ASSERT_EQ(section, "panini ;🥪"sv);
            ASSERT_EQ(key, "panini"sv);
            ASSERT_EQ(value, "🥪"sv);
        });
}

TEST(panini, EqualsSignIsReserved)
{
    panini::parse(  //
        R"([panini=🥪])",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "1"sv);
            ASSERT_EQ(key, "Expected key before '='"sv);
        });
    panini::parse(  //
        R"(panini = 🥪 = 🍞])",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "1"sv);
            ASSERT_EQ(key, "Expected key before '='"sv);
        });
}

TEST(panini, HandlesMissingKey)
{
    panini::parse(  //
        R"(
  = 🍞
sandwich = 🥪
)",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "2"sv);
            ASSERT_EQ(key, "Expected key before '='"sv);
        });
}

TEST(panini, HandlesMissingValue)
{
    panini::parse(  //
        R"(
bread
sandwich = 🥪
)",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "2"sv);
            ASSERT_EQ(key, "Unexpected end of key"sv);
        });
}

TEST(panini, HandlesIncompleteSectionDeclaration)
{
    panini::parse(  //
        R"(
[panini
bread = 🍞
sandwich = 🥪
)",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "2"sv);
            ASSERT_EQ(key, "Unexpected end of section"sv);
        });
}

TEST(panini, HandlesInvalidSectionName)
{
    panini::parse(  //
        R"(
[panini=🥪]
bread = 🍞
sandwich = 🥪
)",
        [](panini::State state, String section, String key, String) {
            ASSERT_EQ(state, panini::State::Error);
            ASSERT_EQ(section, "2"sv);
            ASSERT_EQ(key, "Expected key before '='"sv);
        });
}

auto main(int argc, char* argv[]) -> int
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
