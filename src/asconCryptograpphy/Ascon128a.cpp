#include "asconCryptography/Ascon128a.hpp"

Cryptography::Ascon128a::Ascon128a()
{
    m_associatedData = reinterpret_cast<const unsigned char*>(ASCON_ASSOCIATED_DATA);;
    m_associatedDataLength = AsconMagicNumber::ASCON_ASSOCIATED_DATALENGTH;
}

Cryptography::Ascon128a::~Ascon128a()
{
}

unsigned char *Cryptography::Ascon128a::getCipherText()
{
    return m_cipherText;
}

void Cryptography::Ascon128a::setPlainText(unsigned char *plainText, unsigned long long plainTextLength)
{
    m_plainText = plainText;
    m_plainTextLength = plainTextLength;
}

void Cryptography::Ascon128a::setKey(unsigned char *key)
{
    m_key = key;
}

void Cryptography::Ascon128a::setNonce(unsigned char *nonce)
{
    m_nonce = nonce;
}

void Cryptography::Ascon128a::encrypt()
{
    m_cipherText = new unsigned char[m_plainTextLength + ASCON_TAG_SIZE];
    Ascon::crypto_aead_encrypt(m_cipherText, &m_cipherTextLength, m_plainText, m_plainTextLength, m_associatedData, m_associatedDataLength, nullptr, m_nonce, m_key);
}

unsigned char *Cryptography::Ascon128a::decrypt()
{
    unsigned char* decryptText = new unsigned char[m_cipherTextLength - ASCON_TAG_SIZE];
    Ascon::crypto_aead_decrypt(decryptText, &m_plainTextLength, nullptr, m_cipherText, m_cipherTextLength, m_associatedData, m_associatedDataLength, m_nonce, m_key);
    return decryptText;
}

std::shared_ptr<Cryptography::Ascon128a> Cryptography::Ascon128a::create()
{
    return std::make_shared<Cryptography::Ascon128a>();
}