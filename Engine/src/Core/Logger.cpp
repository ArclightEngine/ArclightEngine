#include <Arclight/Core/Logger.h>

#include <iostream>

#include <string>

namespace Arclight {
namespace Logger {

template<>
void Log(const UnicodeString& arg){
    std::string s;
    arg.toUTF8String<std::string>(s);
    std::cout << s;
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

void Log(float arg){
    std::cout << arg;
}

} // namespace Logger
} // namespace Arclight