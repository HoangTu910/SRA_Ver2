// #include "test/asconCryptography/Ascon128aTest.hpp"
// #include <cstring>

// namespace Test
// {
// std::shared_ptr<Ascon128aTest> Test::Ascon128aTest::create()
// {
//     return std::make_shared<Ascon128aTest>();
// }

// void Ascon128aTest::RunAscon128aTest()
// {
//     auto ascon128a = Ascon128aTest::create();
//     unsigned char fixedKey[ASCON_KEY_SIZE] = {
//         0x01, 0x02, 0x03, 0x04,
//         0x05, 0x06, 0x07, 0x08,
//         0x09, 0x0A, 0x0B, 0x0C,
//         0x0D, 0x0E, 0x0F, 0x10
//     };
//     ascon128a->setKey(fixedKey);

//     unsigned char fixedNonce[ASCON_NONCE_SIZE] = {
//         0xFF, 0xEE, 0xDD, 0xCC,
//         0xBB, 0xAA, 0x99, 0x88,
//         0x77, 0x66, 0x55, 0x44,
//         0x33, 0x22, 0x11, 0x00
//     };
//     ascon128a->setNonce(fixedNonce);

//     unsigned char fixedPlainText[16] = {
//         0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
//         0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30
//     };

//     ascon128a->setPlainText(fixedPlainText, 16);
    
//     ascon128a->encrypt();
//     auto cipherText = ascon128a->getCipherText();
//     auto decryptText = ascon128a->decrypt();
//     bool testResult = false;
//     for (int i = 0; i < 16; i++)
//     {
//         if (ascon128a->m_plainText[i] != decryptText[i])
//         {
//             break;
//         }
//         testResult = true;
//     }
//     TEST_RESULT(testResult);
// }

// Ascon128aTest::Ascon128aTest() {
    
// }

// Ascon128aTest::~Ascon128aTest()
// {

// }
// } // namespace Test