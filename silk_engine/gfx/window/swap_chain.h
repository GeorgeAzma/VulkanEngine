#pragma once

#include "core/event.h"
#include "gfx/images/image.h"

class Surface;

class SwapChain : NonCopyable
{
public:
    SwapChain(const Surface& surface);
    ~SwapChain();

    uint32_t getWidth() const { return extent.width; }
    uint32_t getHeight() const { return extent.height; }
    size_t getImageCount() const { return images.size(); }
    uint32_t getImageIndex() const { return image_index; }
    const std::vector<shared<Image>>& getImages() const { return images; }

    void recreate();

    bool acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence = nullptr);
    bool present(VkSemaphore wait_semaphore);

    operator const VkSwapchainKHR& () const { return swap_chain; }

private:
    VkSwapchainKHR swap_chain = nullptr;
    std::vector<shared<Image>> images;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    uint32_t image_index = 0;
    const Surface& surface;
    bool vsync = false;
};