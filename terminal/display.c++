#include "terminal_manager.h++"

#include <iostream>
#include <cstring>

using display = vanwestco::terminal::display;

namespace termctl { /* terminal control commands */
static constexpr const char* const csi_base        = "\x1b[";
static constexpr const char* const cursor_column_1 = "\x1b[1G";
static constexpr const char* const clear_line      = "\x1b[2K";

static constexpr const char* const input_prompt    = "\x1b[1m>\x1b[0m ";
static constexpr const char* const more_characters = "\x1b[1m$\x1b[0m";

/* lengths of the printable characters */
static constexpr const int input_prompt_length     = 2;
static constexpr const int more_characters_length  = 1;
} /* ~namespace termctl */

display::display_update::display_update(
        const update_type t, const std::string& l, const int c) 
: type(t), line(l), cursor(c) {
    
}

display::display_update::display_update(
        const update_type t, const char* l, const int c) 
: type(t), line(l), cursor(c) {
    
}

const std::string& display::display_update::get_line() const {
    return line;
}

const display::display_update::update_type& 
        display::display_update::get_type() const {
    return type;
}

const int display::display_update::cursor_pos() const {
    return cursor;
}

/*----------------------------------------------------------------------------*/

display::display(const int width) : display_thread([this] { this->loop(); }) {
    terminal_width = width;
    cursor = 0;
    print_offset = 0;
}

display::~display() {
    this->update(display::display_update(
            display::display_update::update_type::end_of_line, ""));
    display_thread.join();
}

void display::loop() {
    bool running = true;
    sketch_input_line();
    
    while (running) {
        // std::unique_lock<std::mutex> lock(update_sync);
        // 
        // /* check to see if anything's there */
        // if (updates.empty()) {
        //     update_cv.wait(lock, [this] { return !updates.empty(); });
        // }
        // const display_update next = updates.front();
        /* wait for a display update */
        const display_update next = updates.wait_front();
        
        /* do tasks */
        switch (next.get_type()) {
        case display_update::update_type::no_update:
            break;
        case display_update::update_type::input_line:
            /* store the new input line */
            input_line = next.get_line();
            
            [[fallthrough]];
        case display_update::update_type::cursor_pos:
            cursor = next.cursor_pos();
            
            if (print_offset == 0) { /* starting from bare prompt */
                /* shift the text window right */
                if (cursor > terminal_width
                             - termctl::input_prompt_length
                             - termctl::more_characters_length) {
                    print_offset = cursor
                                   - (terminal_width
                                      - termctl::input_prompt_length
                                      - termctl::more_characters_length);
                }
            } else {
                if (cursor < print_offset) {
                    print_offset = cursor;
                } else if (cursor - print_offset 
                           > terminal_width
                             - 2 * termctl::more_characters_length) {
                    print_offset = cursor 
                                   - (terminal_width 
                                      - 2 * termctl::more_characters_length);
                }
            }
            
            sketch_input_line();
            break;
        case display_update::update_type::new_line:
            /* write the new line over the input line, line-feeding, 
               in the bottom left */
            std::cout << termctl::clear_line
                      << termctl::cursor_column_1
                      << next.get_line().c_str()
                      << std::endl;
            sketch_input_line();
            break;
        case display_update::update_type::end_of_line:
            running = false;
            std::cout << termctl::clear_line
                      << termctl::cursor_column_1
                      << std::flush;
            break;
        }
        
        updates.pop();
    }
}

void display::sketch_input_line() {
    int fill_length, left_fill_length; /* number of characters surrounding and
                                          to the left of the message,
                                          respectively */
    
    left_fill_length = print_offset == 0 
                       ? termctl::input_prompt_length
                       : termctl::more_characters_length;
    
    int screen_cursor_pos = cursor 
                            - print_offset     /* character index in message */
                            + left_fill_length /* offset from left-hand side */
                            + 1;               /* terminal text columns are
                                                  indexed from 1 */
    /* prepare screen for write */
    std::cout << termctl::clear_line
              << termctl::cursor_column_1;
    
    if (print_offset == 0) { /* starting at prompt */
        std::cout << termctl::input_prompt;
        
        if (input_line.length() <= terminal_width
                                   - termctl::input_prompt_length
                                   - termctl::more_characters_length) {
            std::cout << input_line;
        } else {
            std::cout << input_line.substr(0, terminal_width 
                                              - termctl::input_prompt_length
                                              - termctl::more_characters_length)
                      << termctl::more_characters;
        }
    } else { /* starting wherever */
        std::cout << termctl::more_characters
                  << input_line.substr(print_offset, 
                                       terminal_width 
                                       - 2 * termctl::more_characters_length);
        if (input_line.length() > terminal_width
                                  - 2 * termctl::more_characters_length
                                  + print_offset) {
            std::cout << termctl::more_characters;
        }
    }
    
    std::cout << termctl::csi_base << screen_cursor_pos << 'G'
              << std::flush;
}

void display::update(display::display_update next) {
    updates.push(next);
}
