// Logging system, bcz i need it
#include <terminal.hpp>
#include <logging.hpp>

Logging::Logging(const char *log_name) {
    this->log_name = log_name;
}

void Logging::info(const char *text) {
    Terminal::Print("[");Terminal::Print(this->log_name);Terminal::Print("][INFO]: ");
    Terminal::Print(text);
}