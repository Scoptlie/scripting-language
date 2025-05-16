#include "struct.h"

#include "string.h"

namespace SL {
	void Struct::expand(size_t newNEntries) {
		auto newEntries = new Entry[newNEntries];
		for (auto i = size_t(0); i < newNEntries; i++) {
			newEntries[i].state = entryStateEmpty;
		}
		
		auto newLoad = size_t(0);
		for (auto i = size_t(0); i < nEntries; i++) {
			auto entry = &entries[i];
			if (entry->state == entryStateOccupied) {
				newLoad++;
				auto idx = entry->key->hash() & (newNEntries - 1);
				for (;;) {
					auto newEntry = &newEntries[idx];
					if (newEntry->state == entryStateEmpty) {
						newEntry->state = entryStateOccupied;
						newEntry->key = entry->key;
						newEntry->val = entry->val;
						break;
					}
					idx = (idx + 1) & (newNEntries - 1);
				}
			}
		}
		
		delete[] entries;
		
		nEntries = newNEntries;
		entries = newEntries;
		load = newLoad;
	}
	
	Struct::Entry *Struct::find(String *key) {
		auto idx = key->hash() & (nEntries - 1);
		auto tombstoneIdx = SIZE_MAX;
		for (;;) {
			auto entry = &entries[idx];
			if (entry->state == entryStateTombstone) {
				tombstoneIdx = idx;
			} else if (entry->state == entryStateEmpty) {
				if (tombstoneIdx != SIZE_MAX) {
					return &entries[tombstoneIdx];
				} else {
					return entry;
				}
			} else if (entry->state == entryStateOccupied &&
				entry->key->isEqual(key)) {
				
				return entry;
			}
			
			idx = (idx + 1) & (nEntries - 1);
		}
	}
	
	bool Struct::get(String *key, Val *oVal) {
		auto entry = find(key);
		if (entry->state == entryStateOccupied) {
			*oVal = entry->val;
			return true;
		} else {
			return false;
		}
	}
	
	void Struct::set(String *key, Val val) {
		// If load > (nEntries * 0.6875)
		if (load > (nEntries / 2) + (nEntries / 8) + (nEntries / 16)) {
			expand(nEntries * 2);
		}
		
		auto entry = find(key);
		if (entry->state != entryStateOccupied) {
			entry->state = entryStateOccupied;
			entry->key = key;
			load++;
		}
		entry->val = val;
	}
	
	void Struct::remove(String *key) {
		auto entry = find(key);
		entry->state = entryStateTombstone;
	}
	
	Struct *Struct::create(Heap *heap, size_t nEntries) {
		auto r = (Struct*)heap->createObject(sizeof(Struct), objectTypeStruct);
		r->nEntries = nEntries;
		r->entries = new Entry[nEntries];
		r->load = 0;
		
		for (auto i = size_t(0); i < nEntries; i++) {
			r->entries[i].state = entryStateEmpty;
		}
		
		return r;
	}
}
