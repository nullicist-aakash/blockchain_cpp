#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include "SHA/SHA256.h"
#define HEX( x, len ) std::setw(2 * len) << std::setfill('0') << std::hex << std::uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << std::dec

// Returns the file contents as C++ string
std::string get_file_contents(const char *filename)
{
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
    std::uint64_t n_transactions{};
};

// Reads variable integer from location `str` and returns it.
uint64_t read_varint(const char* str, size_t& delta)
{
    uint64_t output = 0;

    memcpy(&output, str, 1);
    if (output < 0xFD)
        delta += 1;
    else if (output == 0xFD)
        output = *(uint16_t*)(str + 1), delta += 3;
    else if (output == 0xFE)
        output = *(uint32_t*)(str + 1), delta += 5;
    else
        output = *(uint64_t*)(str + 1), delta += 9;

    return output;
}

// Computes the double SHA-256 hash of the given data
uint8_t* double_sha256(uint8_t* start, long long len)
{
    // Calculate hash
    SHA256 sha1;
    sha1.update(start, len);
    uint8_t * digest = sha1.digest();

    // Double hash
    SHA256 sha2;
    sha2.update(digest, 32);
    auto digest2 = sha2.digest();

    // Remove old digest, as it is not required anymore
    delete[] digest;

    return digest2;
}

// Computes the merkle hash from the given leaf hashes
void merkle_hash(std::vector<uint8_t*>& hashes)
{
    while (hashes.size() != 1)
    {
        // If array size is odd, make it even
        if (hashes.size() % 2 == 1)
        {
            // make a deep copy of last hash so that we can delete it safely without double delete problem
            auto new_elem = new uint8_t[32];
            memcpy(new_elem, hashes.back(), 32);
            hashes.push_back(new_elem);
        }

        for (auto i = 0; i < (hashes.size() / 2); ++i)
        {
            // Concatenate two hashes
            auto concat = new uint8_t[64];
            memcpy(concat, hashes[i * 2], 32);
            memcpy(concat + 32, hashes[i * 2 + 1], 32);

            // Calculate double SHA-256 hash of concatenation
            auto new_hash = double_sha256(concat, 64);
            delete[] concat;

            // delete memory at index i and use current value instead
            delete[] hashes[i];
            hashes[i] = new_hash;
        }

        // Deallocate the other half of vector from heap
        for (auto i = hashes.size() / 2; i < hashes.size(); ++i)
            delete[] hashes[i];

        // resize array
        hashes.resize(hashes.size() / 2);
    }
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
    auto block_base = contents.c_str();
    size_t offset = 0;
    for (int i = 0; i < 1000; ++i)
    {
        // create a block
        Block block;

        // copy initial 88 bytes from contents
        memcpy(&block, block_base + offset, 88);

        // offset points to number of transactions
        std::cout << "Offsets of block #" << i << ": 0x" << HEX(offset, 4) << " to 0x" << HEX(offset + block.block_size + 8 - 1, 4) << std::endl;
        offset += 88;

        // Print current block information
        std::cout << "Magic Number: " << HEX(block.magic_number, 4) << std::endl;
        std::cout << "Blockchain Size: " << block.block_size << std::endl;
        std::cout << "Header Info: " << std::endl;
        std::cout << "\tVersion Number           : " << block.header.version_number << std::endl;
        std::cout << "\tPrevious Hash            : ";
        for (int j = 31; j >= 0; --j)
            std::cout << HEX(block.header.prev_hash[j], 1);
        std::cout << std::endl << "\tMerkle Hash (Stored)     : ";
        for (int j = 31; j >= 0; --j)
            std::cout << HEX(block.header.merkle_hash[j], 1);

        std::cout << std::endl << "\tCurrent Hash (Calculated): ";
        auto hash = double_sha256((uint8_t*)&block.header, 80);
        for (int j = 31; j >= 0; --j)
            std::cout << HEX(hash[j], 1);
        delete[] hash; // Free the memory after usage

        std::cout << std::endl << "\tTime of Block Creation   : ";
        std::time_t temp = block.header.time;
        std::cout << std::put_time(std::gmtime(&temp), "%Y-%m-%d %I:%M:%S %p") << std::endl;
        std::cout << "\tDifficulty               : " << HEX(block.header.difficulty, 4) << std::endl;
        std::cout << "\tNonce                    : " << block.header.nonce << std::endl;

        // Read the number of transactions
        auto n_transactions = read_varint(block_base + offset, offset);
        std::cout << "\tNumber of Transactions   : " << n_transactions << std::endl;

        std::vector<uint8_t*> hashes;
        // Read the actual transactions, offset now points to beginning of first transaction
        for (std::uint64_t transaction_index = 0; transaction_index < n_transactions; ++transaction_index)
        {
            auto start_address = block_base + offset;
            std::cout << "\t\tTransaction #" << transaction_index << " offsets: 0x" << HEX(offset, 4) << " to 0x";

            // skip version number
            offset += 4;

            // read input
            auto n_inputs = read_varint(block_base + offset, offset);

            // read all inputs
            while (n_inputs--)
            {
                // skip previous transaction hash and output index
                offset += 36;

                // read signature script length
                auto script_len = read_varint(block_base + offset, offset);

                // skip script and sequence number
                offset += script_len + 4;
            }

            // read outputs
            auto n_outputs = read_varint(block_base + offset, offset);

            // read all outputs
            while (n_outputs--)
            {
                // skip Satoshis
                offset += 8;

                // read length of public key script
                auto script_len = read_varint(block_base + offset, offset);

                // skip key script
                offset += script_len;
            }

            // skip lock time
            offset += 4;

            std::cout << HEX(offset - 1, 4) << std::endl;

            // get the length of transaction
            auto transaction_len = block_base + offset - start_address;
            hashes.push_back(double_sha256((uint8_t*)(start_address), transaction_len));
        }

        // Compute the merkle hash from transactions
        merkle_hash(hashes);

        std::cout << "\tMerkle Hash (Computed)   : ";
        for (int j = 31; j >= 0; --j)
            std::cout << HEX(hashes[0][j], 1);

        // deallocate memory of computed hash
        delete[] hashes[0];

        std::cout << std::endl << std::endl;

        if (n_transactions > 1)
            break;
    }
}