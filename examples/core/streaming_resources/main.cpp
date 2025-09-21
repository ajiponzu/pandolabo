#include <exception>
#include <iostream>
#include <print>

#include "streaming_resources.hpp"

int main() {
  std::println("Starting StreamingResources application...");

  try {
    samples::core::StreamingResources app;
    std::println("Application object created successfully.");

    app.run();
    std::println("Application finished normally.");
  } catch (const std::exception& e) {
    std::println(stderr, "Exception caught: {}", e.what());
    return 1;
  } catch (...) {
    std::println(stderr, "Unknown exception caught!");
    return 1;
  }

  std::println("Main function exiting normally.");
  return 0;
}
