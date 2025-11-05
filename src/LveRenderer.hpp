#pragma once

#include "LveWindow.hpp"
#include "LveDevice.hpp"
#include "LveSwapChain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace lve {
    class LveRenderer {
    public:
        LveRenderer(
            LveWindow& window,
            LveDevice& device
        );
        ~LveRenderer();

        LveRenderer(const LveRenderer &) = delete;
        LveRenderer &operator = (const LveRenderer &) = delete;

        inline VkRenderPass getSwapChainRenderPass() const { return lve_swap_chain->getRenderPass(); };
        inline float getAspectRation() const { return lve_swap_chain->extentAspectRatio(); };
        inline bool isFrameInProgress() const { return is_frame_started; }
        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(is_frame_started && "Cannot get command buffer when frame not in progress");
            return command_buffers[current_frame_index];
        }

        int getFrameIndex() const {
            assert(is_frame_started && "Cannot get frame index when frame not in progress");
            return current_frame_index;
        }

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer command_buffer);
        void endSwapChainRenderPass(VkCommandBuffer command_buffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        LveWindow& lve_window;
        LveDevice& lve_device;
        std::unique_ptr<LveSwapChain> lve_swap_chain;
        std::vector<VkCommandBuffer> command_buffers;

        uint32_t current_image_index = 0;
        int current_frame_index = 0;
        bool is_frame_started = false;
    };
}