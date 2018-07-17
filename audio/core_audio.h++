/**
 * PortAudio wrapper for basilio_chat, so I don't have as many headaches.
 * 
 * @author Charles Van West
 */

#ifndef BASILIO_CHAT_CORE_AUDIO_HXX
#define BASILIO_CHAT_CORE_AUDIO_HXX

namespace portaudio {

#include "portaudio.h"

/* format aliases */
constexpr const PaSampleFormat paFloat32_alias = paFloat32;
constexpr const PaSampleFormat paInt16_alias = paInt16;

/* flag aliases */
constexpr const PaStreamFlags paClipOff_alias = paClipOff;
} /* ~namespace portaudio */

#include <string>
#include <array>
#include <memory>
#include <utility>
#include <exception>
#include <stdexcept>
#include <cstdint>

namespace vanwestco {

/*----------------------------------------------------------------------------*
 |                            Audio_(Use_)?Exception                          |
 *----------------------------------------------------------------------------*/

/**
 * General audio exception class, thrown when there's an error initializing or
 * terminating the PortAudio library.
 * 
 * @version 0.0
 */
class Audio_Exception : public std::exception {
public:
    /**
     * Constructs an Audio_Exception from the raw PortAudio error code.
     * 
     * @param err the error code
     */
    Audio_Exception(const portaudio::PaError err)
    : error(portaudio::Pa_GetErrorText(err)) { }
    
    /**
     * Constructs an Audio_Exception with a custom message.
     * 
     * @param err the message
     */
    Audio_Exception(const std::string& err) : error(err) { }
    
    /**
     * Gets the error message.
     * 
     * @return the message
     */
    const char* what() const noexcept override { return error.c_str(); }
private:
    std::string error;
};

/**
 * More specific audio exception class, thrown when something goes wrong while
 * using the library.
 * 
 * @version 0.0
 */
class Audio_Use_Exception : public Audio_Exception {
public:
    using Audio_Exception::Audio_Exception; /* inherit constructors */
};

/*----------------------------------------------------------------------------*
 |                               Audio_Block                                  |
 *----------------------------------------------------------------------------*/

/**
 * Template for a variable-channel-count block of audio.
 * 
 * @tparam S the sample type
 * @tparam C the channel count
 * @tparam N the per-channel block size
 * 
 * @version 0.4
 */
template <typename S, int C, std::size_t N> class Audio_Block {
    static_assert(C > 0, "channel count cannot be less than 1");
    static_assert(N > 0, "block size cannot be less than 1");
public:
    /**
     * Type of the audio channels.
     */
    using Channel_t = std::array<S, N>;
    
    /**
     * Constructs an audio block with the correct-size vector of channels.
     */
    Audio_Block() : channels(new std::array<Channel_t, C>) { }
    
    /**
     * No copying.
     */
    Audio_Block(Audio_Block&) = delete;
    
    /**
     * Only moving.
     */
    Audio_Block(Audio_Block&& old) = default;
    
    /**
     * And deleting.
     */
    ~Audio_Block() = default;
    
    /**
     * Gets a channel by its index, if possible; defaults to the channel at
     * index 0 (useful for mono data).
     * 
     * @throws std::out_of_range if an index is out of bounds
     * 
     * @return the channel
     */
    Channel_t& channel(int index = 0) {
        if (index < 0 || index > C) {
            throw std::out_of_range("index out of bounds");
        }
        return (*channels)[index];
    }
    
    const Channel_t& channel(int index = 0) const {
        return channel(index);
    }
private:
    /**
     * Stores the channels in a moveable housing.
     */
    std::unique_ptr<std::array<Channel_t, C>> channels;
};

/*----------------------------------------------------------------------------*
 |                               Audio_Handle                                 |
 *----------------------------------------------------------------------------*/

/**
 * Represents a handle on the default system audio devices. The implicit
 * assumption here is that the system consists of a single-channel microphone
 * and stereo speakers.
 * 
 * @version 0.1
 */
class Audio_Handle {
public:
    /**
     * Represents the sample-per-second rate.
     */
    constexpr const static int SAMPLE_RATE = 44100;
    
    /**
     * Defines what sort of sample we're getting.
     */
    constexpr const static portaudio::PaSampleFormat SAMPLE_TYPE =
            portaudio::paInt16_alias;
    
    /**
     * Alias for the typename of the sample type.
     */
    using Sample_t = std::int16_t;
    
    /**
     * Sets the number of channels. Doesn't do anything to anything other than
     * Block_t and the Portaudio initialization. Currently set to mono.
     */
    constexpr const static int CHANNEL_COUNT = 1;
    
    /**
     * Sets the number of frames per buffer.
     */
    constexpr const static std::size_t FRAMES_PER_BUFFER = 4096;
    
    /**
     * Holds the size of each sample.
     */
    constexpr const static std::size_t SAMPLE_SIZE = sizeof(Sample_t);
    
    /**
     * Type alias for the kind of audio block Audio_Handle uses.
     */
    using Block_t = Audio_Block<Sample_t,
                                CHANNEL_COUNT,
                                FRAMES_PER_BUFFER>;
    
    /**
     * Constructs an audio handle. Behavior is only defined when one handle
     * exists at a time, which is probably not good but fine for now.
     * TODO: add the option to change the number of frames per buffer.
     * 
     * @throws Audio_Exception if something happens during PortAudio startup
     * @throws Audio_Use_Exception if something happens during stream opening
     */
    Audio_Handle();
    
    /**
     * Destroys the audio handle. As with the constructor, behavior is only
     * defined with one handle in existence.
     */
    ~Audio_Handle();
    
    /**
     * Attempts to record a block of single-channel audio data from the default
     * input.
     * 
     * @throws Audio_Use_Exception
     * 
     * @return the recorded data
     */
    Block_t record_block();
    
    /**
     * Attemps to play a block of ~stereo~ mono audio on the default output.
     * 
     * Note: {block} is not const solely because for some reason that causes
     * segfaults. {block} *should* be left unmodified.
     * 
     * @throws Audio_Use_Exception if something happens
     */
    void play_block(Block_t& block);
    
    /**
     * Attempts to record and play blocks of audio simultaneously, so neither
     * method need lock the other out for high-quality voice chat.
     * 
     * @throws Audio_Use_Exception
     * 
     * @return the recorded data
     */
    /* TODO: implement such a method */
private:
    /**
     * Initializes the Portaudio library.
     * 
     * @throws Audio_Exception
     */
    void initialize();
    
    /**
     * Acquires the default input and output device info.
     */
    void get_device_info();
    
    /**
     * Sets the input and output stream parameters.
     */
    void set_parameters();
    
    /**
     * Opens the stream.
     * 
     * @throws Audio_Use_Exception
     */
    void open_stream();
    
    /**
     * Starts the stream for reading and writing.
     * 
     * @throws Audio_Use_Exception
     */
    void start_stream();
    
    /**
     * Shuts down the audio stream.
     * 
     * @throws Audio_Use_Exception
     */
    void shut_stream();
    
    /**
     * Terminates the PortAudio library.
     * 
     * @throws Audio_Exception
     */
    void terminate();
    
    portaudio::PaStreamParameters input_parameters;
    portaudio::PaStreamParameters output_parameters;
    const portaudio::PaDeviceInfo* input_info;
    const portaudio::PaDeviceInfo* output_info;
    portaudio::PaStream* stream;
};

} /* ~namespace vanwestco */

#endif /* ~BASILIO_CHAT_CORE_AUDIO_HXX */
