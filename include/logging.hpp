#pragma once

class Logging
{
    public:
        Logging(const char *log_name);
        void info(const char *text);
    private:
        const char *log_name;
};
