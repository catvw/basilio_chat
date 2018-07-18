#include "basilio_chat.h++"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <thread>
#include <mutex>
#include <chrono> /* XXX */

using namespace vanwestco;

void basilio_chat::process_console() {
    bool running = true;
    
    while (running) {
        std::string line = term.read_line();
        
        /* check for exit command */
        if (line.compare("/exit") == 0) {
            running = false;
            audio_active = false;
            /* prepare disconnect */
/*          packet pack(1, packet_type::disconnect, false, "X");
            
            try {
                write_packet(sock, &pack);
            } catch (socket::SocketException exc) {
                term->write_err(std::string("exception in exit: ")
                             += exc.what());
            }*/
            packet finish(0,
                          packet_type::null_packet,
                          false,
                          nullptr);
            std::unique_lock<std::mutex> lock(outbound_packets_sync);
            outbound_packets.push(finish);
            outbound_packets_cv.notify_one();
        } else if (line.length() != 0) {
            /* prepare packet */
            packet pack(static_cast<packet_size>(line.length()),
                        packet_type::plaintext,
                        false,
                        line.c_str());
            std::unique_lock<std::mutex> lock(outbound_packets_sync);
            outbound_packets.push(pack);
            outbound_packets_cv.notify_one();
        }
    }
}

void basilio_chat::process_outbound_packets() {
    bool running = true;
    
    while (running) {
        std::unique_lock<std::mutex> lock(outbound_packets_sync);
        
        /* check to see if anything's there */
        if (outbound_packets.empty()) {
            outbound_packets_cv.wait(lock, [this] {
                return !outbound_packets.empty();
            });
        }
        const packet next = outbound_packets.front();
        outbound_packets.pop();
        
        /* check for exit */
        if (next.get_type() == packet_type::null_packet) { /* TODO: change */
            running = false;
        } else { /* send packet */
            try {
                write_packet(&sock, &next);
            } catch (socket::SocketException exc) {
                term.write_err(std::string("exception in send: ")
                            += exc.what());
                running = false;
            }
        }
    }
    
    sock.disconnect();
}

void basilio_chat::process_server() {
    bool running = true;
    
    while (running) {
        try {
            packet pack = read_packet(&sock);
            switch (pack.get_type()) {
            default:
            case packet_type::plaintext:
                term.write_line(std::string(pack.get_payload(),
                                            pack.get_length()));
                if (bell_alert && !pack.is_self()) { std::cout.put('\x07'); }
                break;
            case packet_type::ping:
                /* TODO: implement ping back */
                break;
            case packet_type::typing_list:
                /* TODO: implement list display */
                break;
            case packet_type::audio:
                /* TODO: implement audio */
                break;
            }
        } catch (basilio_chat::exception exc) {
            term.write_err("Server forcibly closed connection. Type /exit to "
                           "exit.");
            running = false;
        } catch (socket::SocketException exc) {
            if (!sock.isConnected()) {
                running = false;
            } else {
                term.write_err(std::string("exception in server handling: ")
                            += exc.what());
            }
        }
    }
}

void basilio_chat::process_microphone() {
    while (audio_active) {
        try {
            /* read a block of audio */
            Audio_Handle::Block_t block = audio_handle->record_block();
            packet pack(block.channel().size() * Audio_Handle::sample_size,
                        packet_type::audio);
            
            /* copy the audio into the packet */
            int pack_index = 0;
            /* XXX, assumes int */
            for (Audio_Handle::Sample_t n : block.channel()) {
                for (int i = 0; i < Audio_Handle::sample_size; ++i) {
                    pack[pack_index++] = static_cast<char>(n & 0xFF);
                    n >>= 8;
                }
            }
            
            /* send the packet off to the output queue */
            std::unique_lock<std::mutex> lock(outbound_packets_sync);
            outbound_packets.push(pack);
            outbound_packets_cv.notify_one();
        } catch (Audio_Use_Exception& exc) {
            term.write_line(exc.what());
        }
    }
}

void basilio_chat::process_speakers() {
    
}

/*----------------------------------------------------------------------------*/

basilio_chat::exception::exception(const std::string& ms) : message(ms) { }

const char* basilio_chat::exception::what() const noexcept {
    return message.c_str();
}

/*----------------------------------------------------------------------------*/

basilio_chat::bell_toggle_command::bell_toggle_command(
        std::atomic<bool>& bell_flag, terminal* term)
: bell_flag(bell_flag), term(term) { }

void basilio_chat::bell_toggle_command::operator()() {
    bell_flag = !bell_flag;
    std::ostringstream message;
    message << "\x1b[33mBell alert: \x1b[31m"
            << (bell_flag ? "on\x07" : "off") << "\x1b[0m";
    term->write_line(message.str());
}

/*----------------------------------------------------------------------------*/

basilio_chat::basilio_chat(const std::string& address,
                           const std::string& port,
                           const std::string& username,
                           bool debug, bool voice)
: address(address), port(port), username(username), debug(debug), voice(voice),
  bell_alert(false), bell_command_ref(bell_alert, &term), audio_active(true),
  audio_handle(voice ? new Audio_Handle() : nullptr) { }

void basilio_chat::main() {
    std::ostringstream out_stream;
    out_stream << "Connecting to " << address << ':' << port
               << " as " << username << "...";
    term.write_line(out_stream.str());
    
    bool success = sock.connect(address.c_str(), port.c_str());
    if (!success) {
        throw basilio_chat::exception(std::string("could not connect to ")
                                      += address);
    }
    
    /* XXX */
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    /* send username over */
    packet uname(username.length(), packet_type::join, false, username.c_str());
    write_packet(&sock, &uname);
    
    /* register commands */
    /* TODO: change the command system to '/'-style commands and rename this
             sort of thing to "keybinds" or whatever */
    term.register_command('l', &bell_command_ref);
    
    /* start threads */
    std::thread console([&] { process_console(); });
    std::thread server([&] { process_server(); });
    std::thread packets([&] { process_outbound_packets(); });
    std::thread microphone;
    if (voice) { microphone = std::thread([&] { process_microphone(); }); }
    
    /* join threads */
    console.join();
    server.join();
    packets.join();
    if (voice) { microphone.join(); }
}

void basilio_chat::write_line(const std::string& line) {
    term.write_line(line);
}

void basilio_chat::write_line(const char* line) {
    term.write_line(line);
}
