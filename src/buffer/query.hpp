#pragma once

#include "external.hpp"

class Device;

struct Query {

	READONLY uint64_t value;
	READONLY uint64_t status;

	/**
	 * Verify that this method returns true before trying to
	 * read the value returned by the query read (.value)
	 */
	bool present() const {
		return status != 0;
	}

};

static_assert(sizeof(Query) == 2 * sizeof(uint64_t));

/**
 * For information regarding Vulkan Timestamps see
 * https://nikitablack.github.io/post/how_to_use_vulkan_timestamp_queries/
 */
class QueryPool {

	public:

		READONLY uint32_t count;
		READONLY VkDevice vk_device;
		READONLY VkQueryPool vk_pool;

	public:

		QueryPool() = default;
		QueryPool(Device& device, VkQueryType type, int count, VkQueryPipelineStatisticFlags statistics = 0);

		/**
		 * Free the underlying vulkan object
		 */
		void close();

		/**
		 * Return the specified query value
		 */
		Query read(int index) const;

};
