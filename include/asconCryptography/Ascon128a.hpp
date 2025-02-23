#pragma once

#include "asconEncryptionHelper/asconPermutations.hpp"
#include "asconEncryptionHelper/asconCore.hpp"
#include "asconEncryptionHelper/asconEncrypt.hpp"
#include "asconEncryptionHelper/asconDecrypt.hpp"
#include "asconEncryptionHelper/asconConstants.hpp"
#include "asconEncryptionHelper/asconPrintstate.hpp"

#include <memory>
#include <vector>

namespace Cryptography
{
class Ascon128a
{
protected:
    std::vector<unsigned char> m_cipherText;
    std::vector<unsigned char> m_plainText;
    std::vector<unsigned char> m_associatedData;
    std::vector<unsigned char> m_nonce;
    std::vector<unsigned char> m_key;
    std::vector<unsigned char> m_decryptText;

    std::shared_ptr<Cryptography::Ascon128a> m_ascon128a;
public:
    Ascon128a();
    ~Ascon128a();

    /**
     * @brief Set the plain text to encrypt
     * @param plainText The plain text to encrypt
     * @param plainTextLength The length of the plain text
     */
    void setPlainText(const std::vector<unsigned char>& plainText);

    /**
     * @brief Get the cipher text after encryption
     * @return The cipher text
     */
    std::vector<unsigned char> getCipherText();

    /**
     * @brief Get nonce
     */
    std::vector<unsigned char> getNonce();

    /**
     * @brief Set the key for encryption
     */
    void setKey(const std::vector<unsigned char>& key);

    /**
     * @brief Set the nonce for encryption
     */
    void setNonce();

    /**
     * @brief Set the nonce for encryption
     * @param nonce The nonce to set
     */
    void setNonce(const std::vector<unsigned char>& nonce);

    /**
     * @brief Encrypt the data using Ascon128a
     */
    void encrypt();

    /**
     * @brief Decrypt the data using Ascon128a
     */
    std::vector<unsigned char> decrypt();

    /**
     * @brief get ciphertext length
     */
    size_t getCipherTextLength();

    /**
     * @brief Smart pointer to create Ascon128a object
     * @return A shared pointer to a new Ascon128a object
     */
    static std::shared_ptr<Cryptography::Ascon128a> create();
};
} // namespace Cryptography
