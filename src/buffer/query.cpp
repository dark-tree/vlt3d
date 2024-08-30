#include "query.hpp"
#include "setup/device.hpp"
#include "setup/callback.hpp"

QueryPool::QueryPool(Device& device, VkQueryType type, int count, VkQueryPipelineStatisticFlags statistics) {
	VkQueryPoolCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.flags = 0;

	create_info.queryCount = count;
	create_info.queryType = type;
	create_info.pipelineStatistics = statistics;

	this->count = count;
	this->vk_device = device.vk_device;

	vkCreateQueryPool(device.vk_device, &create_info, AllocatorCallbackFactory::named("QueryPool"), &vk_pool);
}

void QueryPool::close() {
	vkDestroyQueryPool(vk_device, vk_pool, AllocatorCallbackFactory::named("QueryPool"));
}

Query QueryPool::read(int index) const {
	Query query;
	query.value = 0;
	query.status = 1;

	VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
	vkGetQueryPoolResults(vk_device, vk_pool, index, 1, sizeof(Query), &query, sizeof(Query), flags);

	return query;
}