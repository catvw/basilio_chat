#include "core_audio.h++"

/* annoyingly need to do this */
vanwestco::Audio_Handle::Audio_Stream::~Audio_Stream() { }

#ifdef USE_PORTAUDIO_FOR_SOUND
/*----------------------------------------------------------------------------*
 |                              PortAudio_Stream                              |
 *----------------------------------------------------------------------------*/

#include "portaudio.h"

/**
 * Class representing a handle on the PortAudio library, for use inside an
 * Audio_Handle.
 * 
 * @version 0
 */
class PortAudio_Stream : public vanwestco::Audio_Handle::Audio_Stream {
public:
    /**
     * Defines what sort of sample we're getting.
     * TODO: read this from Audio_Handle somehow
     */
    constexpr const static PaSampleFormat SAMPLE_TYPE = paInt16;
    
    PortAudio_Stream();
    ~PortAudio_Stream() override;
    
    vanwestco::Audio_Handle::Block_t record() override;
    void play(vanwestco::Audio_Handle::Block_t& block) override;
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
    
    /**
     * Creates an Audio_Exception from a PortAudio error code.
     * 
     * @param the error code
     * @return the exception
     */
    static vanwestco::Audio_Exception make_aexc(const PaError error);
    
    /**
     * Creates an Audio_Use_Exception from a PortAudio error code.
     * 
     * @param the error code
     * @return the exception
     */
    static vanwestco::Audio_Use_Exception make_auexc(const PaError error);
    
    PaStreamParameters input_parameters;
    PaStreamParameters output_parameters;
    const PaDeviceInfo* input_info;
    const PaDeviceInfo* output_info;
    PaStream* stream;
};

/*----------------------------------------------------------------------------*
 |                       PortAudio_Stream implementation                      |
 *----------------------------------------------------------------------------*/

PortAudio_Stream::PortAudio_Stream() {
    initialize();
    get_device_info();
    set_parameters();
    open_stream();
    start_stream();
}

PortAudio_Stream::~PortAudio_Stream() {
    Pa_StopStream(stream);
    
    try {
        shut_stream();
    } catch (...) { }
    
    try {
        terminate();
    } catch (...) { }
}

vanwestco::Audio_Handle::Block_t PortAudio_Stream::record() {
    vanwestco::Audio_Handle::Block_t block;
    PaError error = Pa_ReadStream(stream,
                                  block.channel().data(),
                                  vanwestco::Audio_Handle::FRAMES_PER_BUFFER);
    if (error != paNoError) {
        throw make_auexc(error);
    }
    
    return block;
}

void PortAudio_Stream::play(vanwestco::Audio_Handle::Block_t& block) {
    PaError error = Pa_WriteStream(stream,
                                   block.channel().data(),
                                   vanwestco::Audio_Handle::FRAMES_PER_BUFFER);
    if (error != paNoError) {
        throw make_auexc(error);
    }
}

/*----------------------------------------------------------------------------*
 |                PortAudio_Stream private implementation                     |
 *----------------------------------------------------------------------------*/

void PortAudio_Stream::initialize() {
    PaError error = Pa_Initialize();
    if (error != paNoError) {
        throw make_aexc(error);
    }
}

void PortAudio_Stream::get_device_info() {
    input_parameters.device = Pa_GetDefaultInputDevice();
    output_parameters.device = Pa_GetDefaultOutputDevice();
    input_info = Pa_GetDeviceInfo(input_parameters.device);
    output_info = Pa_GetDeviceInfo(output_parameters.device);
}

void PortAudio_Stream::set_parameters() {
    input_parameters.channelCount
            = /*input_info->maxInputChannels*/
              vanwestco::Audio_Handle::CHANNEL_COUNT;
    input_parameters.sampleFormat = SAMPLE_TYPE;
    input_parameters.suggestedLatency = input_info->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = nullptr;
    
    output_parameters.channelCount
            = /*output_info->maxOutputChannels*/
              vanwestco::Audio_Handle::CHANNEL_COUNT;
    output_parameters.sampleFormat = SAMPLE_TYPE;
    output_parameters.suggestedLatency = output_info->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = nullptr;
}

void PortAudio_Stream::open_stream() {
    /* currently set to open for blocking I/O */
    PaError error = Pa_OpenStream(&stream,
                                  &input_parameters,
                                  &output_parameters,
                                  vanwestco::Audio_Handle::SAMPLE_RATE,
                                  vanwestco::Audio_Handle::FRAMES_PER_BUFFER,
                                  paClipOff,
                                  nullptr,
                                  nullptr);
    if (error != paNoError) {
        try {
            terminate();
        } catch (...) { }
        
        throw make_auexc(error);
    }
}

void PortAudio_Stream::start_stream() {
    PaError error = Pa_StartStream(stream);
    if (error != paNoError) {
        try {
            shut_stream();
        } catch (...) { }
        
        try {
            terminate();
        } catch (...) { }
        
        throw make_auexc(error);
    }
}

void PortAudio_Stream::shut_stream() {
    /* close the stream */
    PaError error = Pa_CloseStream(stream);
    if (error != paNoError) {
        throw make_auexc(error);
    }
}

void PortAudio_Stream::terminate() {
    /* terminate PortAudio */
    PaError error = Pa_Terminate();
    if (error != paNoError) {
        throw make_aexc(error);
    }
}

vanwestco::Audio_Exception
        PortAudio_Stream::make_aexc(const PaError error) {
    return vanwestco::Audio_Exception(Pa_GetErrorText(error));
}

vanwestco::Audio_Use_Exception
        PortAudio_Stream::make_auexc(const PaError error) {
    return vanwestco::Audio_Use_Exception(Pa_GetErrorText(error));
}

#else
#ifdef USE_PULSEAUDIO_FOR_SOUND
/*----------------------------------------------------------------------------*
 |                             PulseAudio_Stream                              |
 *----------------------------------------------------------------------------*/
/**
 * Class representing a handle on the PulseAudio library, for use inside an
 * Audio_Handle.
 * 
 * @version 0
 */
class PulseAudio_Stream : public vanwestco::Audio_Handle::Audio_Stream {
public:
    /**
     * Defines what sort of sample we're getting.
     * TODO: read this from Audio_Handle somehow
     */
    constexpr const static PaSampleFormat SAMPLE_TYPE = paInt16;
    
    PulseAudio_Stream();
    ~PulseAudio_Stream();
    
    vanwestco::Audio_Handle::Block_t record() override;
    void play(vanwestco::Audio_Handle::Block_t& block) override;
};

#endif /* ~USE_PULSEAUDIO_FOR_SOUND */
#endif /* ~USE_PORTAUDIO_FOR_SOUND */

/*----------------------------------------------------------------------------*
 |                                    Audio_Handle                            |
 *----------------------------------------------------------------------------*/

#ifdef USE_PORTAUDIO_FOR_SOUND
vanwestco::Audio_Handle::Audio_Handle() : stream(new PortAudio_Stream()) { }
#else
#ifdef USE_PULSEAUDIO_FOR_SOUND
vanwestco::Audio_Handle::Audio_Handle() : stream(new PulseAudio_Stream()) { }
#endif /* ~USE_PULSEAUDIO_FOR_SOUND */
#endif /* ~USE_PORTAUDIO_FOR_SOUND */
