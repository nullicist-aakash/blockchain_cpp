#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#define HEX( x, len ) std::setw(2 * len) << std::setfill('0') << std::hex << std::uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << std::dec

// Displays the memory contents of `str` in hexadecimal format, covering initial `len` bytes.
void hex_view(const char* str, const size_t len)
{
    for (int i = 0; i < ((int)len >> 4) + (len % 16 ? 1 : 0); ++i)
    {
        // Get the range of data to print
        int start = i * 16;
        int end = std::min(i * 16 + 15, (int)len - 1);

        // Print the HEX offset to left
        std::cout << "0x" << HEX(start, 4) << " |  ";

        // Print the bytes in HEX format in middle
        for (int j = start; j <= end; ++j)
            std::cout << HEX(str[j], 1) << (j % 8 == 7 ? "  " : " ");

        for (int j = end; j < i * 16 + 15; ++j)
            std::cout << (j % 8 == 7 ? "    " : "   ");

        std::cout << "| ";

        // Print the ASCII equivalent on right side
        for (int j = start; j <= end; ++j)
            std::cout << (std::isprint(str[j]) ? str[j] : '.');
        std::cout << '\n';
    }
}

// Returns the file contents as C++ string
std::string get_file_contents(const char *filename)
{
    // Open file for reading in binary mode
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in)
    {
        std::cerr << "Error in opening file" << std::endl;
        throw errno;
    }

    std::string contents;

    // Allocate required memory in heap for string
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());

    // Copy contents from file to string
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], (int)contents.size());

    // return the string
    return contents;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./reader.out <block_data>" << std::endl;
        exit(-1);
    }

    const char* FILE_PATH = argv[1];
    auto contents = get_file_contents(FILE_PATH);
    hex_view(contents.c_str(), contents.size());
}