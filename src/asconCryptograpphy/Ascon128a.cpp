#include "asconCryptography/Ascon128a.hpp"

Cryptography::Ascon128a::Ascon128a()
{
    m_nonce.resize(ASCON_NONCE_SIZE);
    m_associatedData = std::vector<unsigned char>(ASCON_ASSOCIATED_DATA, ASCON_ASSOCIATED_DATA + AsconMagicNumber::ASCON_ASSOCIATED_DATALENGTH);
}

Cryptography::Ascon128a::~Ascon128a()
{
    // No need for manual deletion with vectors
}

std::vector<unsigned char> Cryptography::Ascon128a::getCipherText()
{
    return m_cipherText;
}

std::vector<unsigned char> Cryptography::Ascon128a::getNonce()
{
    return m_nonce;
}

std::vector<unsigned char> Cryptography::Ascon128a::getPresharedSecretKey()
{
    return m_presharedSecretKey;
}

void Cryptography::Ascon128a::setPlainText(const std::vector<unsigned char>& plainText)
{
    m_plainText = plainText;
}

void Cryptography::Ascon128a::setKey(const std::vector<unsigned char>& key)
{
    m_key = key;
}

void Cryptography::Ascon128a::setNonce()
{
    m_nonce.resize(ASCON_NONCE_SIZE);
    Ascon::generate_nonce(m_nonce.data());
}

void Cryptography::Ascon128a::setNonce(const std::vector<unsigned char>& nonce)
{
    m_nonce = nonce;
}

void Cryptography::Ascon128a::encrypt()
{
    size_t plainTextLength = m_plainText.size();
    m_cipherText.resize(plainTextLength + ASCON_TAG_SIZE);
    unsigned long long cipherTextLength;
    
    Ascon::crypto_aead_encrypt(
        m_cipherText.data(),
        &cipherTextLength,
        m_plainText.data(),
        plainTextLength,
        m_associatedData.data(),
        m_associatedData.size(),
        nullptr,
        m_nonce.data(),
        m_key.data()
    );
}

std::vector<unsigned char> Cryptography::Ascon128a::decrypt()
{
    std::vector<unsigned char> decryptText(m_cipherText.size() - ASCON_TAG_SIZE);
    unsigned long long plainTextLength;
    
    Ascon::crypto_aead_decrypt(
        decryptText.data(),
        &plainTextLength,
        nullptr,
        m_cipherText.data(),
        m_cipherText.size(),
        m_associatedData.data(),
        m_associatedData.size(),
        m_nonce.data(),
        m_key.data()
    );
    
    decryptText.resize(plainTextLength);
    return decryptText;
}

size_t Cryptography::Ascon128a::getCipherTextLength()
{
    return m_cipherText.size();
}

std::shared_ptr<Cryptography::Ascon128a> Cryptography::Ascon128a::create()
{
    return std::make_shared<Cryptography::Ascon128a>();
}

std::vector<unsigned char> Cryptography::Ascon128a::getAuthTagFromCipherText()
{
    return std::vector<unsigned char>(m_cipherText.end() - ASCON_TAG_SIZE, m_cipherText.end());
}

void Cryptography::Ascon128a::setAssociatedData(const std::vector<unsigned char>& associatedData)
{
    m_associatedData = associatedData;
}

std::vector<unsigned char> Cryptography::Ascon128a::getAssociatedData()
{
    return m_associatedData;
}