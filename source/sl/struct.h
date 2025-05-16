#pragma once

#include "heap.h"
#include "val.h"

namespace SL {
	struct String;
	
	struct Struct : public Object {
		enum EntryState {
			entryStateEmpty,
			entryStateOccupied,
			entryStateTombstone,
		};
		
		struct Entry {
			EntryState state;
			String *key;
			Val val;
		};
		
		size_t nEntries;
		Entry *entries;
		
		// Number of non-empty (occupied or tombstone) entries
		size_t load;
		
		void expand(size_t newNEntries);
		Entry *find(String *key);
		bool get(String *key, Val *oVal);
		void set(String *key, Val val);
		void remove(String *key);
		
		static Struct *create(Heap *heap, size_t nEntries);
		
	};
}
