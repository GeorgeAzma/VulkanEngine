#pragma once

class Queue
{
	friend class QueueFamily;

public:
	Queue(VkQueue queue, float priority = 1.0f);

	VkResult present(const VkPresentInfoKHR& present_info) const;
	void submit(const VkSubmitInfo& submit_info, VkFence fence = nullptr) const;
	void wait() const;

	operator const VkQueue& () const { return queue; }

private:
	VkQueue queue = nullptr;
	float priority = 1.0f;
};