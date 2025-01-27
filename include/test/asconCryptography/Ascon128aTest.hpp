#include "asconCryptography/Ascon128a.hpp"
#include "setupConfiguration/utils.hpp"

namespace Test
{
class Ascon128aTest : public Cryptography::Ascon128a
{
private:

public:
    Ascon128aTest();
    ~Ascon128aTest();
    static std::shared_ptr<Ascon128aTest> create();
    static void RunAscon128aTest();
};
} // namespace Test