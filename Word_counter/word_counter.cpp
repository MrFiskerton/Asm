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
        if(str[i] != ' ' && last_is_space) {
            result++;
            last_is_space = false;
        } else if(str[i] == ' ') {
            last_is_space = true;
        }
    }

    return result;
}

size_t naive_word_counter (const std::string &str) {
	return naive_word_counter(str.c_str(), str.length());
}


static __m128i space_mask_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);

size_t asm_word_counter (const char* str, size_t size) {
    if(size <= 64) { 											//Not to put a lot of crutches when checking size all the time
        return naive_word_counter(str, size);
    }

    size_t offset = 0;
    size_t result = 0;
    const size_t step = 16;

    bool last_is_space = false;									//Aligning src pointer to multiplicity of 16
    while ((size_t) (str + offset) % step != 0) {
        char c = *(str + offset);
        if(last_is_space && c != ' ') result++;
        last_is_space = (c == ' ');
        offset++;
    }
    										
    if ( (last_is_space && *(str + offset) != ' ' && offset != 0) || (offset == 0 && *str != ' ')) result++; // Check if next is not a space
    if (offset != 0 && *str != ' ') result++;					// Check if it was space char at the begining

    															// Using vectorization, every step is 16 bytes
    __m128i store = _mm_set_epi32(0, 0, 0, 0); 					// Variable is used to store bytes got in `for` loop
    __m128i cur_cmp, next_cmp; 									// Variables store current 16 bytes and next 16 bytes respectively after comparing them with space mask

    size_t rest = size - (size - offset) % step - step; 		// Getting the end of aligned by 16 addresses space
    
    __m128i tmp;												// Get first 16 bytes of string from memory and compare them with space mask
    __asm__("movdqa\t" "(%2), %1\n"
            "pcmpeqb\t" "%1, %0"
            :"=x"(next_cmp), "=x"(tmp)
            :"r"(str + offset), "0"(space_mask_reg));
    
    for(size_t i = offset; i < rest; i += step) {
        cur_cmp = next_cmp;

        uint32_t msk;

        __m128i tmp04, tmp05, tmp06, tmp07; 					
        __asm__("movdqa\t" "(%7), %3\n"
                "pcmpeqb\t" "%3, %0\n"
                "movdqa\t" "%0, %6\n"
                "palignr\t" "$1, %4, %6\n"
                "pandn\t" "%4, %6\n"
                "psubsb\t" "%6, %5\n"
                "paddusb\t" "%5, %1\n"
                "pmovmskb\t" "%1, %2"
                :"=x"(next_cmp), "=x"(store), "=r"(msk), "=x"(tmp04), "=x"(tmp05), "=x"(tmp06), "=x"(tmp07)
                :"r"(str + i + step), "0"(space_mask_reg), "4"(cur_cmp), "5"(_mm_set_epi32(0, 0, 0, 0)), "1"(store));  

        														
        if(msk != 0 || i + step >= rest) {		// if at least one byte in store is more than 127 or if it is last iteration of loop
            __m128i tmp1, tmp2; 								
            uint32_t high, low;					// Splitting 16-bytes register on 2 8-bytes registers. (To get then minor and major parts)
            __asm__("psadbw\t" "%3, %0\n"
                    "movd\t" "%0, %2\n"
                    "movhlps\t" "%0, %0\n"
                    "movd\t" "%0, %1\n"
                    :"=x" (tmp1), "=r"(high), "=r"(low), "=x"(tmp2)
                    :"0"(_mm_set_epi32(0, 0, 0, 0)), "3"(store));

            result += high + low;
            store = _mm_set_epi32(0, 0, 0, 0);
        }
    }
    offset = rest; 								// Moving to the end of aligned block

    // Check if previous char was space but current is a new word. If not to do this, last word will be counted twice
    if(*(str + offset - 1) == ' ' && *(str + offset) != ' ') result--;
    
    // Counting words in not aligned rest of string
    last_is_space = *(str + offset - 1) == ' ';
    for(size_t i = offset; i < size; i++) {
        if (*(str + i) != ' ' && last_is_space) result++;
        last_is_space = *(str + i) == ' ';
    }

    return result;
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
		test_str = random_string_generator(1e7, 1e8, 8);

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