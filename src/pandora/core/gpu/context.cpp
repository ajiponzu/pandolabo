#include <string>
#include <unordered_set>

#include "pandora/core/gpu.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace pandora::core::gpu {

Context::Context(std::shared_ptr<gpu_ui::WindowSurface> ptr_window_surface) {
  // Helpers to manage instance extensions
  auto get_available_instance_extensions = []() {
    std::unordered_set<std::string> names;
    for (const auto& prop : vk::enumerateInstanceExtensionProperties()) {
      names.emplace(prop.extensionName.data());
    }
    return names;
  };

  auto contains_ext = [](const std::vector<const char*>& exts,
                         const char* name) {
    return std::any_of(exts.begin(), exts.end(), [&](const char* e) {
      return std::strcmp(e, name) == 0;
    });
  };

  auto try_add_ext = [&](std::vector<const char*>& exts,
                         const char* name,
                         const std::unordered_set<std::string>& available) {
    if (available.contains(name) && !contains_ext(exts, name)) {
      exts.emplace_back(name);
    }
  };

  // Initialize Vulkan.hpp
  {
    static vk::detail::DynamicLoader dl{};
    auto vk_get_instance_proc_addr =
        dl.getProcAddress<::PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_get_instance_proc_addr);
  }

  // Create Vulkan instance
  {
    vk::ApplicationInfo app_info{"pandolabo",
                                 PANDOLABO_VK_VERSION,
                                 "pandolabo",
                                 PANDOLABO_VK_VERSION,
                                 PANDOLABO_VK_VERSION};

    std::vector<const char*> extensions;
    const auto available_exts = get_available_instance_extensions();

    if (ptr_window_surface) {
      // Window mode: add GLFW extensions
      uint32_t extension_count = 0u;
      const auto glfw_extensions =
          glfwGetRequiredInstanceExtensions(&extension_count);
      extensions = std::vector<const char*>(glfw_extensions,
                                            glfw_extensions + extension_count);
      // VK_KHR_surface は通常 GLFW が返すため明示追加しない
    }
    // Headless mode: no GLFW or surface extensions needed

#ifdef GPU_DEBUG
    // ある場合のみ追加（重複も抑止）
    try_add_ext(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, available_exts);
    try_add_ext(
        extensions, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, available_exts);

    m_ptrMessenger = std::make_unique<debug::Messenger>();
    m_ptrInstance = m_ptrMessenger->createDebugInstance(app_info, extensions);
#else
    vk::InstanceCreateInfo create_info({}, &app_info, {}, extensions);
    m_ptrInstance = vk::createInstanceUnique(create_info, nullptr);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_ptrInstance);
#endif

    if (!m_ptrInstance) {
      return;
    }
  }

  if (ptr_window_surface) {
    // Create window surface
    m_ptrWindowSurface = ptr_window_surface;
    m_ptrWindowSurface->constructSurface(m_ptrInstance);

// Create Vulkan device
#ifdef GPU_DEBUG
    m_ptrDevice = std::make_unique<Device>(
        m_ptrInstance, m_ptrWindowSurface->getSurface(), *m_ptrMessenger);
#else
    m_ptrDevice = std::make_unique<Device>(m_ptrInstance,
                                           m_ptrWindowSurface->getSurface());
#endif

    // Create Vulkan swapchain
    m_ptrSwapchain =
        std::make_unique<Swapchain>(*m_ptrDevice, m_ptrWindowSurface);
  } else {
// Create Vulkan device
#ifdef GPU_DEBUG
    m_ptrDevice = std::make_unique<Device>(
        m_ptrInstance, vk::UniqueSurfaceKHR(nullptr), *m_ptrMessenger);
#else
    m_ptrDevice =
        std::make_unique<Device>(m_ptrInstance, vk::UniqueSurfaceKHR(nullptr));
#endif
  }

  m_isInitialized = true;
}

Context::~Context() {
  m_ptrDevice.release();
  m_ptrInstance.release();
#ifdef GPU_DEBUG
  m_ptrMessenger.release();
#endif
}

void Context::resetSwapchain() {
  m_ptrDevice->waitIdle();
  m_ptrWindowSurface->setWindowSize();
  m_ptrSwapchain->resetSwapchain(*m_ptrDevice, m_ptrWindowSurface);
}

}  // namespace pandora::core::gpu
