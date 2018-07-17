#include "terminal_manager.h++"

#include <iostream>
#include <stdexcept>
#include <utility>

using input_reader = vanwestco::terminal::input_reader;
using display      = vanwestco::terminal::display;
using update_type  = vanwestco::terminal::display::display_update::update_type;

input_reader::input_reader(display& d)
: cursor(0), out(d),
  command_thread([this] {
    bool running = true;
    while (running) {
        /* wait for a command to be available */
        char next_key = command_queue.wait_front();
        
        if (next_key == '\x00') { /* should stop command operation */
            running = false;
        } else {
            /* get the command */
            vanwestco::terminal::command* next; {
                std::lock_guard l(command_access_lock);
                next = commands.at(next_key);
            }
            
            /* pop it out of the queue */
            command_queue.pop();
            
            /* actually run it */
            (*next)();
        }
    }
  }) {

}

input_reader::~input_reader() {
    /* send a stop message to the command thread */
    command_queue.push('\x00');
    
    /* wait for it to finish (to avoid std::terminate()) */
    command_thread.join();
}

std::string input_reader::get_line() {
    /* ensure thread exclusion here */
    std::lock_guard<std::mutex> get_line_exclude(get_line_lock);
    
    bool reading = true;
    std::string next_line;
    
    while (reading) {
        update_type next_update = update_type::NO_UPDATE;
        char next = std::cin.get();
        bool newline = false;
        
        switch (next) {
        case '\n': /* new line */
            newline = true;
            next_update = update_type::INPUT_LINE;
            break;
        case '\x09':
        case '\x7F': /* backspace/delete */
            if (cursor != 0 && input_line.size() > 0) {
                /* remove character */
                input_line.erase(input_line.begin() + --cursor);
                next_update = update_type::INPUT_LINE;
            }
            break;
        case '\x1B': /* special controls */
            if (std::cin.get() == '[') {
                char key = std::cin.get();
                switch (key) {
             /* add letter for ANSI escape code here:
                case 'A': // ... */
                }
            }
            break;
        /* line navigation commands: */
        case 'f' - 96: /* cursor forward */
            if (cursor < input_line.size()) {
                ++cursor;
            }
            next_update = update_type::CURSOR_POS;
            break;
        case 'b' - 96: /* cursor back */
            if (cursor > 0) {
                --cursor;
            }
            next_update = update_type::CURSOR_POS;
            break;
        case 'a' - 96: /* beginning of line */
            cursor = 0;
            next_update = update_type::CURSOR_POS;
            break;
        case 'e' - 96: /* end of line */
            cursor = input_line.size();
            next_update = update_type::CURSOR_POS;
            break;
        default:
            if ((1 <= next) && (next <= 26)) { /* control character */
                char control = next + 96;
                
                /* check if key is registered */
                bool registered; {
                    std::lock_guard l(command_access_lock);
                    registered = (commands.find(control) != commands.end());
                }
                
                if (registered) { /* push the key pressed into the queue */
                    command_queue.push(control);
                }
            } else if (input_line.size() < MAX_LINE_LENGTH) { /* within range */
                /* add character */
                input_line.insert(input_line.begin() + cursor++, next);
                next_update = update_type::INPUT_LINE;
            }
            break;
        }
        
        /* do display update */
        switch (next_update) {
        case update_type::NO_UPDATE:
            break;
        case update_type::INPUT_LINE: {
            /* necessary because input_line is stored as std::vector */
            char nl_int[input_line.size() + 1];
            int i = 0;
            for (; i < input_line.size(); ++i) {
                nl_int[i] = input_line[i];
            }
            nl_int[i] = 0; /* null terminator */
            
            if (newline) {
                /* send new input line off  */
                next_line = std::string(nl_int);
                cursor = 0;
                input_line.clear();
                reading = false;
                
                /* clear the old input line */
                nl_int[0] = 0;
            }
            
            out.update(display::display_update(update_type::INPUT_LINE,
                                               nl_int, cursor));
            break;
        }
        
        case update_type::CURSOR_POS:
            out.update(display::display_update(update_type::CURSOR_POS,
                                               "", cursor));
            break;
        default:
            break;
        }
    }
    
    return next_line;
}

void input_reader::register_command(char key,
                                    vanwestco::terminal::command* cmd) {
    std::lock_guard l(command_access_lock);
    commands.insert(
        std::make_pair<char, vanwestco::terminal::command*>(std::move(key),
                                                            std::move(cmd)));
}
