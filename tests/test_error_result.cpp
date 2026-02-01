#include <catch2/catch_test_macros.hpp>
#include <string>

#include "pandolabo.hpp"

using namespace pandora::core;

TEST_CASE("Error factories and context", "[error]") {
  const auto base = Error::runtime("runtime failure");
  REQUIRE(base.type() == ErrorType::Runtime);
  REQUIRE(base.message() == "runtime failure");

  const auto with_context = base.withContext("stage");
  REQUIRE(with_context.type() == ErrorType::Runtime);
  REQUIRE(with_context.context() == "stage");

  const auto text = with_context.toString();
  REQUIRE(text.find("Runtime") != std::string::npos);
  REQUIRE(text.find("stage") != std::string::npos);
  REQUIRE(text.find("runtime failure") != std::string::npos);
}

TEST_CASE("Result basic behavior", "[result]") {
  Result<int> ok_value{42};
  REQUIRE(ok_value.isOk());
  REQUIRE_FALSE(ok_value.isError());
  REQUIRE(ok_value.value() == 42);

  Result<int> err_value{Error::config("config error")};
  REQUIRE(err_value.isError());
  REQUIRE(err_value.error().type() == ErrorType::Config);

  const auto void_ok = ok();
  REQUIRE(void_ok.isOk());
}
