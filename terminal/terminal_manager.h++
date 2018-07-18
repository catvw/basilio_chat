/**
 * Terminal manager for an odd little chat program; I didn't feel like
 * using ncurses, mostly because I don't know how.
 * 
 * @author Charles Van West
 * @version 0
 */

#ifndef BASILIO_CHAT_TERMINAL_HXX
#define BASILIO_CHAT_TERMINAL_HXX

#include "../sync_queue.t++"

#include <string>
#include <thread>
#include <unordered_map>
#include <termios.h>

namespace vanwestco {

/*---------------------------------------------------------------------*
 \                               terminal                              |
 *---------------------------------------------------------------------*/

/**
 * Represents the terminal to write to.
 * 
 * @version 0
 */
class terminal {
public:
    /*-----------------------------------------------------------------*
     |                              display                            |
     *-----------------------------------------------------------------*/
    /**
     * Manages the task of displaying characters on the terminal.
     * 
     * @version 0
     */
    class display {
    public:
        /*-------------------------------------------------------------*
         |                         display_update                      |
         *-------------------------------------------------------------*/
        
        /**
         * Represents a queued display update.
         * 
         * @version 0
         */
        class display_update {   
        public:
            /*---------------------------------------------------------*
             |                        update_type                      |
             *---------------------------------------------------------*/
            
            /**
             * Lists the possible update types.
             * no_update:   don't do anything
             * new_line:    add a new line of dialogue above the input line
             * input_line:  update the input line characters
             * cursor_pos:  update the cursor position
             * end_of_line: program's about to exit
             * 
             * @version 0
             */
            enum class update_type {
                no_update,
                new_line,
                input_line, 
                cursor_pos,
                end_of_line
            };
            
            /*---------------------------------------------------------*
             |                 display_update functions                |
             *---------------------------------------------------------*/
            
            /**
             * Constructs a display update with the given elements. (string)
             * 
             * @param type the update type
             * @param line the new text
             * @param cursor the cursor position (-1 by default)
             */
            display_update(const update_type type, 
                           const std::string& line, 
                           const int cursor = -1);
            
            /**
             * Constructs a display update with the given elements. (char*)
             * 
             * @param type the update type
             * @param line the new text
             * @param cursor the cursor position (-1 by default)
             */
            display_update(const update_type type, 
                           const char* line,
                           const int cursor = -1);
            
            /**
             * Returns the update type.
             * 
             * @return the type
             */
            const update_type& get_type() const;
            
            /**
             * Returns the contained text.
             * 
             * @return the line
             */
            const std::string& get_line() const;
            
            /**
             * Returns the cursor position, or -1 if not set (probably).
             * 
             * @return the cursor position
             */
            const int cursor_pos() const;
       private:
            update_type type;
            std::string line;
            int cursor;
        };
        
        /*-------------------------------------------------------------*
         |                        display functions                    |
         *-------------------------------------------------------------*/
        
        /**
         * Constructs a display object for a terminal of the given width.
         * 
         * @param width width of the terminal in characters
         */
        display(const int width = 80);
        
        /**
         * Destroys the current display. Will not return until the display
         * thread shuts down.
         */
        ~display();
        
        /**
         * Queues an update for the display loop.
         * 
         * @param next the update
         */
        void update(display_update next);
    private:
        /**
         * Runs the main display update loop.
         * 
         */
        void loop();
        
        /**
         * Updates the text input line, using whatever parameters are
         * currently set.
         */
        void sketch_input_line();
        
        std::thread display_thread;
        int terminal_width;
        int print_offset;
        std::string input_line;
        int cursor;
        sync_queue<display_update> updates;
    };
    
    /*-----------------------------------------------------------------*
     |                              command                            |
     *-----------------------------------------------------------------*/
    
    /** Generic command base class.
     * 
     * @version 0
     */
    class command {
    public:
        /**
         * Executes the command.
         */
        virtual void operator()() = 0;
        
        /**
         * Since commands may contain resources, don't allow copying.
         */
        command(command&) = delete;
        
        /**
         * (so the compiler doesn't bother me)
         */
        command() = default;
    };
    
    /*-----------------------------------------------------------------*
     |                            input_reader                         |
     *-----------------------------------------------------------------*/
    
    /**
     * Handles reading lines from cin.
     * 
     * @version 0
     */
    class input_reader {
    public:
        /**
         * Constructs an input reader with the current display to update (as
         * characters appear).
         * 
         * @param d the display
         */
        input_reader(display& d);
        
        /**
         * (only necessary to stop the command dispatch thread)
         */
        ~input_reader();
        
        /**
         * Reads and returns a line from cin, updating the display along the
         * way.
         * 
         * @return the line
         */
        std::string get_line();
        
        /**
         * Registers a command to a given key.
         *  
         * @see terminal::register_command()
         */
         void register_command(char key, terminal::command* cmd);
    private:
        constexpr const static int max_line_length = 1023;
        std::vector<char> input_line;
        int cursor; /* basically an offset in input_line */
        display& out;
        std::mutex get_line_lock; /* for get_line() thread safety */
        
        std::mutex command_access_lock;
        std::unordered_map<char, terminal::command*> commands;
        
        /* stuff for async command callbacks */
        sync_queue<char> command_queue;
        std::thread command_thread;
    };
    
    /*-----------------------------------------------------------------*
     |                         terminal functions                      |
     *-----------------------------------------------------------------*/
    
    /**
     * Constructs a terminal object and sets up the terminal for manual
     * control.
     * 
     */
    terminal();
    
    /**
     * Destroys the terminal object, returning the terminal to its previous
     * settings.
     */
    ~terminal();
    
    /**
     * Writes a line to the terminal. (string)
     * 
     * @param line the line to write
     */
    void write_line(const std::string& line);
    
    /**
     * Writes a line to the terminal. (char*)
     * 
     * @param line the line to write
     */
    void write_line(const char* line);
    
    /**
     * Writes an error message to the terminal. (string)
     * 
     * @param line the error to write
     */
    void write_err(const std::string& line);
    
    /**
     * Writes an error message to the terminal. (char*)
     * 
     * @param line the error to write
     */
    void write_err(const char* line);
    
    /**
     * Reads a line from the terminal.
     * 
     * @return the read line
     */
    std::string read_line();
    
    /**
     * Registers a command for a given Control-[key] action. For example,
     * calling register_command('h', &go_on_hold) would cause go_on_hold->() to
     * be invoked whenever the user entered Control-h.
     * 
     * @param key the C-[key] to listen for
     * @param cmd the command to run
     */
    void register_command(char key, command* cmd);
private:
    /**
     * Finds and reports the current width of the terminal. Only called once,
     * during initialization, so don't get your hopes up for auto-resizing.
     * 
     * @param fildes the file descriptor of the terminal
     * 
     * @return the terminal width in characters
     */
    static int get_terminal_width(int fildes = 0);
    
    struct termios t_settings;
    struct termios t_original;
    
    input_reader in;
    display out;
};

} /* ~namespace vanwestco */

#endif
