#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

int main() {
    const char* passFileEnv = std::getenv("SSH_PASS_FILE");
    if (!passFileEnv) {
        return 1;
    }

    std::ifstream f(passFileEnv, std::ios::binary);
    if (f) {
        // Read the whole file
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        // Print to stdout
        std::cout << content;
    }
    return 0;
}
