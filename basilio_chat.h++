/**
 * A neat little program to communicate with a friend.
 *
 * @author Charles Van West
 * @version 0
 */

#ifndef BASILIO_CHAT_HXX
#define BASILIO_CHAT_HXX

#include "terminal/terminal_manager.h++"
#include "packet.h++"
#include "../socket/Socket.h++"
#include "audio/core_audio.h++"

#include <string>
#include <exception>
#include <atomic>
#include <queue>
#include <memory>

namespace vanwestco {

/**
 * Represents an instantiation of the program.
 *
 */
class basilio_chat {
public:
    /**
     * Constructs a new basilio_chat object and initializes PortAudio through
     * core_audio.h.
     * 
     * @param the server IP address to connect to
     * @param the port to connect to
     * @param the username to use
     * @param whether debug mode is on (one never knows with Basilio)
     * @param whether voice chat is enabled
     * 
     * @throws Audio_Exception
     * @throws Audio_Use_Exception
     */
    basilio_chat(const std::string& address,
                 const std::string& port,
                 const std::string& username,
                 bool debug = false,
                 bool voice = false);
    
    /**
     * Toggles whether a bell character is printed as an alert when a message
     * comes in.
     */
    class bell_toggle_command : public terminal::command {
    public:
        /**
         * Constructs a command instance with an atomic<bool> to toggle.
         *
         * @param flag the bool to toggle
         * @param term the terminal to write notifications to
         */
        bell_toggle_command(std::atomic<bool>& flag, terminal* term);
        
        /**
         * Toggles bell_flag on and off.
         */
        void operator()();
    private:
        std::atomic<bool>& bell_flag;
        terminal* term;
    };
    
    /**
     * Generic exception class for this program.
     */
    class exception : public std::exception {
    public:
        exception(const std::string&);
        const char* what() const noexcept;
    private:
        std::string message;
    };
    
    /**
     * Runs the main bits of the program.
     * 
     * @throws vanwestco::socket::SocketException if something goes wrong with
     *                                            the nextwork (see 
     *                                            ../socket/Socket.hh)
     * @throws vanwestco::basilio_chat::exception if something else happens
     */
    void main();
    
    /**
     * Allows writing out from a driver program.
     * 
     * @param line the line to write
     */
    void write_line(const std::string& line);
    
    /**
     * @see write_line(const std::string& line)
     */
    void write_line(const char* line);
private:
    /**
     * Reads text and commands from the console and sends packets to the
     * outbound packet queue.
     *
     * @param term the terminal to read from
     */
    void process_console();
    
    /**
     * Reads packets from the outbound queue and sends them to the server.
     * 
     * @param sock the socket to send packets to
     * @param term the terminal to write error messages to
     */
    void process_outbound_packets();
    
    /**
     * Reads packets and prints them to the console.
     *
     * @param sock the socket to read packets from.
     * @param term the terminal to write to
     */
    void process_server();
    
    /**
     * Reads audio data from the microphone and puts it into the outbound
     * audio queue.
     */
    void process_microphone();
    
    /**
     * Reads audio data from the inbound queue and plays it over the speakers.
     */
    void process_speakers();
    
    socket::Socket sock;
    terminal term;
    std::atomic<bool> bell_alert;
    bell_toggle_command bell_command_ref;
    
    std::string address;
    std::string port;
    std::string username;
    bool debug;
    bool voice;
    
    /* TODO: update to use sync_queue */
    std::queue<packet> outbound_packets;
    std::mutex outbound_packets_sync;
    std::condition_variable outbound_packets_cv;
    
    std::unique_ptr<Audio_Handle> audio_handle;
    bool audio_active;
    std::queue<Audio_Handle::Block_t> audio_in;
    std::queue<Audio_Handle::Block_t> audio_out;
};

} /* ~namespace vanwestco */

#endif
