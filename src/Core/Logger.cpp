#include <Arclight/Core/Logger.h>

#include <iostream>

#include <string>

namespace Arclight {
namespace Logger {

template<>
void Log<std::string>(const std::string& arg){
    std::cout << arg;
}

void Log(short arg){
    std::cout << arg;
}

void Log(int arg){
    std::cout << arg;
}

void Log(long arg){
    std::cout << arg;
}

void Log(unsigned short arg){
    std::cout << arg;
}

void Log(unsigned int arg){
    std::cout << arg;
}

void Log(unsigned long arg){
    std::cout << arg;
}

void Log(const char* arg){
    std::cout << arg;
}

} // namespace Logger
} // namespace Arclight