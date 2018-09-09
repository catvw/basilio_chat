/*-
 * Driver for basilio_chat.
 * 
 * @author Charles Van West
 * @version 0
 */
 
#include "basilio_chat.h++"
#include "../socket/Socket.h++"

#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <exception>
#include <regex>

#include <argp.h>

struct arguments_object {
    std::string username;
    std::string host;
    std::string port;
    bool debug_mode_on = false;
    bool voice_on = false;
} arguments;

namespace constants {
static constexpr const char* argument_format =
        "[A-Za-z0-9_-]+@[A-Za-z0-9\\.-]+:[0-9]{1,5}";

static constexpr int key_debug = static_cast<int>('d');
static constexpr int key_voice = 0x3001;

static constexpr error_t perr_generic_error = 0x2001;
} /* ~namespace constants */

const char* argp_program_version = "basilio_chat 0.0";
const char* argp_program_bug_address = "nowhere@bananaland.gov";

static const char* doc = "What a great thing the internet is.";
static const char* args_doc = "user@host:port";

static const struct argp_option options[] = {
    { "debug", constants::key_debug, nullptr, 0,
            "Turn debug mode on. You never know..." },
    { "voice", constants::key_voice, nullptr, 0,
            "Turn voice transmission/reception on." },
    { nullptr }
};



static error_t parse_function(int key, char* arg, argp_state* state) {
    struct arguments_object* arguments =
            static_cast<arguments_object*>(state->input);
    
    switch (key) {
    case constants::key_debug:
        arguments->debug_mode_on = true;
        break;
    case constants::key_voice:
        arguments->voice_on = true;
        break;
    case ARGP_KEY_NO_ARGS:
        argp_usage(state);
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num > 0) {
            argp_usage(state);
        }
        
        /* split the input into user, host, and port */
        try {
            char* current_zero = arg;
            bool working = true;
            
            /* make sure we've got user@host:port */
            if (!std::regex_match(std::string(arg),
                                  std::regex(constants::argument_format))) {
                throw std::exception();
            }
            
            for (int i = 0; working; ++i) {
                switch (current_zero[i]) {
                case '@':
                    arguments->username = std::string(current_zero, i);
                    current_zero = &current_zero[i + 1];
                    i = -1;
                    break;
                case ':':
                    arguments->host = std::string(current_zero, i);
                    current_zero = &current_zero[i + 1];
                    i = -1;
                    break;
                case '\0':
                    arguments->port = std::string(current_zero, i);
                    working = false;
                    break;
                }
            }
        } catch (std::exception&) {
            argp_usage(state);
            return constants::perr_generic_error;
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static const struct argp parser = { options, parse_function, args_doc, doc };

using namespace vanwestco;

int main(int argc, char** argv) {
    error_t err = argp_parse(&parser, argc, argv, ARGP_NO_EXIT,
                             nullptr, static_cast<void*>(&arguments));
    if (err != 0 || arguments.host.length() == 0) { return err; }
    
    basilio_chat program(arguments.host,
                         arguments.port,
                         arguments.username,
                         arguments.debug_mode_on,
                         arguments.voice_on);
    try {
        program.main();
    } catch (socket::SocketException& ex) {
        std::ostringstream error;
        error << argv[0]
              << ": socket exception: "
              << ex.what();
        program.write_line(error.str());
        return 2;
    } catch (basilio_chat::exception& ex) {
        std::ostringstream error;
        error << argv[0]
              << ": "
              << ex.what();
        program.write_line(error.str());
        return 3;
    }
    
    return 0;
}
