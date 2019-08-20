#pragma once

#include "vk_render_base.h"

namespace Echo
{
	class VKSwapChain
	{
	public:
		VKSwapChain();
		~VKSwapChain();

		// create
		void create(VkDevice vkDevice);

        // get vk swapchain
        VkSwapchainKHR* getVkSwapchain() { return &m_vkSwapChain; }

        // get vk image
        VkImage getVkImage(ui32 idx) { return m_vkSwapChainImages[idx]; }

	private:
		// create swap chain
		void createSwapChain(VkDevice vkDevice);

		// create image views
		void createImageViews(VkDevice vkDevice);

		// check surface format
		VkSurfaceFormatKHR pickSurfaceSupportFormat();

	private:
		VkSwapchainKHR				m_vkSwapChain = VK_NULL_HANDLE;
        vector<VkImage>::type       m_vkSwapChainImages;
		vector<VkImageView>::type	m_vkSwapChainImageViews;
	};
}
