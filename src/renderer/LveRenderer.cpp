#include "renderer/LveRenderer.hpp"

#include <stdexcept>
#include <array>

namespace lve {
    LveRenderer::LveRenderer(
        LveWindow& window,
        LveDevice& device
    ) :
        lve_window(window),
        lve_device(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }
    LveRenderer::~LveRenderer() {
        freeCommandBuffers();
    }

    void LveRenderer::recreateSwapChain() {
        auto extent = lve_window.getExtent();
        while (extent.width == 0 ||extent.height == 0) {
            extent = lve_window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(lve_device.device());

        if (lve_swap_chain == nullptr)
            lve_swap_chain = std::make_unique<LveSwapChain>(lve_device, extent);
        else {
            std::shared_ptr<LveSwapChain> old_swap_chain = std::move(lve_swap_chain);
            lve_swap_chain = std::make_unique<LveSwapChain>(lve_device, extent, old_swap_chain);

            if (!old_swap_chain->compareSwapFormats(*lve_swap_chain))
                throw std::runtime_error("Swap chain image (or depth) format has changed!");
        }
    }

    void LveRenderer::createCommandBuffers() {
        command_buffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = lve_device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(lve_device.device(), &alloc_info, command_buffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");
    }

    void LveRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            lve_device.device(),
            lve_device.getCommandPool(),
            static_cast<uint32_t>(command_buffers.size()),
            command_buffers.data());
        command_buffers.clear();
    }

    
    VkCommandBuffer LveRenderer::beginFrame() {
        assert(!is_frame_started && "Can't call beginFrame while already in progress");

        auto result = lve_swap_chain->acquireNextImage(&current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");

        is_frame_started = true;

        auto command_buffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recordingg command buffer!");

        return command_buffer;
    }

    void LveRenderer::endFrame() {
        assert(is_frame_started && "Can't call endFrame while fram is not in progress");
        auto command_buffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");

        auto result = lve_swap_chain->submitCommandBuffers(&command_buffer, &current_image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lve_window.wasWindowResized()) {
            lve_window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        is_frame_started = false;
        current_frame_index = (current_frame_index + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer command_buffer) {
        assert(is_frame_started && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a diferent frame");

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = lve_swap_chain->getRenderPass();
        render_pass_info.framebuffer = lve_swap_chain->getFrameBuffer(current_image_index);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = lve_swap_chain->getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.1f, 0.1f, 0.1f,  1.0f};
        clear_values[1].depthStencil = {1.0f, 0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lve_swap_chain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lve_swap_chain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lve_swap_chain->getSwapChainExtent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void LveRenderer::endSwapChainRenderPass(VkCommandBuffer command_buffer) {
        assert(is_frame_started && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a diferent frame");

        vkCmdEndRenderPass(command_buffer);
    }
}