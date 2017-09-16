#include <cassert>
#include <iostream>
#include <sstream> 
#include <chrono>
#include <random>
#include <immintrin.h>

size_t naive_word_counter (const char* str, size_t size);
size_t naive_word_counter (const std::string &str);
size_t asm_word_counter (const std::string &str);
size_t asm_word_counter (const char* str, size_t size);


size_t naive_word_counter (const char* str, size_t size) {
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

size_t asm_word_counter (const char* str, size_t size) {
    //For not to put a lot of crutches connected with checking size.
    if(size <= 64) {
        return naive_word_counter(str, size);
    }

    size_t offset = 0;
    size_t result = 0;
    constexpr size_t step = 16;

    //Aligning src pointer to multiplicity of step
    bool last_is_space = false;
    while ((size_t) (str + offset) % step != 0) {
        char c = *(str + offset);
        if(last_is_space && c != ' ') result++;
        last_is_space = (c == ' ');
        offset++;
    }
    //Check if it was space char at the begining
    if (offset != 0 && *str != ' ') result++;
    //Check if next is not a space
    if ( (offset == 0 && *str != ' ') || (last_is_space && *(str + offset) != ' ' && offset != 0) ) result++;
    //----------------------------------------------------------------------/*/
    //Getting the end of aligned by step addresses space
    size_t rest = size - (size - offset) % step - step; 
    
    __m128i accumulator = _mm_set1_epi8(0);
    const __m128i wspace_mask = _mm_set1_epi8(' ');

    //Take block of step bytes from given string and compare with a space mask.
    __m128i *first_block = (__m128i *) (str + offset);
    __m128i curr_block = _mm_cmpeq_epi8(_mm_load_si128(first_block), wspace_mask);

    for (size_t i = offset; i < rest; i += step) {
        __m128i prev_block = curr_block;
        //Getting next block of step bytes from given string and compare with a space mask.
        curr_block = _mm_cmpeq_epi8(_mm_load_si128((__m128i*)(str + i + step)), wspace_mask);
        
        __m128i shifted_block = _mm_alignr_epi8(curr_block, prev_block, 1); //Move register to one byte.
        __m128i andnot = _mm_andnot_si128(shifted_block, prev_block);       //Make and-not on shifted block.

        //Get the mask of result as comparence with 0 and then storage it in accumulator register.
        accumulator = _mm_adds_epu8(_mm_and_si128(_mm_set1_epi8(1), andnot), accumulator);

        //Check for ovewflow of accumulator.
        //If at least one byte in store is more than 127 or if it is last iteration of loop.
        if (_mm_movemask_epi8(accumulator) != 0 || i + step >= rest) {
            //Splitting 16-bytes register on 2 8-bytes registers. (To get then minor and major parts)
            accumulator = _mm_sad_epu8(_mm_set1_epi8(0), accumulator);
            result += _mm_cvtsi128_si32(accumulator);     //Getting minor part.

            accumulator = _mm_srli_si128(accumulator, 8); //Moving major part to minor area.
            result += _mm_cvtsi128_si32(accumulator);     //Getting major part.

            accumulator = _mm_set1_epi8(0);
        }
    }
    //----------------------------------------------------------------------/*/    
    // Moving to the end of aligned block
    offset = rest;

    // Check if previous char was space but current is a new word. If not to do this, last word will be counted twice.
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

bool test_word_counter(int N = 100, int max_str_len = (int)1e5) {
    std::string test_str;
    double naive_time(0), asm_time(0);
    int naive_result, asm_result;

    while (N--) {
        test_str = random_string_generator(1e1, max_str_len, 8);

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

int main (int argc, char* argv[]) {
    std::srand(std::time(0));
    
    if (argc == 2 && std::string(argv[1]) == "-stress") {
        test_word_counter(1, (int)1e9);
    } else {
        test_word_counter(3);
    }

    return EXIT_SUCCESS;
}
