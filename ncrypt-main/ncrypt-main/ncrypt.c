#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
//hehe :3
typedef struct {
        uint64_t n;
        uint64_t e;
    }pubKey64Bit;

    typedef struct{
        uint64_t n;
        uint64_t d;
    }privKey64Bit;

    typedef struct{
        uint32_t p;
        uint32_t q;
        __uint128_t n;
        uint64_t phi;
        uint64_t e;
        uint64_t d;
    }keyGenInfo;

    typedef struct{
        privKey64Bit privKey;
        pubKey64Bit pubKey;
    }RSAkeyInfo;

__uint128_t modexp(__uint128_t base, __uint128_t exponent, __uint128_t mod){
    uint64_t result = 1;
    base = base%mod;
    while(exponent>0){
        if(exponent & 1){
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent>>=1;
    }

    return result;

}

uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

uint64_t FindExponent(uint64_t phi){
    uint64_t e = 65537; // common exponent choice
    if (gcd(e, phi) == 1) {
        return e;
    }
    // Fallback: search for any odd e > 2
    for(e = 3; e < phi; e += 2) {
        if (gcd(e, phi) == 1) {
            return e;
        }
    }
    return -1;
}

uint64_t GenNum(){
    uint64_t num;
    
    int fd = open("/dev/urandom", O_RDONLY);
    if(fd== -1){
        perror("open");
        return -1;
    }
    
    
    if(read(fd,&num,sizeof(num)) != sizeof(num)){
        perror("read file failed");
        close(fd);
        return -1;
    } 
    
    
    close(fd);
    
    return num;
}

uint64_t Gen32BitOddNum(){
    uint32_t num;
    
    int fd = open("/dev/urandom", O_RDONLY);
    if(fd== -1){
        perror("open");
        return -1;
    }
    
    while(true){
        if(read(fd,&num,sizeof(num)) != sizeof(num)){
            perror("read file failed");
            close(fd);
            return -1;
        } 
        if(num % 2 != 0){
            
            break;
        }
    
    }
    close(fd);
    
    return num;
}

int IsProbPrime(uint64_t n, int k){
    if (n < 2) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0) return 0;

    uint64_t d = n - 1;
    int r = 0;
    while ((d % 2) == 0) {
        d /= 2;
        r++;
    }

    for (int i = 0; i < k; i++) {
        uint64_t a = GenNum() % (n - 4) + 2;
        if (a < 2 || a > n - 2) continue; // ensure valid base

        uint64_t x = modexp(a, d, n);
        if (x == 1 || x == n - 1) continue;

        int witness = 0;
        for (int j = 0; j < r - 1; j++) {
            x = modexp(x, 2, n);
            if (x == n - 1) {
                witness = 1;
                break;
            }
        }
        if (!witness) return 0; // composite
    }
    return 1; // probably prime
}

uint64_t ModInv(uint64_t a, uint64_t m) {
    uint64_t m0 = m, t, q;
    int64_t x0 = 0, x1 = 1; // Use signed types

    if (m == 1) return 0;

    while (a > 1) {
        q = a / m;
        t = m;

        m = a % m, a = t;
        t = x0;

        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) x1 += m0;

    return (uint64_t)x1;
}

uint32_t Gen32bitPrime(){
    while(true){
        uint64_t num = Gen32BitOddNum();
        if(IsProbPrime(num,80)){
            
            return num;
        }
        
    }
}

int EncryptionTest(RSAkeyInfo *keyInfo) {
    char sample = (char)(rand() % (126 - 32 + 1) + 32);
    uint64_t en = modexp((uint64_t)sample, keyInfo->pubKey.e, keyInfo->pubKey.n);
    uint64_t de = modexp(en, keyInfo->privKey.d, keyInfo->privKey.n);
    printf("char encrypted: %c\n", sample);
    printf("encrypted (as number): %" PRIu64 "\n", en);
    printf("decrypted (as number): %" PRIu64 "\n", de);
    printf("decrypted char: %c\n", (char)de);
    if((uint64_t)sample == de){
        printf("encryption test passed\n");
        return 0;
    }else{
        printf("encryption test failed\n");
        return -1;
    }
    
    return -1;
}

int GenSecretPrimes(keyGenInfo *genKey){
    int itrcount = 0;
    while(true){
    
        genKey->p = Gen32bitPrime();
        genKey->q = Gen32bitPrime();

        if(genKey->p != genKey->q){
            return 0;
        }
        if(itrcount > 100){
            return -1;
        }
        itrcount++;
    }
}

int GenKeyDetails(keyGenInfo *genKey){
    genKey->n = (__uint128_t)genKey->p * (__uint128_t)genKey->q;
    genKey->phi = (uint64_t)(genKey->p - 1) * (uint64_t)(genKey->q - 1);
    genKey->e = FindExponent(genKey->phi);
    if(genKey->e == -1){
        return -1;
    }
    genKey->d = ModInv(genKey->e, genKey->phi);
    return 0;
}

void ClearKeys(keyGenInfo *genKey, RSAkeyInfo *keyInfo) {
    
    genKey->p = 0;
    genKey->q = 0;
    genKey->n = 0;
    genKey->phi = 0;
    genKey->e = 0;
    genKey->d = 0;

    
    keyInfo->privKey.n = 0;
    keyInfo->privKey.d = 0;
    keyInfo->pubKey.n = 0;
    keyInfo->pubKey.e = 0;
}

int main(int argc , char *argv[]){
    srand(time(NULL));
    keyGenInfo genKey;
    RSAkeyInfo keyInfo;
    
    if(GenSecretPrimes(&genKey)<0){
        ClearKeys(&genKey, &keyInfo);
        return -1;
    }

    if(GenKeyDetails(&genKey)<0){
        ClearKeys(&genKey, &keyInfo);
        return -1;
    }
    
    keyInfo.privKey.d = genKey.d;
    keyInfo.privKey.n = genKey.n;
    keyInfo.pubKey.n = genKey.n;
    keyInfo.pubKey.e = genKey.e;

    printf("keys and info stored\n");

    if(EncryptionTest(&keyInfo)<0){
        ClearKeys(&genKey, &keyInfo);
        return -1;
    }

    

    return 0;
}
