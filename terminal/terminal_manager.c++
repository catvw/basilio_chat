#include "terminal_manager.h++"

#include <sys/ioctl.h>

using terminal = vanwestco::terminal;

terminal::terminal() 
: t_settings(), t_original(), out(terminal::get_terminal_width()), in(out) {
    /* get current terminal's attributes */
    tcgetattr(0, &t_settings);
    tcgetattr(0, &t_original);

    t_settings.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON);
    t_settings.c_cc[VMIN] = 1;
    t_settings.c_cc[VTIME] = 0;
    
    tcsetattr(0, TCSANOW, &t_settings);
}

terminal::~terminal() {
    tcsetattr(0, TCSANOW, &t_original);
}

void terminal::write_line(const std::string& line) {
    out.update(display::display_update(
            display::display_update::update_type::NEW_LINE, line));
}

void terminal::write_line(const char* line) {
    write_line(std::string(line));
}

void terminal::write_err(const std::string& line) {
    out.update(display::display_update(
            display::display_update::update_type::NEW_LINE,
            std::string("\x1b[31m").append(line).append("\x1b[0m")));
            /* XXX, should probably move red handling into display */
}

void terminal::write_err(const char* line) {
    write_line(std::string(line));
}

std::string terminal::read_line() {
    return in.get_line();
}

void terminal::register_command(char key, terminal::command* cmd) {
    in.register_command(key, cmd);
}

/*----------------------------------------------------------------------------*/

int terminal::get_terminal_width(int fildes) {
    struct winsize window;
    ioctl(fildes, TIOCGWINSZ, &window);
    return window.ws_col;
}
