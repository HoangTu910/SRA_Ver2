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
    std::vector<unsigned char> m_cipherText = {};
    std::vector<unsigned char> m_plainText = {};
    std::vector<unsigned char> m_associatedData = {};
    std::vector<unsigned char> m_nonce = {};
    std::vector<unsigned char> m_key = {};
    std::vector<unsigned char> m_decryptText = {};
    std::vector<unsigned char> m_presharedSecretKey = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}; 

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

    std::vector<unsigned char> getPlainText();

    /**
     * @brief Get the associated data for encryption
     * @return The associated data
     */
    std::vector<unsigned char> getAssociatedData();
    /**
     * @brief Get nonce
     */
    std::vector<unsigned char> getNonce();

    /**
     * @brief Get the preshared secret key
     * @return The preshared secret key
     */
    std::vector<unsigned char> getPresharedSecretKey();

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
     * @brief Get the authentication tag from ciphertext
     * @return The authentication tag
     */
    std::vector<unsigned char> getAuthTagFromCipherText();

    /**
     * @brief get ciphertext length
     */
    size_t getCipherTextLength();

    /**
     * @brief Smart pointer to create Ascon128a object
     * @return A shared pointer to a new Ascon128a object
     */
    static std::shared_ptr<Cryptography::Ascon128a> create();

    /**
     * @brief Set the associated data for encryption
     * @param associatedData The associated data to set
     */
    void setAssociatedData(const std::vector<unsigned char>& associatedData);
};
} // namespace Cryptography
