#include <iostream>
#include <fstream>
#include "SHA256.h"
using namespace std;

#define HEX( x, len ) setw(2 * len) << setfill('0') << hex << uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << dec
const char* FILE_PATH = "/Users/aakashgupta/projects/blockchain_cpp/data/1k_initial.dat";

struct BlockHeader
{
    std::uint32_t version_number;
    std::uint8_t prev_hash[32];
    std::uint8_t merkle_hash[32];
    std::uint32_t time;
    std::uint32_t difficulty;
    std::uint32_t nonce;
};

struct Block
{
    std::uint32_t magic_number{};
    std::uint32_t block_size{};
    BlockHeader header{};
    std::uint8_t n_transactions{};
    char* payload{};

    explicit Block(const char* str)
    {
        constexpr size_t reads = sizeof(magic_number) + sizeof(block_size) + sizeof(header) + sizeof(n_transactions);
        memcpy(this, str, reads);
        payload = new char[block_size - 80 - 1];
        memcpy(payload, str + reads, block_size - 80 - 1);
        std::reverse(this->header.prev_hash, this->header.prev_hash + 32);
        std::reverse(this->header.merkle_hash, this->header.merkle_hash + 32);
    }
    uint8_t* get_block_hash() const;
    friend std::ostream& operator<< (std::ostream& out, const Block& block);
    ~Block()
    {
        delete[] payload;
    }
};

void hex_view(const char* str, const size_t len)
{
    for (int i = 0; i < ((int)len >> 4) + (len % 16 ? 1 : 0); ++i)
    {
        int start = i * 16;
        int end = min(i * 16 + 15, (int)len);
        cout << "0x" << HEX(start, 4) << " | ";
        for (int j = start; j <= end; ++j)
            cout << HEX(str[j], 1) << " ";
        for (int j = end; j < i * 16 + 15; ++j)
            cout << "   ";
        cout << "| ";
        for (int j = start; j <= end; ++j)
            cout << (std::isprint(str[j]) ? str[j] : '.');
        cout << endl;
    }
}

uint8_t* Block::get_block_hash() const
{
    auto b = header;
    std::reverse(b.prev_hash, b.prev_hash + 32);
    std::reverse(b.merkle_hash, b.merkle_hash + 32);

    SHA256 sha1;
    sha1.update(string((char*)&b, (char*)&b + sizeof(b)));
    uint8_t * dest = sha1.digest();

    SHA256 sha2;
    sha2.update(string(dest, dest + 32));
    delete[] dest;

    dest = sha2.digest();
    // Trick: Bytes are stored in little endian. For calculations, HASH = Block bytes. This hash is directly stored in storage without reversal.
    // If we read these bytes by storing it in 256-bit integer, 0000 will come in the lhs instead of rhs of storage.
    // Alternatively, the result is the little endian direct storage value
    reverse(dest, dest + 32);
    return dest;
}

std::ostream& operator<< (std::ostream& out, const Block& block)
{
    out << "Magic Number: " << HEX(block.magic_number, 4) << endl;
    out << "Block Size: " << block.block_size << endl;
    out << "Header Info: " << endl;
    out << "\tVersion Number: " << block.header.version_number << endl;
    out << "\tPrevious Hash: ";
    for (auto &c: block.header.prev_hash)
        out << HEX(c, 1);
    out << endl << "\tMerkle Hash:   ";
    for (auto &c: block.header.merkle_hash)
        out << HEX(c, 1);
    out << endl << "\tCurrent Hash:  ";
    auto res = block.get_block_hash();
    for (int i = 0; i < 32; ++i)
        cout << HEX(res[i], 1);
    delete[] res;

    out << endl << "\tTime of Transaction: ";
    std::time_t temp = block.header.time;
    out << std::put_time(std::gmtime(&temp), "%Y-%m-%d %I:%M:%S %p") << endl;

    out << "\tDifficulty: " << HEX(block.header.difficulty, 4) << endl;
    out << "\tNonce: " << block.header.nonce << endl;
    return out;
}

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

    for (size_t offset = 0, i = 0; offset + sizeof(Block) <= contents.size(); ++i)
    {
        Block b(contents.c_str() + offset);
        cout << "==================================== Block number " << i << " ====================================" << endl;
        cout << b << endl;
        offset += b.block_size + 8;
    }
    return 0;
}