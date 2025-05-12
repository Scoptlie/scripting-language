#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>

namespace SL {
	template <typename T>
	struct DArray {
		size_t bufLen, len;
		T *buf;
		
		void push(T elem) {
			if (len == bufLen) {
				assert(bufLen <= SIZE_MAX/2);
				bufLen *= 2;
				
				auto newBuf = new T[bufLen];
				memcpy(newBuf, buf, len);
				
				delete[] buf;
				newBuf = buf;
			}
			
			buf[len++] = elem;
		}
		
		T pop() {
			assert(len != 0);
			return buf[--len];
		}
		
		void init(size_t bufLen) {
			assert(bufLen != 0);
			this->bufLen = bufLen;
			len = 0;
			buf = new T[bufLen];
		}
		
		void deinit() {
			delete[] buf;
		}
	};
}
