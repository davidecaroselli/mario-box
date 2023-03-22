//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_AUDIOLIBRARY_H
#define MARIOBOX_AUDIOLIBRARY_H

#include <functional>
#include "core/A2A03.h"

class AudioLibrary {
public:
    virtual void connect(std::function<double(double, void *)> sampler, void *context) = 0;

    virtual void close() = 0;

    ~AudioLibrary() = default;
};


#endif //MARIOBOX_AUDIOLIBRARY_H
