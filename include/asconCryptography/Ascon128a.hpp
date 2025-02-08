#pragma once

#include "asconEncryptionHelper/asconPermutations.hpp"
#include "asconEncryptionHelper/asconCore.hpp"
#include "asconEncryptionHelper/asconEncrypt.hpp"
#include "asconEncryptionHelper/asconDecrypt.hpp"
#include "asconEncryptionHelper/asconConstants.hpp"
#include "asconEncryptionHelper/asconPrintstate.hpp"

#include <memory>

namespace Cryptography
{
class Ascon128a
{
protected:
    unsigned char *m_cipherText;
    unsigned long long m_cipherTextLength;
    unsigned char *m_plainText;
    unsigned long long m_plainTextLength;
    const unsigned char *m_associatedData;
    unsigned long long m_associatedDataLength;
    unsigned char *m_nonce;
    unsigned char *m_key;

    std::shared_ptr<Cryptography::Ascon128a> m_ascon128a;
public:
    Ascon128a();
    ~Ascon128a();

    /**
     * @brief Set the plain text to encrypt
     * @param plainText The plain text to encrypt
     * @param plainTextLength The length of the plain text
     */
    void setPlainText(unsigned char *plainText, unsigned long long plainTextLength);

    /**
     * @brief Get the cipher text after encryption
     * @return The cipher text
     */
    unsigned char *getCipherText();

    /**
     * @brief Get nonce
     */
    unsigned char *getNonce();

    /**
     * @brief Set the key for encryption
     */
    void setKey(unsigned char *key);

    /**
     * @brief Set the nonce for encryption
     */
    void setNonce();

    /**
     * @brief Set the nonce for encryption
     * @param nonce The nonce to set
     */
    void setNonce(unsigned char *nonce);

    /**
     * @brief Encrypt the data using Ascon128a
     */
    void encrypt();

    /**
     * @brief Decrypt the data using Ascon128a
     */
    unsigned char *decrypt();

    /**
     * @brief get ciphertext lenght
     */
    unsigned long long getCipherTextLenght();

    /**
     * @brief Smart pointer to create Ascon128a object
     * @return A shared pointer to a new Ascon128a object
     */
    static std::shared_ptr<Cryptography::Ascon128a> create();
};
} // namespace Cryptography
