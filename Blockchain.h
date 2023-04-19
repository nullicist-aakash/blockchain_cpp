#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

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

    struct InputTransaction
    {
        std::uint8_t prev_trans_hash[32] {};
        std::uint32_t output_index {};
        std::uint64_t sign_public_length;
        std::uint8_t* sign_public_key;
        std::uint32_t seq_no;

        InputTransaction(const char*, size_t&);

        friend std::ostream& operator<< (std::ostream&, const InputTransaction&);

        ~InputTransaction();
    };

    struct OutputTransaction
    {
        std::uint64_t coin_value;
        std::uint64_t out_script_len;
        std::uint8_t* script_public_key;

        OutputTransaction(const char*, size_t&);

        friend std::ostream& operator<< (std::ostream&, const OutputTransaction&);

        ~OutputTransaction();
    };

    struct Transaction
    {
        std::uint32_t version;
        std::uint64_t n_inputs;
        std::vector<InputTransaction> inputs;
        std::uint64_t n_outputs;
        std::vector<OutputTransaction> outputs;

        Transaction(const char*, size_t&);

        friend std::ostream& operator<< (std::ostream&, const Transaction&);
    };

    struct Block
    {
        std::uint32_t magic_number{};
        std::uint32_t block_size{};
        BlockHeader header{};
        std::uint64_t n_transactions{};
        std::vector<Transaction> transactions;
        std::uint32_t lock_time;

        explicit Block(const char*, size_t&);
        [[nodiscard("Heap allocated object should be freed")]] std::uint8_t* get_block_hash() const;
        friend std::ostream& operator<< (std::ostream&, const Block&);
    };

    void hex_view(const char* str, const size_t len);
}