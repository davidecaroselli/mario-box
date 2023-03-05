//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_LOGGER_H
#define MARIOBOX_LOGGER_H


class Logger {
public:
    static void err(const char *format, ...);

    static void warn(const char *format, ...);
};


#endif //MARIOBOX_LOGGER_H
