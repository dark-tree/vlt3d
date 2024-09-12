
#include <vstl.hpp>
#include "util/math/bits.hpp"
#include "util/collection/pyramid.hpp"
#include "util/exception.hpp"
#include "buffer/array.hpp"
#include "util/arena.hpp"
#include "util/collection/ring.hpp"
#include "util/util.hpp"
#include "util/thread/delegator.hpp"

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

TEST(util_bits_msb_mask) {

	CHECK(Bits::msbMask<uint8_t>(0b0110'0011), 0b0100'0000);
	CHECK(Bits::msbMask<uint8_t>(0b0001'0011), 0b0001'0000);
	CHECK(Bits::msbMask<uint8_t>(0b0000'0010), 0b0000'0010);
	CHECK(Bits::msbMask<uint8_t>(0b0000'0001), 0b0000'0001); // special case, lowest bit
	CHECK(Bits::msbMask<uint8_t>(0b1000'0000), 0b1000'0000); // special case, highest bit
	CHECK(Bits::msbMask<uint8_t>(0b0000'0000), 0b0000'0000); // special case, 0

	// 2 BYTES
	CHECK(Bits::msbMask<uint16_t>(0b0000'0010'0010'0100), 0b0000'0010'0000'0000);
	CHECK(Bits::msbMask<uint16_t>(0b0000'0000'0000'0001), 0b0000'0000'0000'0001);
	CHECK(Bits::msbMask<uint16_t>(0b1000'0000'0000'0000), 0b1000'0000'0000'0000);
	CHECK(Bits::msbMask<uint16_t>(0b0000'0000'0000'0000), 0b0000'0000'0000'0000);

	// 4 BYTES
	CHECK(Bits::msbMask<uint32_t>(0x0000'0180), 0x0000'0100);
	CHECK(Bits::msbMask<uint32_t>(0x01F0'0030), 0x0100'0000);
	CHECK(Bits::msbMask<uint32_t>(0xFFFF'FFFF), 0x8000'0000);
	CHECK(Bits::msbMask<uint32_t>(0x0000'0000), 0x0000'0000);

};

TEST(util_bits_msb_index) {

	CHECK(Bits::msbIndex<uint8_t>(0b0110'0011), 6);
	CHECK(Bits::msbIndex<uint8_t>(0b0001'0011), 4);
	CHECK(Bits::msbIndex<uint8_t>(0b0000'0010), 1);
	CHECK(Bits::msbIndex<uint8_t>(0b0000'0001), 0); // special case, lowest bit
	CHECK(Bits::msbIndex<uint8_t>(0b1000'0000), 7); // special case, highest bit

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

	LoggerLock lock {Logger::FATAL | Logger::ERROR | Logger::WARN};

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

TEST(image_managed_mipmaps) {

	ManagedImageDataSet set {8, 8, 4, true};
	set.resize(3, 5); // (8 * 3, 8 * 5)

	CHECK(set.levels(), 4);
	CHECK(set.level(0).width(), 8 * 3);
	CHECK(set.level(0).height(), 8 * 5);
	CHECK(set.level(1).width(), 4 * 3);
	CHECK(set.level(1).height(), 4 * 5);
	CHECK(set.level(2).width(), 2 * 3);
	CHECK(set.level(2).height(), 2 * 5);
	CHECK(set.level(3).width(), 1 * 3);
	CHECK(set.level(3).height(), 1 * 5);

	set.level(0).clear({200, 0, 0, 255});
	set.level(1).clear({150, 50, 0, 255});
	set.level(2).clear({50, 150, 0, 255});
	set.level(3).clear({0, 200, 0, 255});

	ImageData smooth = ImageData::allocate(8, 8);
	ImageData solid = ImageData::allocate(8, 8);

	for (int y = 0; y < 8; y ++) {
		for (int x = 0; x < 8; x ++) {
			smooth.pixel(x, y)[0] = x * 32;
			smooth.pixel(x, y)[1] = y * 32;
			smooth.pixel(x, y)[2] = ((x ^ y) & 0b111) * 32;
			smooth.pixel(x, y)[3] = 255;
		}
	}

	for (int y = 0; y < 8; y ++) {
		for (int x = 0; x < 8; x ++) {
			solid.pixel(x, y)[0] = (x < 4) != (y < 4) ? 220 : 0;
			solid.pixel(x, y)[1] = (x < 4) != (y < 4) ? 220 : 0;
			solid.pixel(x, y)[2] = 0;
			solid.pixel(x, y)[3] = 255;
		}
	}

	set.blit(0 * 8, 0 * 8, smooth, ImageScaling::NEAREST);
	set.blit(2 * 8, 1 * 8, smooth, ImageScaling::NEAREST);
	set.blit(2 * 8, 4 * 8, smooth, ImageScaling::NEAREST);
	set.blit(1 * 8, 2 * 8, solid, ImageScaling::NEAREST);
	set.blit(2 * 8, 3 * 8, solid, ImageScaling::NEAREST);

	uint8_t* p0 = set.level(1).pixel(1 * 4 - 1, 2 * 4 - 1);
	uint8_t* p1 = set.level(1).pixel(1 * 4 + 2, 2 * 4 + 0);

	CHECK(p0[0], 150);
	CHECK(p0[1], 50);
	CHECK(p0[2], 0);

	CHECK(p1[0], 220);
	CHECK(p1[1], 220);
	CHECK(p1[2], 0);

	ImageData collage = set.unified();
	collage.save("testing/image_managed_mipmaps.png");
	collage.close();

	set.close();

	CHECK(set.levels(), 0);

}

TEST(image_managed_layers) {

	ManagedImageDataSet set {4, 4, 4, false};
	set.resize(2, 2);

	CHECK(set.levels(), 1);
	CHECK(set.level(0).width(), 8);
	CHECK(set.level(0).height(), 8);

	ImageData a = ImageData::allocate(8, 8);
	ImageData b = ImageData::allocate(8, 8);

	a.clear({0, 0, 0, 255});
	b.clear({0, 0, 0, 255});

	for (int y = 0; y < 8; y ++) {
		for (int x = 0; x < 8; x ++) {
			a.pixel(x, y)[0] = (x < 4) != (y < 4) ? 220 : 0;
			a.pixel(x, y)[1] = (x < 4) != (y < 4) ? 220 : 0;
		}
	}

	for (int y = 0; y < 8; y ++) {
		for (int x = 0; x < 8; x ++) {
			b.pixel(x, y)[0] = (x < 4) != (y < 4) ? 220 : 0;
			b.pixel(x, y)[2] = (x < 4) != (y < 4) ? 220 : 0;
		}
	}

	set.addLayer(a, ImageScaling::NEAREST);
	a.close();

	EXPECT_ANY({
		set.resize(2, 2);
	});

	set.addLayer(b, ImageScaling::NEAREST);
	b.close();

	set.save("testing/image_managed_layers.png");

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

	LoggerLock lock {Logger::FATAL | Logger::ERROR};

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

//	ASSERT(array.getImage().level(0).width() == 8);
//	ASSERT(array.getImage().level(0).height() >= (8*3));
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

TEST(util_arena_linear_brick) {

	LinearArena::Brick<uint8_t> brick;

	std::vector<int> offsets;

	CHECK(brick.hasAllocated(), false);

	int a = brick.allocate();
	CHECK(brick.top(), a);

	int b = brick.allocate();
	CHECK(brick.top(), std::max(a, b));

	brick.free(a);
	brick.free(b);

	for (int i = 0; i < 8; i ++) {
		CHECK(brick.hasFree(), true);
		int value = brick.allocate();

		ASSERT(value >= 0);

		if (util::contains(offsets, value)) {
			FAIL("LinearArena::Brick allocation error");
		}

		offsets.push_back(value);
	}

	CHECK(brick.hasFree(), false);
	CHECK(brick.hasAllocated(), true);

	brick.free(3);
	CHECK(brick.allocate(), 3);
	CHECK(brick.hasFree(), false);
	CHECK(brick.hasAllocated(), true);

}

TEST(util_arena_linear_block) {

	LinearArena::Block<uint8_t, 4> block;
	std::vector<int> offsets;

	CHECK(block.remaining(), 4 * 8);

	for (int j = 0; j < 4; j ++) {
		for (int i = 0; i < 8; i++) {
			int offset = block.allocate();
			offsets.push_back(offset);
			CHECK(offset & 0b1111'1000, j << 3);
		}
	}

	CHECK(block.remaining(), 0);
	CHECK(block.allocate(), -1);

	for (int offset : offsets) {
		block.free(offset);
	}

	CHECK(block.remaining(), 4 * 8);

	int a = block.allocate();
	CHECK(block.top(), a);
	block.free(a);

	int b = block.allocate();
	CHECK(block.top(), b);
	block.free(b);

	CHECK(a, b);

}

TEST(util_arena_linear_full) {

	LinearArena arena;
	arena.expand();

	{
		const int a = arena.allocate();
		CHECK(arena.top(), a);

		const int b = arena.allocate();
		CHECK(arena.top(), std::max(a, b));

		arena.free(std::max(a, b));
		CHECK(arena.top(), std::min(a, b));

		arena.free(std::min(a, b));
	}

	std::vector<long> ids;

	for (int i = 0; i < 3000; i ++) {
		long id = arena.allocate();

		ASSERT(id >= 0);

		if (util::contains(ids, id)) {
			FAIL("LinearArena allocation error");
		}

		ids.push_back(id);
	}

	for (int i = 0; i < 3000; i ++) {
		arena.free(ids[i]);
	}

	for (int i = 0; i < 3000; i ++) {
		CHECK(ids[i], arena.allocate());
	}

}

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