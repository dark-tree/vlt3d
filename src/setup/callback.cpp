
#include "callback.hpp"
#include "util/logger.hpp"

/*
 * AllocatorState
 */

#if !defined(NDEBUG)
std::string AllocatorCallbackFactory::WrappedAllocator::AllocatorState::pad(std::string str, int padding) {
	str.append(padding - str.length(), ' ');
	return str;
}

std::string AllocatorCallbackFactory::WrappedAllocator::AllocatorState::pad(size_t counter, int padding) {
	return pad(std::to_string(counter), padding);
}

AllocatorCallbackFactory::WrappedAllocator::AllocatorState::AllocatorState(const char* identifier, bool verbose)
: identifier(identifier), verbose(verbose) {}

AllocatorCallbackFactory::WrappedAllocator::AllocatorState* AllocatorCallbackFactory::WrappedAllocator::AllocatorState::of(void* user_data) {
	return static_cast<AllocatorState*>(user_data);
}

std::string AllocatorCallbackFactory::WrappedAllocator::AllocatorState::getHeaderString() {
	return "| Identifier      | Allocations     | Allocated       |\n"
		   "+ --------------- + --------------- + --------------- +";
}

std::string AllocatorCallbackFactory::WrappedAllocator::AllocatorState::getRowString() {
	std::lock_guard lock {mutex};

	long delta_allocations = allocations - frees;
	long delta_allocated = allocated - freed;

	return  "| " + pad(identifier, 15) + " | " + pad(delta_allocations, 15) + " | " + pad(delta_allocated, 15) + " |";
}

void AllocatorCallbackFactory::WrappedAllocator::AllocatorState::onFree(size_t size) {
	std::lock_guard lock {mutex};
	frees ++;
	freed += size;

	if (verbose) {
		logger::debug("[Allocator] ", identifier, " freed ", size, " bytes");
	}
}

void AllocatorCallbackFactory::WrappedAllocator::AllocatorState::onAllocate(size_t size) {
	std::lock_guard lock {mutex};
	allocations ++;
	allocated += size;

	if (verbose) {
		logger::debug("[Allocator] ", identifier, " allocated ", size, " bytes");
	}
}

/*
 * WrappedAllocator
 */

void* AllocatorCallbackFactory::WrappedAllocator::vkProxyAllocationFunction(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope scope) {
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

void AllocatorCallbackFactory::WrappedAllocator::vkProxyFreeFunction(void* user_data, void* pointer) {
	if (pointer == nullptr) {
		return;
	}

	// retrieve our header struct back, remember to free
	// header->root only after we finish using the header
	Header* header = static_cast<Header*>(pointer) - 1;
	vkProxyInternalFreeNotification(user_data, header->size, DUMMY_TYPE, DUMMY_SCOPE);
	free(header->root);
}

void* AllocatorCallbackFactory::WrappedAllocator::vkProxyReallocationFunction(void* user_data, void* pointer, size_t size, size_t alignment, VkSystemAllocationScope scope) {

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

void AllocatorCallbackFactory::WrappedAllocator::vkProxyInternalAllocationNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
	AllocatorState::of(user_data)->onAllocate(size);
}

void AllocatorCallbackFactory::WrappedAllocator::vkProxyInternalFreeNotification(void* user_data, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
	AllocatorState::of(user_data)->onFree(size);
}

AllocatorCallbackFactory::WrappedAllocator::WrappedAllocator(const char* identifier, bool verbose) {
	allocator.pUserData = new AllocatorState (identifier, verbose);
	allocator.pfnAllocation = vkProxyAllocationFunction;
	allocator.pfnFree = vkProxyFreeFunction;
	allocator.pfnReallocation = vkProxyReallocationFunction;
	allocator.pfnInternalAllocation = vkProxyInternalAllocationNotification;
	allocator.pfnInternalFree = vkProxyInternalFreeNotification;
}

void AllocatorCallbackFactory::WrappedAllocator::print(bool header) {
	if (header) {
		std::cout << AllocatorState::getHeaderString() << "\n";
	}

	std::cout << AllocatorState::of(allocator.pUserData)->getRowString() << "\n";
}

/*
 * AllocatorCallbackFactory
 */

void AllocatorCallbackFactory::printAllocators() {
	bool first = true;

	for (auto& [key, allocator] : allocators) {
		allocator.print(first);
		first = false;
	}
}

AllocatorCallbackFactory::WrappedAllocator& AllocatorCallbackFactory::getAllocator(const char* identifier, bool verbose) {
	std::lock_guard lock {mutex};
	return allocators.try_emplace(identifier, identifier, verbose).first->second;
}

AllocatorCallbackFactory& AllocatorCallbackFactory::getFactory() {
	static AllocatorCallbackFactory factory;
	return factory;
}
#endif

VkAllocationCallbacks* AllocatorCallbackFactory::named(const char* identifier, bool verbose) {
	#if !defined(NDEBUG)
	return &getFactory().getAllocator(identifier, verbose).allocator;
	#endif

	return nullptr;
}

void AllocatorCallbackFactory::print() {
	#if !defined(NDEBUG)
	getFactory().printAllocators();
	#endif
}