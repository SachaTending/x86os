#pragma once

class Console
{
    public:
        void operator <<(char val);
        void operator <<(char *val);
        void operator <<(int val);
        void operator <<(const char *val);
}; // class Console

static Console con;