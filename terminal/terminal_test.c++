#include "terminal_manager.h++"

#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <iostream>

using namespace vanwestco;

std::atomic<bool> bell_flag(false);

class toggle_bell_command : public terminal::command {
public:
    toggle_bell_command(std::atomic<bool>& bell_flag) : bell_flag(bell_flag) {
        
    }
    
    void operator()() {
        std::cout << '\x07' << std::flush;
        bell_flag = !bell_flag;
    }
private:
    std::atomic<bool>& bell_flag;
} toggle_bell(bell_flag);

int main() {
    terminal term;
    term.register_command('b', &toggle_bell);
    term.write_line("Write something: ");
    
    std::string line;
    bool running = true;
    do {
        line = term.read_line();
        
        [[unlikely]] if (line.compare("/exit") == 0) {
            running = false;
        } else {
            term.write_line(std::string("name: ") += line);
            if (bell_flag) {
                std::cout << '\x07' << std::flush;
            }
        }
    } while (running);
    
    /*
    weird_print.join();
    */
    
    return 0;
}
