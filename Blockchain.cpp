#include "Blockchain.h"
#include "SHA256.h"
#include <cstring>
#include <cmath>
#include <iostream>
using namespace std;

#define HEX( x, len ) std::setw(2 * len) << std::setfill('0') << std::hex << std::uppercase << (((1ll << (8 * len)) - 1) & (unsigned int)( x )) << std::dec

namespace Blockchain
{
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

    uint8_t* calculate_double_hash(void* start, size_t len)
    {
        SHA256 sha1;
        sha1.update(std::string((char*)start, (char*)start + len));
        uint8_t * dest = sha1.digest();

        SHA256 sha2;
        sha2.update(std::string(dest, dest + 32));
        delete[] dest;

        dest = sha2.digest();
        return dest;
    }

    InputTransaction::InputTransaction(const char* in, size_t &offset)
    {
        memcpy(this, in + offset, 36);
        offset += 36;

        sign_public_length = read_varint(in + offset, offset);

        sign_public_key = new uint8_t[sign_public_length];
        memcpy(sign_public_key, in + offset, sign_public_length);
        offset += sign_public_length;

        memcpy(&seq_no, in + offset, 4);
        offset += 4;
    }

    std::ostream& operator<< (std::ostream& out, const InputTransaction& in_tran)
    {
        out << "\t\tPrevious Transaction Hash: ";
        for (int i = 31; i >= 0; --i)
            out << HEX(in_tran.prev_trans_hash[i], 1);
        out << endl << "\t\tOutput Index: " << HEX(in_tran.output_index, 4) << endl;
        out << "\t\tScript Signature Length: " << in_tran.sign_public_length << endl;
        out << "\t\tScript Signature: ";
        for (int i = in_tran.sign_public_length - 1; i >= 0; --i)
            out << HEX(in_tran.sign_public_key[i], 1);
        out << endl << "\t\tSequence Number: " << HEX(in_tran.seq_no, 4) << endl;
        return out;
    }

    InputTransaction::~InputTransaction()
    {
        delete[] sign_public_key;
    }

    OutputTransaction::OutputTransaction(const char* in, size_t &offset)
    {
        memcpy(&coin_value, in + offset, 8);
        offset += 8;

        out_script_len = read_varint(in + offset, offset);

        script_public_key = new uint8_t[out_script_len];
        memcpy(script_public_key, in + offset, out_script_len);
        offset += out_script_len;
    }

    std::ostream& operator<< (std::ostream& out, const OutputTransaction& out_tran)
    {
        out << "\t\tCoin Value: " << out_tran.coin_value << " (" << (out_tran.coin_value * 1.0 / 100000000.00) << " bitcoins)" << endl;
        out << "\t\tOutput Script length: " << out_tran.out_script_len << endl;
        out << "\t\tOutput Script: ";
        for (int i = out_tran.out_script_len - 1; i >= 0; --i)
            out << HEX(out_tran.script_public_key[i], 1);
        out << endl;
        return out;
    }

    OutputTransaction::~OutputTransaction()
    {
        delete[] script_public_key;
    }

    Transaction::Transaction(const char* in, size_t &offset)
    {
        auto start = offset;
        version = *(uint32_t*)(in + offset);
        offset += 4;

        n_inputs = read_varint(in + offset, offset);
        for (int i = 0; i < n_inputs; ++i)
            inputs.emplace_back(in, offset);

        n_outputs = read_varint(in + offset, offset);
        for (int i = 0; i < n_outputs; ++i)
            outputs.emplace_back(in, offset);

        memcpy(&lock_time, in + offset, 4);
        offset += 4;

        this->transaction_hash = calculate_double_hash((void*)(in + start), offset - start);
    }

    std::ostream& operator<< (std::ostream& out, const Transaction& tran)
    {
        out << "\tVersion: " << tran.version << endl;

        out << "\tTransaction Hash (Calculated): ";
        for (int i = 31; i >= 0; --i)
            out << HEX(tran.transaction_hash[i], 1);

        out << endl << "\tNumber of inputs: " << tran.n_inputs << endl;
        for (int i = 0; i < tran.n_inputs; ++i)
            out << "\tInput " << i << ":" << endl << tran.inputs[i];

        out << "\tNumber of outputs: " << tran.n_outputs << endl;
        for (int i = 0; i < tran.n_outputs; ++i)
            out << "\tOutput " << i << ":" << endl << tran.outputs[i];

        out << "\tLock time: ";
        std::time_t temp = tran.lock_time;
        out << std::put_time(std::gmtime(&temp), "%Y-%m-%d %I:%M:%S %p") << std::endl;
        return out;
    }

    Transaction::~Transaction()
    {
        delete[] this->transaction_hash;
    }

    Block::Block(const char* in, size_t& offset)
    {
        memcpy(this, in + offset, 88);
        offset += 88;

        n_transactions = read_varint(in + offset, offset);

        for (int i = 0; i < n_transactions; ++i)
            transactions.emplace_back(in, offset);
    }

    uint8_t* Block::get_block_hash() const
    {
        return calculate_double_hash((void*)&header, sizeof(header));
    }

    uint8_t* Block::calculate_merkle_hash() const
    {
        const auto& sz = this->n_transactions;

        if (sz == 1)
        {
            auto ret = new uint8_t[32];
            memcpy(ret, this->transactions[0].transaction_hash, 32);
            return ret;
        }

        auto hashes_mem = new uint8_t[(sz + (sz % 2)) * 32];
        for (int i = 0 ; i < n_transactions; ++i)
            memcpy(hashes_mem + i * 32, transactions[i].transaction_hash, 32);

        if (sz % 2)
            memcpy(hashes_mem + sz * 32, transactions.back().transaction_hash, 32);

        auto cur_sz = (sz + (sz % 2));
        while (cur_sz > 1)
        {
            if (cur_sz % 2)
            {
                memcpy(hashes_mem + cur_sz * 32, hashes_mem + (cur_sz - 1) * 32, 32);
                cur_sz++;
            }

            for (int i = 0; i < cur_sz; i += 1)
            {
                auto hash = calculate_double_hash(hashes_mem + i * 64, 64);
                memcpy(hashes_mem + i * 32, hash, 32);
                delete[] hash;
            }

            cur_sz /= 2;
        }

        auto ret = new uint8_t[32];
        memcpy(ret, hashes_mem, 32);
        delete[] hashes_mem;
        return ret;
    }

    std::ostream& operator<< (std::ostream& out, const Block& block)
    {
        out << "Magic Number: " << HEX(block.magic_number, 4) << std::endl;
        out << "Blockchain Size: " << block.block_size << std::endl;
        out << "Header Info: " << std::endl;
        out << "\tVersion Number          : " << block.header.version_number << std::endl;
        out << "\tPrevious Hash           : ";
        for (int i = 31; i >= 0; --i)
            out << HEX(block.header.prev_hash[i], 1);
        out << std::endl << "\tMerkle Hash (Stored)    : ";
        for (int i = 31; i >= 0; --i)
            out << HEX(block.header.merkle_hash[i], 1);
        out << std::endl << "\tMerkle Hash (Calculated): ";

        auto calculated_merkle = block.calculate_merkle_hash();
        for (int i = 31; i >= 0; --i)
            out << HEX(calculated_merkle[i], 1);
        delete[] calculated_merkle;

        out << std::endl << "\tCurrent Hash            : ";
        auto res = block.get_block_hash();
        for (int i = 31; i >= 0; --i)
            out << HEX(res[i], 1);
        delete[] res;

        out << std::endl << "\tTime of Block Creation  : ";
        std::time_t temp = block.header.time;
        out << std::put_time(std::gmtime(&temp), "%Y-%m-%d %I:%M:%S %p") << std::endl;

        out << "\tDifficulty              : " << HEX(block.header.difficulty, 4) << std::endl;
        out << "\tNonce                   : " << block.header.nonce << std::endl;
        out << "Number of Transactions: " << block.n_transactions << std::endl;
        for (int i = 0; i < block.n_transactions; ++i)
            out << "Transaction " << i << ": " << endl << block.transactions[i];

        return out;
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
            int end = min(i * 16 + 15, (int)len - 1);
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