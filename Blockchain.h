#pragma once
#include <cstdint>
#include <fstream>

namespace Blockchain
{
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

        explicit Block(const char*);
        [[nodiscard("Heap allocated object should be freed")]] std::uint8_t* get_block_hash() const;
        friend std::ostream& operator<< (std::ostream&, const Block&);
        ~Block();
    };

    void hex_view(const char* str, const size_t len);
}