#include "query.hpp"
#include "setup/device.hpp"
#include "setup/callback.hpp"
#include "setup/debug.hpp"

QueryPool::QueryPool(Device& device, VkQueryType type, int count, VkQueryPipelineStatisticFlags statistics) {
	VkQueryPoolCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.flags = 0;

	create_info.queryCount = count;
	create_info.queryType = type;
	create_info.pipelineStatistics = statistics;

	results.resize(count);
	this->vk_device = device.vk_device;

	vkCreateQueryPool(device.vk_device, &create_info, AllocatorCallbackFactory::named("QueryPool"), &vk_pool);
}

void QueryPool::close() {
	vkDestroyQueryPool(vk_device, vk_pool, AllocatorCallbackFactory::named("QueryPool"));
}

void QueryPool::load() {
	const size_t count = size();

	VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
	vkGetQueryPoolResults(vk_device, vk_pool, 0, count, sizeof(Query) * count, results.data(), sizeof(Query), flags);
}

Query QueryPool::read(int index) const {
	return results[index];
}

size_t QueryPool::size() const {
	return results.size();
}

void QueryPool::setDebugName(const char* name) const {
	VulkanDebug::name(vk_device, VK_OBJECT_TYPE_QUERY_POOL, vk_pool, name);
}