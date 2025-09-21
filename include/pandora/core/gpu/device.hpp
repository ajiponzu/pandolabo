/*
 * device.hpp - GPU device wrapper for Pandolabo Vulkan C++ wrapper
 *
 * This header contains the Device class which wraps Vulkan physical and logical
 * devices. The Device class manages GPU resources, memory allocation, and queue
 * operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

#include "../types.hpp"
#include "config.hpp"

#ifdef GPU_DEBUG
  #include "debug.hpp"
#endif

namespace pandora::core::gpu {

/// @brief GPU device wrapper class
/// @details This class wraps Vulkan physical and logical devices.
/// When GPU resources, memory, or other operations are created, this class is
/// used. Management and operation authority is mainly under Context class.
///
/// Rule of Five
class Device {
 private:
  vk::PhysicalDevice m_physicalDevice;
  vk::UniqueDevice m_ptrLogicalDevice;
  bool m_hasWindowSurface;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> compute;
    std::optional<uint32_t> transfer;
    std::optional<uint32_t> present;
  } m_queueFamilyIndices;

 public:
  /// @brief Construct Device with Vulkan instance and window surface
  /// @param ptr_instance Vulkan instance
  /// @param ptr_window_surface Window surface for presentation
  /// @param ptr_messenger Debug messenger (debug builds only)
  Device(const vk::UniqueInstance& ptr_instance,
         const vk::UniqueSurfaceKHR& ptr_window_surface
#ifdef GPU_DEBUG
         ,
         const std::unique_ptr<debug::Messenger>& ptr_messenger
#endif
  );

  // Rule of Five
  ~Device();
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;

  /// @brief Get physical device handle
  /// @return Reference to Vulkan physical device
  const auto& getPhysicalDevice() const {
    return m_physicalDevice;
  }

  /// @brief Get logical device handle
  /// @return Reference to Vulkan logical device unique pointer
  const auto& getPtrLogicalDevice() const {
    return m_ptrLogicalDevice;
  }

  /// @brief Search queue family index from queue family type
  /// @details Queue in the header file means command queue for GPU calculation.
  /// Queue family is a group of queues. Each queue family has different
  /// purposes and allowed operations.
  /// @param family_type Queue target selection enum
  /// @return Queue family index
  uint32_t getQueueFamilyIndex(QueueFamilyType family_type) const;

  /// @brief Get GPU command queue
  /// @param queue_family_index Queue family index (0 or result from
  /// getQueueFamilyIndex)
  /// @return Vulkan queue object
  vk::Queue getQueue(uint32_t queue_family_index);

  /// @brief Get maximum usable sample count for multisampling
  /// @return Maximum sample count supported by the device
  vk::SampleCountFlagBits getMaxUsableSampleCount() const;

  /// @brief Wait until all GPU operations are complete
  /// @details From performance perspective, this function is not recommended.
  /// This function should be used only for application shutdown.
  void waitIdle() const {
    m_ptrLogicalDevice->waitIdle();
  }

 private:
  /// @brief Construct logical device with validation layers (if debug)
  /// @param ptr_messenger Debug messenger for validation (debug builds only)
  void constructLogicalDevice(
#ifdef GPU_DEBUG
      const std::unique_ptr<debug::Messenger>& ptr_messenger
#endif
  );
};

}  // namespace pandora::core::gpu
