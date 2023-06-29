#include <memory>
#include "memory.h"
#include <fstream>

Memory::Memory() {
    Reset();
}

uint8_t* Memory::getRawMemory() {
    return memory.data();
}

uint16_t* Memory::getRawEndAddressTable() {
    return endAddressTable.data();
}

void Memory::Reset() {
    memory.fill(0);
    endAddressTable.fill(0);
    std::copy(begin(SPRITES), end(SPRITES), begin(memory));
}

bool Memory::initializeFromFile(std::string path) {
    std::ifstream file(path, std::ios::binary);
    if (file) {
        // Find the file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Create a vector to hold the file contents
        std::vector<uint8_t> buffer(fileSize);

        // Read the file byte-wise into the vector
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        std::copy(buffer.begin(), buffer.end(), memory.begin() + 0x200);
        file.close();
        return true;
    }   
    return false;
            
}