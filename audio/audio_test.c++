#include "core_audio.h++"

#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        vanwestco::Audio_Handle handle;
        constexpr const int seconds = 10;
        constexpr const int frames_per_second =
                vanwestco::Audio_Handle::sample_rate
              / vanwestco::Audio_Handle::frames_per_buffer;
        
        /* play for 10 seconds */
        if (argc == 1) {
            for (int i = 0; i < frames_per_second * seconds; ++i) {
                try {
                    vanwestco::Audio_Handle::Block_t block
                            = handle.record_block();
                    handle.play_block(block);
                } catch (std::exception& exc) {
                    std::cout << exc.what() << std::endl;
                }
            }
         } else if (argc == 2) {
            for (int i = 0; i < 100; ++i) {
                auto block = handle.record_block();
                for (auto n : block.channel()) {
                    std::cout << n << ' ';
                }
                std::cout << std::endl;
            }
         }
    } catch (std::exception& exc) {
        std::cout << exc.what() << std::endl;
    }
    
    return 0;
}
