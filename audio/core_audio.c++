#include "core_audio.h++"

/*----------------------------------------------------------------------------*
 |                                    Audio_Handle                            |
 *----------------------------------------------------------------------------*/

vanwestco::Audio_Handle::Audio_Handle() {
    initialize();
    get_device_info();
    set_parameters();
    open_stream();
    start_stream();
}

vanwestco::Audio_Handle::~Audio_Handle() {
    portaudio::Pa_StopStream(stream);
    
    try {
        shut_stream();
    } catch (...) { }
    
    try {
        terminate();
    } catch (...) { }
}

vanwestco::Audio_Handle::Block_t vanwestco::Audio_Handle::record_block() {
    vanwestco::Audio_Handle::Block_t block;
    portaudio::PaError error = portaudio::Pa_ReadStream(stream,
                                                        block.channel().data(),
                                                        FRAMES_PER_BUFFER);
    if (error != portaudio::paNoError) {
        throw vanwestco::Audio_Use_Exception(error);
    }
    
    return block;
}

void vanwestco::Audio_Handle::play_block(
        vanwestco::Audio_Handle::Block_t& block) {
    portaudio::PaError error =
            portaudio::Pa_WriteStream(stream,
                                      block.channel().data(),
                                      FRAMES_PER_BUFFER);
    if (error != portaudio::paNoError) {
        throw vanwestco::Audio_Use_Exception(error);
    }
}

/*----------------------------------------------------------------------------*
 |                             Private functions                              |
 *----------------------------------------------------------------------------*/

void vanwestco::Audio_Handle::initialize() {
    portaudio::PaError error = portaudio::Pa_Initialize();
    if (error != portaudio::paNoError) {
        throw vanwestco::Audio_Exception(error);
    }
}

void vanwestco::Audio_Handle::get_device_info() {
    input_parameters.device = portaudio::Pa_GetDefaultInputDevice();
    output_parameters.device = portaudio::Pa_GetDefaultOutputDevice();
    input_info = portaudio::Pa_GetDeviceInfo(input_parameters.device);
    output_info = portaudio::Pa_GetDeviceInfo(output_parameters.device);
}

void vanwestco::Audio_Handle::set_parameters() {
    input_parameters.channelCount
            = /*input_info->maxInputChannels*/ CHANNEL_COUNT;
    input_parameters.sampleFormat = SAMPLE_TYPE;
    input_parameters.suggestedLatency = input_info->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = nullptr;
    
    output_parameters.channelCount
            = /*output_info->maxOutputChannels*/ CHANNEL_COUNT;
    output_parameters.sampleFormat = SAMPLE_TYPE;
    output_parameters.suggestedLatency = output_info->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = nullptr;
}

void vanwestco::Audio_Handle::open_stream() {
    /* currently set to open for blocking I/O */
    portaudio::PaError error
            = portaudio::Pa_OpenStream(&stream,
                                       &input_parameters,
                                       &output_parameters,
                                       SAMPLE_RATE,
                                       FRAMES_PER_BUFFER,
                                       portaudio::paClipOff_alias,
                                       nullptr,
                                       nullptr);
    if (error != portaudio::paNoError) {
        try {
            terminate();
        } catch (...) { }
        
        throw vanwestco::Audio_Use_Exception(error);
    }
}

void vanwestco::Audio_Handle::start_stream() {
    portaudio::PaError error = portaudio::Pa_StartStream(stream);
    if (error != portaudio::paNoError) {
        try {
            shut_stream();
        } catch (...) { }
        
        try {
            terminate();
        } catch (...) { }
        
        throw vanwestco::Audio_Use_Exception(error);
    }
}

void vanwestco::Audio_Handle::shut_stream() {
    /* close the stream */
    portaudio::PaError error = portaudio::Pa_CloseStream(stream);
    if (error != portaudio::paNoError) {
        throw vanwestco::Audio_Use_Exception(error);
    }
}

void vanwestco::Audio_Handle::terminate() {
    /* terminate PortAudio */
    portaudio::PaError error = portaudio::Pa_Terminate();
    if (error != portaudio::paNoError) {
        throw vanwestco::Audio_Exception(error);
    }
}
