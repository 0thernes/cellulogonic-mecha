#include "cellulogonic_internal.h"

/* ============================================================================
 * Domain-Isolated Philox4x32-10 Counter-Based PRNG (Clean-Slate P4)
 * Guarantees bit-exact identical streams across Windows Clang & Linux GCC.
 * ============================================================================ */

#define PHILOX_M0 0xD2511F53u
#define PHILOX_M1 0xCD9E8D57u
#define PHILOX_W0 0x9E3779B9u
#define PHILOX_W1 0xBB67AE85u

static inline void philox4x32_10(uint32_t ctr[4], uint32_t key[2]) {
    uint32_t c0 = ctr[0], c1 = ctr[1], c2 = ctr[2], c3 = ctr[3];
    uint32_t k0 = key[0], k1 = key[1];
    for (int r = 0; r < 10; ++r) {
        uint64_t p0 = (uint64_t)PHILOX_M0 * (uint64_t)c0;
        uint64_t p1 = (uint64_t)PHILOX_M1 * (uint64_t)c2;
        uint32_t hi0 = (uint32_t)(p0 >> 32);
        uint32_t lo0 = (uint32_t)p0;
        uint32_t hi1 = (uint32_t)(p1 >> 32);
        uint32_t lo1 = (uint32_t)p1;
        uint32_t n0 = hi1 ^ c1 ^ k0;
        uint32_t n1 = lo1;
        uint32_t n2 = hi0 ^ c3 ^ k1;
        uint32_t n3 = lo0;
        c0 = n0; c1 = n1; c2 = n2; c3 = n3;
        k0 += PHILOX_W0;
        k1 += PHILOX_W1;
    }
    ctr[0] = c0; ctr[1] = c1; ctr[2] = c2; ctr[3] = c3;
}

uint64_t cell_philox_u64(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo) {
    uint32_t ctr[4];
    uint32_t key[2];
    ctr[0] = (uint32_t)(ctr_lo & 0xFFFFFFFFu);
    ctr[1] = (uint32_t)(ctr_lo >> 32);
    ctr[2] = (uint32_t)(ctr_hi & 0xFFFFFFFFu);
    ctr[3] = domain ^ (uint32_t)(ctr_hi >> 32);
    key[0] = (uint32_t)(seed & 0xFFFFFFFFu) ^ (domain * 0x9E3779B1u);
    key[1] = (uint32_t)(seed >> 32) ^ (domain * 0x85EBCA6Bu);
    philox4x32_10(ctr, key);
    return ((uint64_t)ctr[0] << 32) | (uint64_t)ctr[1];
}

uint32_t cell_philox_u32(uint64_t seed, uint32_t domain, uint64_t ctr_hi, uint64_t ctr_lo) {
    return (uint32_t)(cell_philox_u64(seed, domain, ctr_hi, ctr_lo) >> 32);
}

/* True Hardware Thermal Silicon Entropy via CPUID-guarded x86-64 RDSEED / RDRAND (for seed == 0 Feral Mode!) */
uint64_t cell_hw_rdseed_u64(void) {
    uint64_t val = 0;
#if defined(__x86_64__) || defined(_M_X64)
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0u), "c"(0u));
    uint32_t max_leaf = eax;
    int has_rdrand = 0, has_rdseed = 0;
    if (max_leaf >= 1u) {
        __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1u), "c"(0u));
        has_rdrand = (int)((ecx >> 30) & 1u);
    }
    if (max_leaf >= 7u) {
        __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(7u), "c"(0u));
        has_rdseed = (int)((ebx >> 18) & 1u);
    }
    unsigned char ok = 0;
    if (has_rdseed) {
        for (int tries = 0; tries < 32; ++tries) {
            __asm__ __volatile__("rdseed %0; setc %1" : "=r"(val), "=qm"(ok));
            if (ok && val != 0) return val;
        }
    }
    if (has_rdrand) {
        for (int tries = 0; tries < 32; ++tries) {
            __asm__ __volatile__("rdrand %0; setc %1" : "=r"(val), "=qm"(ok));
            if (ok && val != 0) return val;
        }
    }
    uint32_t lo = 0, hi = 0;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    val = (((uint64_t)hi << 32) | lo) ^ 0x9E3779B97F4A7C15ULL;
#else
    val = (uint64_t)(cell_wall_seconds() * 1e9) ^ 0xD1B54A32D192ED03ULL;
#endif
    return val ? val : 0xCAFEBABE12345678ULL;
}

/* ============================================================================
 * High-Resolution Wall Timer (Windows QPC / POSIX clock_gettime)
 * ============================================================================ */

double cell_wall_seconds(void) {
#if defined(_WIN32)
    int64_t cnt = 0, freq = 1;
    QueryPerformanceCounter(&cnt);
    QueryPerformanceFrequency(&freq);
    return (double)cnt / (double)(freq > 0 ? freq : 1);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

/* ============================================================================
 * NIST FIPS 180-4 SHA-256 (Zero External Dependencies)
 * ============================================================================ */

static const uint32_t SHA256_K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static inline uint32_t rotr32(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32u - n));
}

static void sha256_transform(CellSha256Ctx* ctx, const uint8_t blk[64]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)blk[i * 4] << 24) |
               ((uint32_t)blk[i * 4 + 1] << 16) |
               ((uint32_t)blk[i * 4 + 2] << 8) |
               ((uint32_t)blk[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr32(m[i - 15], 7) ^ rotr32(m[i - 15], 18) ^ (m[i - 15] >> 3);
        uint32_t s1 = rotr32(m[i - 2], 17) ^ rotr32(m[i - 2], 19) ^ (m[i - 2] >> 10);
        m[i] = m[i - 16] + s0 + m[i - 7] + s1;
    }
    uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
    uint32_t e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t t1 = h + S1 + ch + SHA256_K[i] + m[i];
        uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void cell_sha256_init(CellSha256Ctx* ctx) {
    ctx->datalen = 0;
    ctx->bitlen  = 0;
    ctx->state[0] = 0x6a09e667u; ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u; ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu; ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu; ctx->state[7] = 0x5be0cd19u;
}

void cell_sha256_update(CellSha256Ctx* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void cell_sha256_final(CellSha256Ctx* ctx, uint8_t out_hash[32]) {
    uint32_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += (uint64_t)ctx->datalen * 8ULL;
    for (int j = 0; j < 8; ++j) {
        ctx->data[63 - j] = (uint8_t)(ctx->bitlen >> (j * 8));
    }
    sha256_transform(ctx, ctx->data);
    for (int j = 0; j < 4; ++j) {
        for (int k = 0; k < 8; ++k) {
            out_hash[k * 4 + j] = (uint8_t)((ctx->state[k] >> (24 - j * 8)) & 0xFFu);
        }
    }
}

void cell_sha256_to_hex(const uint8_t hash[32], char hex[65]) {
    static const char HEX[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        hex[i * 2]     = HEX[(hash[i] >> 4) & 0xFu];
        hex[i * 2 + 1] = HEX[hash[i] & 0xFu];
    }
    hex[64] = '\0';
}

/* ============================================================================
 * Silicon CPUID Probe (x86-64)
 * ============================================================================ */

static void run_cpuid(uint32_t leaf, uint32_t subleaf, uint32_t out[4]) {
#if defined(__x86_64__) || defined(_M_X64)
    __asm__ __volatile__(
        "cpuid"
        : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
        : "a"(leaf), "c"(subleaf)
    );
#else
    out[0] = out[1] = out[2] = out[3] = 0;
#endif
}

CELL_API void cellulogonic_probe_hw(CellHwCaps* out_caps) {
    if (!out_caps) return;
    memset(out_caps, 0, sizeof(*out_caps));
    uint32_t r[4] = {0};
    run_cpuid(0, 0, r);
    uint32_t max_leaf = r[0];
    memcpy(out_caps->vendor + 0, &r[1], 4);
    memcpy(out_caps->vendor + 4, &r[3], 4);
    memcpy(out_caps->vendor + 8, &r[2], 4);
    out_caps->vendor[12] = '\0';

    if (max_leaf >= 1) {
        run_cpuid(1, 0, r);
        out_caps->has_popcnt = (r[2] >> 23) & 1u;
    }
    if (max_leaf >= 7) {
        run_cpuid(7, 0, r);
        out_caps->has_bmi1   = (r[1] >> 3) & 1u;
        out_caps->has_avx2   = (r[1] >> 5) & 1u;
        out_caps->has_bmi2   = (r[1] >> 8) & 1u;
        out_caps->has_rdseed = (r[1] >> 18) & 1u;
    }
    run_cpuid(0x80000000u, 0, r);
    uint32_t max_ext = r[0];
    if (max_ext >= 0x80000001u) {
        run_cpuid(0x80000001u, 0, r);
        out_caps->has_rdtscp = (r[3] >> 27) & 1u;
    }
    if (max_ext >= 0x80000004u) {
        for (uint32_t i = 0; i < 3; ++i) {
            run_cpuid(0x80000002u + i, 0, r);
            memcpy(out_caps->brand + i * 16, r, 16);
        }
        out_caps->brand[48] = '\0';
    } else {
        snprintf(out_caps->brand, sizeof(out_caps->brand), "Generic x86-64");
    }
}
