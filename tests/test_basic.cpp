#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic test", "[basic]") {
  REQUIRE(true);
}

TEST_CASE("Math operations", "[math]") {
  REQUIRE(2 + 2 == 4);
  REQUIRE(10 - 5 == 5);
  REQUIRE(3 * 4 == 12);
}

TEST_CASE("String test", "[string]") {
  std::string hello = "Hello";
  std::string world = "World";

  REQUIRE(hello.length() == 5);
  REQUIRE(world.length() == 5);
  REQUIRE(hello + " " + world == "Hello World");
}
