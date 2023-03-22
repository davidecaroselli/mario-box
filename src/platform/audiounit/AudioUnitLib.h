//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_AUDIOUNITLIB_H
#define MARIOBOX_AUDIOUNITLIB_H

#include <AudioUnit/AudioUnit.h>
#include "platform/AudioLibrary.h"

class AudioUnitLib : public AudioLibrary {
public:
    explicit AudioUnitLib(unsigned int sample_rate = 22100);

    void connect(A2A03 *apu) {
        auto proxy = [](double t, void *_apu) {
            auto *apu = (A2A03 *) _apu;
            return apu->sample(t);
        };

        this->connect(proxy, apu);
    }

    void connect(std::function<double(double, void *)> sampler, void *context) override;

    void close() override;

    ~AudioUnitLib();

private:
    const unsigned int sample_rate;
    AudioUnit toneUnit;
    void *render_context;
};


#endif //MARIOBOX_AUDIOUNITLIB_H
