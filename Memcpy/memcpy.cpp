#include <iostream>
#include <ctime>
#include <cassert>
#include <emmintrin.h>
#include <chrono>

void* naive_memcpy (void *dst, void const *src, size_t size) {
	if (dst == NULL || src == NULL) return NULL;

 	char* dst8 = (char*)dst;
    char* src8 = (char*)src;

    while (size--) { *dst8++ = *src8++; }

	return dst;
}

void* asm_memcpy (void *dst, void const *src, size_t size) {
	if (dst == NULL || src == NULL) return NULL;

	if (size <= 1024 * sizeof (__m128i) / 8) {
		return naive_memcpy(dst, src, size);
	}

	const size_t step = sizeof(size_t) * 2; // 8 or 16

	char* dst8 = (char*)dst;
    char* src8 = (char*)src;

    while ((size_t) dst8 % step != 0 && size--) { *dst8++ = *src8++; }

    while (size >= step) {
    	__m128i reg = _mm_loadu_si128((__m128i *) (src8));
        _mm_stream_si128((__m128i *) (dst8), reg);

        size -= step;
        dst8 += step, src8 += step;
    }

    _mm_sfence();

    while (size--) { *dst8++ = *src8++; }

	return dst8;
}

void test_memcpy (size_t N = 100, int range = 100000) {
	srand(time(0));
	while (N--) {
		int size = rand() % range + 1;
        int a[size], b[size];

        std::cout << "---------------------------" << std::endl;
        std::cout << "Size:  " << size << std::endl;

        for (auto &t : a) { t = rand();}
	
		std::chrono::time_point<std::chrono::high_resolution_clock> start, finish;
		std::chrono::duration<double> elapsed;

    	start = std::chrono::high_resolution_clock::now();
    	naive_memcpy(b, a, sizeof(a));
    	finish = std::chrono::high_resolution_clock::now();

    	elapsed = finish - start;
    	double naive_time = elapsed.count();

		std::cout << "Naive: " << naive_time << std::endl;
		for (int i = 0; i < size; ++i) { assert(a[i] == b[i]);}	
        	
        for (auto &t : b) { t = rand();}

    	start = std::chrono::high_resolution_clock::now();
    	asm_memcpy(b, a, sizeof(a));
    	finish = std::chrono::high_resolution_clock::now();

    	elapsed = finish - start;
    	double asm_time = elapsed.count();
  
		std::cout << "Asm:   " << asm_time << std::endl;
		for (int i = 0; i < size; ++i) { assert(a[i] == b[i]);}

		std::cout << "Boost: " << naive_time / asm_time << std::endl;
	}
}

int main () {
	test_memcpy(1, 1000000);
	return 0;
}
