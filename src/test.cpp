
#include <vstl.hpp>
#include "util/bits.hpp"
#include "util/pyramid.hpp"
#include "util/exception.hpp"
#include "buffer/array.hpp"
#include "util/arena.hpp"
#include "util/ring.hpp"
BEGIN(VSTL_MODE_LENIENT)

HANDLER { CATCH_PTR (Exception& exception) {
	exception.print();
	FAIL(exception.what())
} }

#define SYNCHRONIZED() \
	static std::atomic<bool> __vstl_running {false}; \
	if (__vstl_running.exchange(true)) { FAIL("Multiple threads detected in synchronized function!"); } \
	struct VstlGuard { ~VstlGuard() { __vstl_running = false; } } __vstl_guard;

TEST(util_bits_width) {

	// unsigned
	CHECK(Bits::width<uint8_t>(), 8);
	CHECK(Bits::width<uint16_t>(), 16);
	CHECK(Bits::width<uint32_t>(), 32);

	// signed
	CHECK(Bits::width<int8_t>(), 8);
	CHECK(Bits::width<int16_t>(), 16);
	CHECK(Bits::width<int32_t>(), 32);

};

TEST(util_bits_msb) {

	CHECK(Bits::msb<uint8_t>(0b0110'0011), 0b0100'0000);
	CHECK(Bits::msb<uint8_t>(0b0001'0011), 0b0001'0000);
	CHECK(Bits::msb<uint8_t>(0b0000'0010), 0b0000'0010);
	CHECK(Bits::msb<uint8_t>(0b0000'0001), 0b0000'0001); // special case, lowest bit
	CHECK(Bits::msb<uint8_t>(0b1000'0000), 0b1000'0000); // special case, highest bit
	CHECK(Bits::msb<uint8_t>(0b0000'0000), 0b0000'0000); // special case, 0

	// 2 BYTES
	CHECK(Bits::msb<uint16_t>(0b0000'0010'0010'0100), 0b0000'0010'0000'0000);
	CHECK(Bits::msb<uint16_t>(0b0000'0000'0000'0001), 0b0000'0000'0000'0001);
	CHECK(Bits::msb<uint16_t>(0b1000'0000'0000'0000), 0b1000'0000'0000'0000);
	CHECK(Bits::msb<uint16_t>(0b0000'0000'0000'0000), 0b0000'0000'0000'0000);

	// 4 BYTES
	CHECK(Bits::msb<uint32_t>(0x0000'0180), 0x0000'0100);
	CHECK(Bits::msb<uint32_t>(0x01F0'0030), 0x0100'0000);
	CHECK(Bits::msb<uint32_t>(0xFFFF'FFFF), 0x8000'0000);
	CHECK(Bits::msb<uint32_t>(0x0000'0000), 0x0000'0000);

};

TEST(util_bits_decompose) {

	int c16 = 0;
	int c3 = 0;
	int c1 = 0;
	int c0 = 0;

	for ([[maybe_unused]] uint32_t flag : Bits::decompose<uint32_t>(0)) c0 ++;
	for ([[maybe_unused]] uint32_t flag : Bits::decompose<uint32_t>(1)) c1 ++;
	for ([[maybe_unused]] uint16_t flag : Bits::decompose<uint16_t>(0xFFFF)) c16 ++;
	for ([[maybe_unused]] uint8_t flag : Bits::decompose<uint8_t>(0b01010001)) c3 ++;

	CHECK(c0, 0);
	CHECK(c1, 1);
	CHECK(c3, 3);
	CHECK(c16, 16);

};

TEST(util_threads_queue) {

	std::vector<int> list;
	TaskQueue queue;

	queue.enqueue([&] () {
		list.push_back(1);
	});

	queue.enqueue([&] () {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		list.push_back(11);
	});

	queue.enqueue([&] () {
		list.push_back(111);
	});

	queue.execute();

	queue.enqueue([&] () {
		list.push_back(1111);
	});

	CHECK(list.size(), 3);
	CHECK(list[0], 1);
	CHECK(list[1], 11);
	CHECK(list[2], 111);

	list.clear();
	queue.execute();

	CHECK(list[0], 1111);

};

TEST(util_threads_mailbox) {

	int counter = 0;
	auto main = std::this_thread::get_id();
	const char* message = nullptr;

	auto function = [&] () {
		try {
			SYNCHRONIZED();
			counter++;

			if (main == std::this_thread::get_id()) {
				message = "Delegated function is running on the calling thread!";
			}
		} catch(...) {
			message = "Exception thrown in delegated function!";
		}

	};

	TaskPool pool {4};
	MailboxTaskDelegator delegator {pool};

	delegator.enqueue(function);
	if (message) FAIL(message);

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	for (int i = 0; i < 1024; i ++) {
		delegator.enqueue(function);
		if (message) FAIL(message);
	}

	delegator.wait();
	CHECK(counter, 1025);

};

TEST(util_ring_basic) {

	RingBuffer<int, 5> buffer;

	// 00 00 00 00 00
	buffer.clear(0);
	CHECK(buffer.size(), 0);
	CHECK(buffer.empty(), true);
	CHECK(buffer.newest(), 0);
	CHECK(buffer.oldest(), 0);

	// 11 00 00 00 00
	buffer.insert(11);
	CHECK(buffer.full(), false);
	CHECK(buffer.empty(), false);
	CHECK(buffer.size(), 1);
	CHECK(buffer.at(0), 11);
	CHECK(buffer.newest(), 11);
	CHECK(buffer.oldest(), 0);

	// 11 12 00 00 00
	buffer.insert(12);
	CHECK(buffer.full(), false);
	CHECK(buffer.size(), 2);
	CHECK(buffer.newest(), 12);
	CHECK(buffer.oldest(), 0);

	// 11 12 13 00 00
	buffer.insert(13);
	CHECK(buffer.full(), false);
	CHECK(buffer.size(), 3);
	CHECK(buffer.newest(), 13);
	CHECK(buffer.oldest(), 0);

	// 11 12 13 14 00
	buffer.insert(14);
	CHECK(buffer.full(), false);
	CHECK(buffer.size(), 4);
	CHECK(buffer.newest(), 14);
	CHECK(buffer.oldest(), 0);

	// 11 12 13 14 15
	buffer.insert(15);
	CHECK(buffer.full(), true);
	CHECK(buffer.size(), 5);
	CHECK(buffer.newest(), 15);
	CHECK(buffer.oldest(), 11);

	// 16 12 13 14 15
	buffer.insert(16);
	CHECK(buffer.full(), true);
	CHECK(buffer.size(), 5);
	CHECK(buffer.newest(), 16);
	CHECK(buffer.oldest(), 12);

	// 16 17 13 14 15
	buffer.insert(17);
	CHECK(buffer.full(), true);
	CHECK(buffer.size(), 5);
	CHECK(buffer.newest(), 17);
	CHECK(buffer.oldest(), 13);

	CHECK(buffer.at(0), 16);
	CHECK(buffer.at(1), 17);
	CHECK(buffer.at(2), 13);
	CHECK(buffer.at(3), 14);
	CHECK(buffer.at(4), 15);

	buffer.clear(1);
	CHECK(buffer.size(), 0);
	CHECK(buffer.empty(), true);
	CHECK(buffer.full(), false);
	CHECK(buffer.at(0), 1);

};

TEST(util_ring_reduce) {

	RingBuffer<int, 3> buffer;

	buffer.clear(0);
	buffer.insert(1);
	buffer.insert(2);
	buffer.insert(3);

	CHECK(std::reduce(buffer.begin(), buffer.end()), 1+2+3);

}

TEST(util_pyramid) {

	Pyramid<int> pyramid;
	ASSERT(pyramid.empty());

	pyramid.push();
	pyramid.append(4);
	pyramid.append(2);
	pyramid.append(1);
	CHECK(pyramid.size(), 1);

	pyramid.push();
	pyramid.append(2);
	pyramid.append(3);
	CHECK(pyramid.size(), 2);
	CHECK(pyramid.top().size(), 2);

	pyramid.push();
	pyramid.append(42);

	Pyramid<int>::View view = pyramid.view();

	std::set<int> level_1 {1, 2, 3, 4, 42};
	std::set<int> level_2 {2, 3, 42};
	std::set<int> level_3 {42};

	CHECK(view.collect(), level_1);
	view.up();
	CHECK(view.collect(), level_2);
	view.up();
	CHECK(view.collect(), level_3);
	view.up();
	ASSERT(view.collect().empty());
	view.up();
	ASSERT(view.collect().empty());

};

TEST(sprite_array) {

	ImageData red = ImageData::allocate(8, 8);
	red.clear({255, 30, 30, 255});

	ImageData green = ImageData::allocate(8, 8);
	green.clear({30, 255, 30, 255});

	ImageData blue = ImageData::allocate(8, 8);
	blue.clear({30, 30, 255, 255});

	SpriteArray array {8, 8};
	array.submitImage("red", red);
	array.submitImage("green", green);
	array.submitImage("blue", blue);

	ASSERT(array.getImage().width() == 8);
	ASSERT(array.getImage().height() >= (8*3));
	ASSERT(array.getSpriteIndex("not-a-real-sprite-1") == 0)
	ASSERT(array.getSpriteIndex("not-a-real-sprite-2") == 0)
	CHECK(array.getSpriteIndex("red"), 0);
	CHECK(array.getSpriteIndex("green"), 1);
	CHECK(array.getSpriteIndex("blue"), 2);

	EXPECT(Exception, {
		uint8_t pixels[10 * 10 * 4];
		ImageData big = ImageData::view(pixels, 10, 10);
		array.submitImage("big", big);
	});

	red.close();
	green.close();
	blue.close();

};

TEST(util_arena) {

	AllocationArena arena {256, 0};

	AllocationBlock* b64_a = arena.allocate(64);
	AllocationBlock* b64_b = arena.allocate(64);
	AllocationBlock* b64_c = arena.allocate(64);
	AllocationBlock* b64_d = arena.allocate(64);
	AllocationBlock* b64_e = arena.allocate(64);

	ASSERT(b64_a != nullptr);
	ASSERT(b64_b != nullptr);
	ASSERT(b64_c != nullptr);
	ASSERT(b64_d != nullptr);
	ASSERT(b64_e == nullptr);

	CHECK(b64_a->getOffset(), 0);
	CHECK(b64_b->getOffset(), 64);
	CHECK(b64_c->getOffset(), 128);
	CHECK(b64_d->getOffset(), 192);

	arena.free(b64_b);

	AllocationBlock* b32_b1 = arena.allocate(32);
	AllocationBlock* b32_b2 = arena.allocate(32);

	ASSERT(b32_b1 != nullptr);
	ASSERT(b32_b2 != nullptr);

	CHECK(b32_b1->getOffset(), 64);
	CHECK(b32_b2->getOffset(), 96);

	// free left block then right
	arena.free(b32_b2);
	arena.free(b64_c);

	AllocationBlock* b96_b2c_1 = arena.allocate(96);
	AllocationBlock* b32_b2c_2 = arena.allocate(32);

	ASSERT(b96_b2c_1 != nullptr);
	ASSERT(b32_b2c_2 == nullptr);

	// free right block then left
	arena.free(b96_b2c_1);
	arena.free(b32_b1);

	AllocationBlock* b128_bc = arena.allocate(128);

	ASSERT(b128_bc != nullptr);

	arena.free(b128_bc);

	// restore original state
	b64_b = arena.allocate(64);
	b64_c = arena.allocate(64);

	ASSERT(b64_b != nullptr);
	ASSERT(b64_c != nullptr);

	// test double consolidation
	arena.free(b64_a);
	arena.free(b64_c);
	arena.free(b64_b);
	arena.free(b64_d);

	AllocationBlock* b256 = arena.allocate(256);

	ASSERT(b256 != nullptr);
	CHECK(b256->getOffset(), 0);

	// say no to memory leaks
	arena.free(b256);
	arena.close();
};