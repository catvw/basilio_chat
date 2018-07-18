/**
 * Audio wrapper for basilio_chat, so I don't have as many headaches.
 * 
 * @author Charles Van West
 */

#ifndef BASILIO_CHAT_CORE_AUDIO_HXX
#define BASILIO_CHAT_CORE_AUDIO_HXX

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
 * terminating the audio library.
 * 
 * @version 1
 */
class Audio_Exception : public std::exception {
public:
    /**
     * Constructs an Audio_Exception with a custom message.
     * 
     * @param err the message
     */
    Audio_Exception(const std::string& err) : error(err) { }
    Audio_Exception(const char* err)        : error(err) { }
    
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
 * @version 0
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
 * @version 4
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
 * @version 4
 */
class Audio_Handle {
public:
    /**
     * Represents the sample-per-second rate.
     */
    constexpr const static int SAMPLE_RATE = 44100;
    
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
     * Base class for the system-dependent audio streaming implementation.
     * TODO: make more generic.
     * 
     * @version 0
     */
    class Audio_Stream {
    public:
        /**
         * Attempts to record a block of single-channel audio data from the
         * default input.
         * 
         * @throws Audio_Use_Exception
         * @return the recorded data
         */
        virtual Block_t record() = 0;
        
        /**
         * Attemps to play a block of ~stereo~ mono audio on the default output.
         * 
         * Note: {block} is not const solely because for some reason that causes
         * segfaults. {block} *should* be left unmodified.
         * 
         * @param block the block to play
         * @throws Audio_Use_Exception 
         */
        virtual void play(Block_t& block) = 0;
        
        /**
         * Attempts to record and play blocks of audio simultaneously, so
         * neither method need lock the other out for high-quality voice chat.
         * TODO: implement such a method.
         * 
         * @param block the block to play
         * @throws Audio_Use_Exception
         * @return the recorded data
         */
        /* virtual Block_t play_record(Block_t& block) = 0; */
        
        /**
         * For cleanup purposes.
         */
        virtual ~Audio_Stream() = 0;
    };
    
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
    ~Audio_Handle() = default;
    
    /**
     * Records a block of single-channel audio data from the default
     * input. This really just calls record() on the underlying stream.
     * 
     * @see Audio_Stream::record()
     * @throws Audio_Use_Exception
     * @return the recorded data
     */
    inline Block_t record_block() { return stream->record(); }
    
    /**
     * Plays a block of mono audio on the default output. This really just calls
     * play() on the underlying stream.
     * 
     * @param block the block to play
     * @see Audio_Stream::play()
     * @throws Audio_Use_Exception
     */
    inline void play_block(Block_t& block) { stream->play(block); }
private:
    std::unique_ptr<Audio_Stream> stream;
};

} /* ~namespace vanwestco */

#endif /* ~BASILIO_CHAT_CORE_AUDIO_HXX */
