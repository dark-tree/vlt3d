#pragma once

#include "external.hpp"
#include "util/logger.hpp"

class AllocatorCallbackFactory {

	private:

		#if !defined(NDEBUG)
		class WrappedAllocator {

			private:

				// used for purpose of harmless code reuse
				static constexpr VkInternalAllocationType DUMMY_TYPE = VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE;
				static constexpr VkSystemAllocationScope DUMMY_SCOPE = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

				class AllocatorState {

					private:

						std::mutex mutex;
						const char* identifier;
						bool verbose;

						long allocations = 0;
						long allocated = 0;
						long frees = 0;
						long freed = 0;

					private:

						// helper formatting methods
						std::string pad(std::string str, int padding);
						std::string pad(size_t counter, int padding);

					public:

						AllocatorState(const char* identifier, bool verbose);

						/// Converts the user_data pointer to the AllocatorState object
						static AllocatorState* of(void* user_data);

						/// Returns a formatted ASCII table header
						static std::string getHeaderString();

						/// Returns the state of this allocator as a single row in a ASCII table
						std::string getRowString();

					public:

						/**
						 * Notify the allocator that memory was freed in it
						 * @param size the number of bytes freed
						 */
						void onFree(size_t size);

						/**
						 * Notify the allocator that memory was allocated in it
						 * @param size the number of bytes allocated
						 */
						void onAllocate(size_t size);

				};

				// the structure prepended to the allocated memory blocks
				struct Header {
					size_t size;
					void* root;
				};

				// Implementation of all the vulkan function required to form a VkAllocationCallbacks
				static void* vkProxyAllocationFunction(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope scope);
				static void vkProxyFreeFunction(void* user_data, void* pointer);
				static void* vkProxyReallocationFunction(void* user_data, void* pointer, size_t size, size_t alignment, VkSystemAllocationScope scope);
				static void vkProxyInternalAllocationNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);
				static void vkProxyInternalFreeNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope);

			public:

				READONLY VkAllocationCallbacks allocator;

				/**
				 * Construct an Allocator
				 *
				 * @param identifier the name of the tracked resource type, like "Sampler" or "Buffer"
				 * @param verbose controls weather each allocation and free call should be logged
				 */
				WrappedAllocator(const char* identifier, bool verbose);

				void print(bool header);

		};

		std::mutex mutex;
		std::unordered_map<std::string, WrappedAllocator> allocators;

		void printAllocators();
		WrappedAllocator& getAllocator(const char* identifier, bool verbose);

		/// get a singleton instance of AllocatorCallbackFactory
		static AllocatorCallbackFactory& getFactory();
		#endif

	public:

		/// get (or create) an allocator for resource named 'identifier'
		static VkAllocationCallbacks* named(const char* identifier, bool verbose = false);

		/// print all allocators as an ASCII table to the log
		static void print();

};