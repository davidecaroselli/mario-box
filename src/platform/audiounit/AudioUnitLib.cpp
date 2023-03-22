//
// Created by Davide Caroselli on 22/03/23.
//

#include "AudioUnitLib.h"
#include <string>
#include <utility>

#define M_TAU 6.283185307179586

struct render_context_t {
    double const sample_rate;
    std::function<double(double, void *)> sampler;
    void *sampler_context;

    double theta = 0;

    render_context_t(const double sample_rate, std::function<double(double, void *)> sampler, void *context)
            : sample_rate(sample_rate), sampler(std::move(sampler)), sampler_context(context) {}
};

OSStatus RenderWave(void *_context, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32,
                    UInt32 inNumberFrames, AudioBufferList *ioData) {
    auto *render_context = (render_context_t *) _context;

    auto *mono = (SInt16 *) ioData->mBuffers[0].mData;
    for (UInt32 frame = 0; frame < inNumberFrames; ++frame) {
        double sample = render_context->sampler(render_context->theta, render_context->sampler_context);
        mono[frame] = (SInt16) (sample * 32767.0f);

        render_context->theta += M_TAU / render_context->sample_rate;
        if (render_context->theta > M_TAU) {
            render_context->theta -= M_TAU;
        }
    }

    return noErr;
}

AudioUnitLib::AudioUnitLib(unsigned int sample_rate) : sample_rate(sample_rate) {
    AudioComponentDescription acd = {
            .componentType = kAudioUnitType_Output,
            .componentSubType = kAudioUnitSubType_DefaultOutput,
            .componentManufacturer = kAudioUnitManufacturer_Apple,
    };

    AudioComponent output = AudioComponentFindNext(nullptr, &acd);
    if (!output) throw std::runtime_error("can't find default output");

    OSStatus err = AudioComponentInstanceNew(output, &toneUnit);
    if (err) throw std::runtime_error("error creating unit: " + std::to_string(err));
}


void AudioUnitLib::connect(std::function<double(double, void *)> sampler, void *context) {
    render_context = new render_context_t(sample_rate, sampler, context);

    OSStatus err;

    AURenderCallbackStruct input = {.inputProc = RenderWave, .inputProcRefCon = render_context};
    err = AudioUnitSetProperty(toneUnit, kAudioUnitProperty_SetRenderCallback,
                               kAudioUnitScope_Input, 0, &input, sizeof(input));
    if (err) throw std::runtime_error("error setting callback: " + std::to_string(err));

    AudioStreamBasicDescription audio_desc = {
            .mSampleRate = (Float64) sample_rate,
            .mFormatID = kAudioFormatLinearPCM,
            .mFormatFlags = 0
                            | kAudioFormatFlagIsSignedInteger
                            | kAudioFormatFlagIsPacked
                            | kAudioFormatFlagIsNonInterleaved,
            .mBytesPerPacket = 2,
            .mFramesPerPacket = 1,
            .mBytesPerFrame = 2,
            .mChannelsPerFrame = 1,
            .mBitsPerChannel = 16,
    };

    err = AudioUnitSetProperty(toneUnit, kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Input, 0, &audio_desc, sizeof(audio_desc));
    if (err) throw std::runtime_error("error setting stream format: " + std::to_string(err));

    err = AudioUnitInitialize(toneUnit);
    if (err) throw std::runtime_error("error initializing unit: " + std::to_string(err));

    err = AudioOutputUnitStart(toneUnit);
    if (err) throw std::runtime_error("error starting unit: " + std::to_string(err));
}

void AudioUnitLib::close() {
    AudioOutputUnitStop(toneUnit);
    AudioUnitUninitialize(toneUnit);
    AudioComponentInstanceDispose(toneUnit);
}

AudioUnitLib::~AudioUnitLib() {
    if (render_context)
        delete (render_context_t *) render_context;
}
