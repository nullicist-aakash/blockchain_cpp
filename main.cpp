#include "Blockchain.h"
#include <iostream>
using namespace std;

const char* FILE_PATH = "/Users/aakashgupta/projects/blockchain_cpp/data/1k_initial.dat";

std::string get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in)
        throw errno;

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], (int)contents.size());
    in.close();
    return std::move(contents);
}

int main()
{
    auto contents = get_file_contents(FILE_PATH);

    for (size_t offset = 0, i = 0; offset + sizeof(Blockchain::Block) <= contents.size(); ++i)
    {
        Blockchain::Block b(contents.c_str() + offset);
        cout << "==================================== Blockchain number " << i << " ====================================" << endl;
        cout << b << endl;
        offset += b.block_size + 8;
    }
    return 0;
}