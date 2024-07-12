#include <iostream>
#include <windows.h>

typedef void (*HelloWorldFunc)();

int main() {
    std::cout << "Simple Application Running..." << std::endl;
    // Load the SimpleDLL
    HMODULE hModule = LoadLibraryA("AppDLL.dll");
    if (hModule) {
        std::cout << "SimpleDLL.dll loaded successfully!" << std::endl;
        HelloWorldFunc HelloWorld = (HelloWorldFunc)GetProcAddress(hModule, "HelloWorld");
        if (HelloWorld) {
            HelloWorld();
        }
        else {
            std::cout << "Failed to find HelloWorld function." << std::endl;
        }
        FreeLibrary(hModule);
    }
    else {
        std::cout << "Failed to load SimpleDLL.dll." << std::endl;
    }
    system("pause");
    return 0;
}