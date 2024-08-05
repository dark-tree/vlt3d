#pragma once

#include "external.hpp"
#include "util/inline.hpp"

class Block {

	public:

		using packed_type = uint64_t;

	public:

		// the next 3 bytes are not yet used for anything
		// first will most likely contain two more 4-bit numbers
		// (temperature? pressure? generic flags? or something else)
		// the next two will most likely represent block-specific local data
		uint8_t reserved[3];

		// the next byte is split into two nibbles,
		// first is the fluid type (similar to the block_type field)
		uint8_t fluid_type : 4;

		// the next nibble is the amount of fluid in this block
		// 0-7 of the given fluid_type, can be used both to represent moistness
		// and standalone water blocks
		uint8_t fluid_amount : 4;

		// this will represent an offset into a list of
		// per chunk pointers to block attachments (per block objects)
		// we need 15 bits for 32x32x32 chunks so one bit of this filed could
		// be used for something else later (maybe a flag to treat the 15 bits
		// as something other than chain offset?)
		uint16_t chain_index;

		// the block type, not much to explain here
		// will map to a global array of per-block functions and
		// configurations, 0 means air or fluid
		uint16_t block_type;

	public:

		FORCE_INLINE packed_type& packed() const {
			return (packed_type&) *this;
		}

		FORCE_INLINE bool operator == (Block other) const {
			return other.packed() == packed();
		}

		FORCE_INLINE bool operator != (Block other) const {
			return other.packed() != packed();
		}

	public:

		FORCE_INLINE explicit Block(packed_type data) {
			packed() = data;
		}

		FORCE_INLINE bool isAir() const {
			// we need to make sure the compiler
			// does a simple `test rax, rax` here
			return packed() == 0;
		}

};

static_assert(sizeof(Block) == sizeof(Block::packed_type), "Invalid size of the world Block class");