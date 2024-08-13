#pragma once

#include "external.hpp"
#include "util/logger.hpp"

class AllocatorCallbackFactory {

	private:

		#if !defined(NDEBUG)
		class WrappedAllocator {

			private:

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

						std::string pad(std::string str, int padding) {
							str.append(padding - str.length(), ' ');
							return str;
						}

						std::string pad(size_t counter, int padding) {
							return pad(std::to_string(counter), padding);
						}

					public:

						AllocatorState(const char* identifier, bool verbose)
						: identifier(identifier), verbose(verbose) {}

						static AllocatorState* of(void* user_data) {
							return static_cast<AllocatorState*>(user_data);
						}

						static std::string getHeaderString() {
							return "| Identifier      | Allocations     | Allocated       |\n"
								   "+ --------------- + --------------- + --------------- +";
						}

						std::string getRowString() {
							std::lock_guard lock {mutex};

							long delta_allocations = allocations - frees;
							long delta_allocated = allocated - freed;

							return  "| " + pad(identifier, 15) + " | " + pad(delta_allocations, 15) + " | " + pad(delta_allocated, 15) + " |";
						}

					public:

						void onFree(size_t size) {
							std::lock_guard lock {mutex};
							frees ++;
							freed += size;

							if (verbose) {
								logger::debug("[Allocator] ", identifier, " freed ", size, " bytes");
							}
						}

						void onAllocate(size_t size) {
							std::lock_guard lock {mutex};
							allocations ++;
							allocated += size;

							if (verbose) {
								logger::debug("[Allocator] ", identifier, " allocated ", size, " bytes");
							}
						}

				};

				struct Header {
					size_t size;
					void* root;
				};

				static void* vkProxyAllocationFunction(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope scope) {
					size_t space = size + alignment;
					void* pointer = calloc(space + sizeof(Header), 1);

					// reserve enough space for at least our header but a
					// different location can be chosen after the pointer is aligned
					void* block = static_cast<Header*>(pointer) + 1;

					// try to satisfy alignment, this should always work
					// as we allocate with 'alignment' bytes of padding
					if (!std::align(alignment, size, block, space)) {
						logger::error("Failed to align block in debug allocator!");
					}

					// now attach our own data once we know
					// how the pointer was aligned
					Header* header = static_cast<Header*>(block) - 1;
					header->size = size;
					header->root = pointer;

					vkProxyInternalAllocationNotification(user_data, header->size, DUMMY_TYPE, DUMMY_SCOPE);
					return block;
				}

				static void vkProxyFreeFunction(void* user_data, void* pointer) {
					if (pointer == nullptr) {
						return;
					}

					// retrieve our header struct back, remember to free
					// header->root only after we finish using the header
					Header* header = static_cast<Header*>(pointer) - 1;
					vkProxyInternalFreeNotification(user_data, header->size, DUMMY_TYPE, DUMMY_SCOPE);
					free(header->root);
				}

				static void* vkProxyReallocationFunction(void* user_data, void* pointer, size_t size, size_t alignment, VkSystemAllocationScope scope) {

					// we implement a reallocation as a simple allocation-copy-free combo
					// no need to do smart things here, it's just for debugging anyway
					void* allocated = vkProxyAllocationFunction(user_data, size, alignment, scope);

					// we need to extract the old size to know how much
					// do we need to copy (realloc can both shrink and grow the allocation)
					Header* header = static_cast<Header*>(pointer) - 1;
					size_t bytes = std::min(size, header->size);

					// copy and free the old block
					memcpy(allocated, pointer, bytes);
					vkProxyFreeFunction(user_data, pointer);
					return allocated;
				}

				static void vkProxyInternalAllocationNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
					AllocatorState::of(user_data)->onAllocate(size);
				}

				static void vkProxyInternalFreeNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
					AllocatorState::of(user_data)->onFree(size);
				}

			public:

				READONLY VkAllocationCallbacks allocator;

				WrappedAllocator(const char* identifier, bool verbose) {
					allocator.pUserData = new AllocatorState (identifier, verbose);
					allocator.pfnAllocation = vkProxyAllocationFunction;
					allocator.pfnFree = vkProxyFreeFunction;
					allocator.pfnReallocation = vkProxyReallocationFunction;
					allocator.pfnInternalAllocation = vkProxyInternalAllocationNotification;
					allocator.pfnInternalFree = vkProxyInternalFreeNotification;
				}

				void print(bool header) {
					if (header) {
						std::cout << AllocatorState::getHeaderString() << "\n";
					}

					std::cout << AllocatorState::of(allocator.pUserData)->getRowString() << "\n";
				}

		};

		std::mutex mutex;
		std::unordered_map<std::string, WrappedAllocator> allocators;

		void printAllocators() {
			bool first = true;

			for (auto& [key, allocator] : allocators) {
				allocator.print(first);
				first = false;
			}
		}

		WrappedAllocator& getAllocator(const char* identifier, bool verbose) {
			std::lock_guard lock {mutex};
			return allocators.try_emplace(identifier, identifier, verbose).first->second;
		}

		static AllocatorCallbackFactory& getFactory() {
			static AllocatorCallbackFactory factory;
			return factory;
		}
		#endif

	public:

		static VkAllocationCallbacks* named(const char* identifier, bool verbose = false) {
			#if !defined(NDEBUG)
			return &getFactory().getAllocator(identifier, verbose).allocator;
			#endif

			return nullptr;
		}

		static void print() {
			#if !defined(NDEBUG)
			getFactory().printAllocators();
			#endif
		}

};