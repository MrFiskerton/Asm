#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <chrono>
#include <random>
#include <emmintrin.h>
#include <immintrin.h>


size_t naive_word_counter(const char* str, size_t size) {
    uint32_t result = 0;

    bool last_is_space = true;
    for (size_t i = 0; i < size; i++) {
        if(str[i] != ' ' && last_is_space) result++;
        last_is_space = (str[i] == ' ');
    }

    return result;
}

size_t naive_word_counter (const std::string &str) {
	return naive_word_counter(str.c_str(), str.length());
}


static __m128i space_mask_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);

size_t asm_word_counter (const char* str, size_t size) {
    return 0;
}

size_t asm_word_counter (const std::string &str) {
	return asm_word_counter(str.c_str(), str.length());
}

std::string random_string_generator (size_t min_len = 1, size_t max_len = 10000, size_t number_of_letters_for_one_space = 8) {
	std::cout << "\nStarting generate random string ( size := "; 

	std::stringstream result;
	size_t len;
	
	std::cout << ( len = std::rand() % (max_len - min_len + 1) + min_len ) << " ) ... ";
	std::flush(std::cout);

	bool is_space;
	for(size_t i = 0; i < len; i++) {
		is_space = std::rand() % (number_of_letters_for_one_space + 1) == 0;
		result << char(is_space ? ' ' : 'a' + std::rand() % 26);
	}

	std::cout << "Finish.\n\n";

	return result.str();
}

double call_with_timer (const std::string header, const std::string &arg, size_t ( *test_f )(const std::string&), int &result) {
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed;

	std::cout << header;
	start = std::chrono::high_resolution_clock::now();
	result = test_f(arg);
	end = std::chrono::high_resolution_clock::now();

	double time = (elapsed = end - start).count();
	std::cout << time << std::endl;

	return time;
}

bool test_word_counter(int N = 100) {
	std::string test_str;
	double naive_time(0), asm_time(0);
	int naive_result, asm_result;

	while (N--) {
		test_str = random_string_generator(1e1, 1e3, 8);

		naive_time += call_with_timer("Naive time: ", test_str, naive_word_counter, naive_result);	
		asm_time   += call_with_timer("Asm time:   ", test_str, asm_word_counter,   asm_result);

		if (naive_result != asm_result) {
			std::cerr << " >>> There is an error. Number of words not equal.\nTest := " << test_str << std::endl;
			std::cerr << "\ntest_size    := " << test_str.length() << " chars"
					  << "\nnaive_result := " << naive_result
					  << "\nasm_result   := " << asm_result
					  << std::endl;
			return false;
		}
	}		

	std::cout << "-------------------------------------------------\n"
			  << " >>> Boost: " << naive_time / asm_time 
			  << std::endl;
	return true;
}

int main () {
	std::srand(std::time(0));
	test_word_counter(3);
	return 0;
}
