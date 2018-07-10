#include "terminal_manager.h++"

#include <iostream>
#include <cstring>

using display = vanwestco::terminal::display;

namespace {
const char* const CSI_BASE        = "\x1b[";
const char* const CURSOR_COLUMN_1 = "\x1b[1G";
const char* const CLEAR_LINE      = "\x1b[2K";

const char* const INPUT_PROMPT    = "\x1b[1m>\x1b[0m ";
const char* const MORE_CHARACTERS = "\x1b[1m$\x1b[0m";

const int INPUT_PROMPT_LENGTH    = 2; /* length of the printable characters */
const int MORE_CHARACTERS_LENGTH = 1;
}


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
            display::display_update::update_type::END_OF_LINE, ""));
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
        case display_update::update_type::NO_UPDATE:
            break;
        case display_update::update_type::INPUT_LINE:
            /* store the new input line */
            input_line = next.get_line();
            
            [[fallthrough]];
        case display_update::update_type::CURSOR_POS:
            cursor = next.cursor_pos();
            
            if (print_offset == 0) { /* starting from bare prompt */
                /* shift the text window right */
                if (cursor > terminal_width
                             - INPUT_PROMPT_LENGTH
                             - MORE_CHARACTERS_LENGTH) {
                    print_offset = cursor
                                   - (terminal_width
                                      - INPUT_PROMPT_LENGTH
                                      - MORE_CHARACTERS_LENGTH);
                }
            } else {
                if (cursor < print_offset) {
                    print_offset = cursor;
                } else if (cursor - print_offset 
                           > terminal_width - 2 * MORE_CHARACTERS_LENGTH) {
                    print_offset = cursor 
                                   - (terminal_width 
                                      - 2 * MORE_CHARACTERS_LENGTH);
                }
            }
            
            sketch_input_line();
            break;
        case display_update::update_type::NEW_LINE:
            /* write the new line over the input line, line-feeding, 
               in the bottom left */
            std::cout << CLEAR_LINE
                      << CURSOR_COLUMN_1
                      << next.get_line().c_str()
                      << std::endl;
            sketch_input_line();
            break;
        case display_update::update_type::END_OF_LINE:
            running = false;
            std::cout << CLEAR_LINE 
                      << CURSOR_COLUMN_1
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
                       ? INPUT_PROMPT_LENGTH
                       : MORE_CHARACTERS_LENGTH;
    
    int screen_cursor_pos = cursor 
                            - print_offset     /* character index in message */
                            + left_fill_length /* offset from left-hand side */
                            + 1;               /* terminal text columns are
                                                  indexed from 1 */
    /* prepare screen for write */
    std::cout << CLEAR_LINE
              << CURSOR_COLUMN_1;
    
    if (print_offset == 0) { /* starting at prompt */
        std::cout << INPUT_PROMPT;
        
        if (input_line.length() <= terminal_width
                                   - INPUT_PROMPT_LENGTH
                                   - MORE_CHARACTERS_LENGTH) {
            std::cout << input_line;
        } else {
            std::cout << input_line.substr(0, terminal_width 
                                              - INPUT_PROMPT_LENGTH 
                                              - MORE_CHARACTERS_LENGTH)
                      << MORE_CHARACTERS;
        }
    } else { /* starting wherever */
        std::cout << MORE_CHARACTERS
                  << input_line.substr(print_offset, 
                                       terminal_width 
                                       - 2 * MORE_CHARACTERS_LENGTH);
        if (input_line.length() > terminal_width
                                  - 2 * MORE_CHARACTERS_LENGTH
                                  + print_offset) {
            std::cout << MORE_CHARACTERS;
        }
    }
    
    std::cout << CSI_BASE << screen_cursor_pos << 'G'
              << std::flush;
}

void display::update(display::display_update next) {
    updates.push(next);
}
