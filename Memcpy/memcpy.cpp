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

	const size_t step = 16;

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
        std::cout << "Size:  " << size << " int" << std::endl;

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

void test_memcpy_large (int N = 100) {
	std::cout << "\n Large test\n";

	srand(time(0));
	std::chrono::time_point<std::chrono::high_resolution_clock> start, finish;
	std::chrono::duration<double> elapsed;

	int size = 1 << 28;
	char* src_str = new char[size];
	char* dst_str = new char[size];

	std::cout << "Size:  " << size << " byte * " << N << std::endl;
	for (int i = 0; i < size - 1; i++) { src_str[i] = 'a' + std::rand() % 26;}
	src_str[size - 1] = '\0';	
		
	
	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i != N; i++)
		asm_memcpy(dst_str, src_str, size);
	finish = std::chrono::high_resolution_clock::now();

	elapsed = finish - start;
	double asm_time = elapsed.count();
	for (int i = 0; i < size; ++i) { assert(src_str[i] == dst_str[i]);}
	std::cout << "Asm:   " << asm_time << std::endl;	


//----------------------------------------------------------------------------------
	for (int i = 0; i < size - 1; i++) { src_str[i] = 'a' + std::rand() % 26;}
	src_str[size - 1] = '\0';	

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i != N; i++)
		naive_memcpy(dst_str, src_str, size);
	finish = std::chrono::high_resolution_clock::now();

	elapsed = finish - start;
	double naive_time = elapsed.count();
	for (int i = 0; i < size; ++i) { assert(src_str[i] == dst_str[i]);}
	std::cout << "Naive: " << naive_time << std::endl;


	std::cout << "Boost: " << naive_time / asm_time << std::endl;	

	delete[] src_str;
	delete[] dst_str;
}

int main () {
	test_memcpy(1, 1000000);
	test_memcpy_large();
	return 0;
}
