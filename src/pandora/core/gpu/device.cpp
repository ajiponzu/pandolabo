#include <array>
#include <format>
#include <iostream>
#include <set>
#include <string>

#include "pandora/core/gpu.hpp"

static std::vector<const char*> getDeviceExtensions(bool has_window_surface) {
  std::vector<const char*> extensions = {
      VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
      VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
  };

  if (has_window_surface) {
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  return extensions;
}

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics;
  std::optional<uint32_t> compute;
  std::optional<uint32_t> transfer;

  bool is_complete() const {
    return graphics.has_value() && compute.has_value() && transfer.has_value();
  }
};

static QueueFamilyIndices find_queue_families(const vk::PhysicalDevice& physical_device) {
  QueueFamilyIndices indices;

  const auto queue_families = physical_device.getQueueFamilyProperties();

  uint32_t family_id = 0U;
  for (const auto& queue_family : queue_families) {
    const auto graphics_support = queue_family.queueFlags & vk::QueueFlagBits::eGraphics;

    const auto compute_support = queue_family.queueFlags & vk::QueueFlagBits::eCompute;

    const auto transfer_support = queue_family.queueFlags & vk::QueueFlagBits::eTransfer;

    if (graphics_support) {
      indices.graphics = family_id;
    } else if (compute_support) {
      indices.compute = family_id;
    } else if (transfer_support) {
      indices.transfer = family_id;
    }

    family_id += 1U;
  }

  return indices;
}

static bool check_device_extension_support(const vk::PhysicalDevice& physical_device,
                                           const std::vector<const char*>& device_extension_list) {
  std::set<std::string> device_extensions(device_extension_list.begin(), device_extension_list.end());
  const auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

  for (const auto& extension : available_extensions) {
    device_extensions.erase(extension.extensionName);
  }

  return device_extensions.empty();
}

template <typename T>
static T get_optional_value(const std::optional<T>& option) {
  if (option.has_value()) {
    return option.value();
  }

  return 0U;
}

pandora::core::gpu::Device::Device(const vk::UniqueInstance& ptr_instance,
                                   const vk::UniqueSurfaceKHR& ptr_window_surface
#ifdef GPU_DEBUG
                                   ,
                                   const std::unique_ptr<debug::Messenger>& ptr_messenger
#endif
                                   )
    : m_hasWindowSurface(ptr_window_surface.get() != nullptr) {
  const auto physical_devices = ptr_instance->enumeratePhysicalDevices();
  if (physical_devices.empty()) {
    m_physicalDevice = nullptr;
    return;
  }

  for (const auto& physical_device : physical_devices) {
    m_physicalDevice = physical_device;

    const auto queue_family_indices = find_queue_families(physical_device);

    if (!queue_family_indices.is_complete()) {
      continue;
    }

    m_queueFamilyIndices.graphics = queue_family_indices.graphics;
    m_queueFamilyIndices.compute = queue_family_indices.compute;
    m_queueFamilyIndices.transfer = queue_family_indices.transfer;

    if (ptr_window_surface) {
      const auto present_support =
          physical_device.getSurfaceSupportKHR(m_queueFamilyIndices.graphics.value(), ptr_window_surface.get());

      if (!present_support) {
        m_queueFamilyIndices.graphics.reset();
        m_queueFamilyIndices.compute.reset();
        m_queueFamilyIndices.transfer.reset();

        continue;
      }

      m_queueFamilyIndices.present = m_queueFamilyIndices.graphics;
    }

    const auto required_extensions = getDeviceExtensions(ptr_window_surface.get() != nullptr);
    const auto extension_support = check_device_extension_support(physical_device, required_extensions);

    auto is_suitable =
        m_queueFamilyIndices.graphics.has_value() && m_queueFamilyIndices.compute.has_value() && extension_support;
    if (ptr_window_surface) {
      bool swap_chain_adequate = false;
      if (extension_support) {
        const auto formats = physical_device.getSurfaceFormatsKHR(ptr_window_surface.get());
        const auto present_modes = physical_device.getSurfacePresentModesKHR(ptr_window_surface.get());

        swap_chain_adequate = !(formats.empty()) && !(present_modes.empty());
      }

      is_suitable = m_queueFamilyIndices.present.has_value() && swap_chain_adequate && is_suitable;
    }

    if (is_suitable) {
      break;
    }

    m_physicalDevice = nullptr;
  }

#ifdef GPU_DEBUG
  if (m_physicalDevice) {
    std::cout << std::format("vulkan_device: {}", m_physicalDevice.getProperties().deviceName.data()) << std::endl;
  }
  constructLogicalDevice(ptr_messenger);
#else
  constructLogicalDevice();
#endif
}

pandora::core::gpu::Device::~Device() {}

void pandora::core::gpu::Device::constructLogicalDevice(
#ifdef GPU_DEBUG
    const std::unique_ptr<debug::Messenger>& ptr_messenger
#endif
) {
  const float_t queue_priority = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  {
    std::set<std::optional<uint32_t>> queue_families = {
        m_queueFamilyIndices.graphics,
        m_queueFamilyIndices.compute,
        m_queueFamilyIndices.transfer,
    };

    for (const auto& queue_family : queue_families) {
      if (!queue_family.has_value()) {
        continue;
      }
      queue_create_infos.emplace_back(vk::DeviceQueueCreateInfo({}, queue_family.value(), 1U, &queue_priority));
    }
  }

  // Create timeline semaphore features
  vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features;
  timeline_semaphore_features.setTimelineSemaphore(VK_TRUE);

  vk::PhysicalDeviceFeatures2 features2;
  features2.setPNext(&timeline_semaphore_features);

  const auto required_extensions = getDeviceExtensions(m_hasWindowSurface);
  vk::DeviceCreateInfo create_info({}, queue_create_infos, {}, required_extensions, nullptr, &features2);

#ifdef GPU_DEBUG
  create_info.setPEnabledLayerNames(ptr_messenger->getValidationLayers());
#endif

  m_ptrLogicalDevice = m_physicalDevice.createDeviceUnique(create_info);
}

const uint32_t pandora::core::gpu::Device::getQueueFamilyIndex(const QueueFamilyType family_type) const {
  switch (family_type) {
    using enum QueueFamilyType;

    case Graphics:
      return get_optional_value(m_queueFamilyIndices.graphics);
    case Compute:
      return get_optional_value(m_queueFamilyIndices.compute);
    case Transfer:
      return get_optional_value(m_queueFamilyIndices.transfer);
    default:
      return 0U;
  }
}

vk::Queue pandora::core::gpu::Device::getQueue(const uint32_t queue_family_index) {
  return m_ptrLogicalDevice->getQueue(queue_family_index, 0U);
}

vk::SampleCountFlagBits pandora::core::gpu::Device::getMaxUsableSampleCount() const {
  const auto physical_device_props = m_physicalDevice.getProperties();
  const auto sample_count = physical_device_props.limits.framebufferColorSampleCounts &
                            physical_device_props.limits.framebufferDepthSampleCounts;

  // Check sample counts in descending order to find the maximum supported
  constexpr std::array sample_priorities = {vk::SampleCountFlagBits::e64,
                                            vk::SampleCountFlagBits::e32,
                                            vk::SampleCountFlagBits::e16,
                                            vk::SampleCountFlagBits::e8,
                                            vk::SampleCountFlagBits::e4,
                                            vk::SampleCountFlagBits::e2,
                                            vk::SampleCountFlagBits::e1};

  for (const auto& priority : sample_priorities) {
    if (sample_count & priority) {
      return priority;
    }
  }

  return vk::SampleCountFlagBits::e1;
}
