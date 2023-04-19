#include "Blockchain.h"
#include "SHA256.h"
#include <cstring>
#include <cmath>
#include <iostream>

#define HEX( x, len ) std::setw(2 * len) << std::setfill('0') << std::hex << std::uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << std::dec

namespace Blockchain
{
    Block::Block(const char* str)
    {
        constexpr size_t reads = sizeof(magic_number) + sizeof(block_size) + sizeof(header) + sizeof(n_transactions);
        memcpy(this, str, reads);
        payload = new char[block_size - 80 - 1];
        memcpy(payload, str + reads, block_size - 80 - 1);
    }

    uint8_t* Block::get_block_hash() const
    {
        SHA256 sha1;
        sha1.update(std::string((char*)&header, (char*)&header + sizeof(header)));
        uint8_t * dest = sha1.digest();

        SHA256 sha2;
        sha2.update(std::string(dest, dest + 32));
        delete[] dest;

        dest = sha2.digest();
        return dest;
    }

    std::ostream& operator<< (std::ostream& out, const Block& block)
    {
        out << "Magic Number: " << HEX(block.magic_number, 4) << std::endl;
        out << "Blockchain Size: " << block.block_size << std::endl;
        out << "Header Info: " << std::endl;
        out << "\tVersion Number: " << block.header.version_number << std::endl;
        out << "\tPrevious Hash: ";
        for (int i = 31; i >= 0; --i)
            out << HEX(block.header.prev_hash[i], 1);
        out << std::endl << "\tMerkle Hash:   ";
        for (int i = 31; i >= 0; --i)
            out << HEX(block.header.merkle_hash[i], 1);
        out << std::endl << "\tCurrent Hash:  ";
        auto res = block.get_block_hash();
        for (int i = 31; i >= 0; --i)
            out << HEX(res[i], 1);
        delete[] res;

        out << std::endl << "\tTime of Transaction: ";
        std::time_t temp = block.header.time;
        out << std::put_time(std::gmtime(&temp), "%Y-%m-%d %I:%M:%S %p") << std::endl;

        out << "\tDifficulty: " << HEX(block.header.difficulty, 4) << std::endl;
        out << "\tNonce: " << block.header.nonce << std::endl;
        return out;
    }

    Block::~Block()
    {
        delete[] payload;
    }

    inline auto min(int a, int b)
    {
        return a < b ? a : b;
    }

    void hex_view(const char* str, const size_t len)
    {
        for (int i = 0; i < ((int)len >> 4) + (len % 16 ? 1 : 0); ++i)
        {
            int start = i * 16;
            int end = min(i * 16 + 15, (int)len);
            std::cout << "0x" << HEX(start, 4) << " | ";
            for (int j = start; j <= end; ++j)
                std::cout << HEX(str[j], 1) << " ";
            for (int j = end; j < i * 16 + 15; ++j)
                std::cout << "   ";
            std::cout << "| ";
            for (int j = start; j <= end; ++j)
                std::cout << (std::isprint(str[j]) ? str[j] : '.');
            std::cout << std::endl;
        }
    }
}