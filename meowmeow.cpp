#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <chrono>
#include <utility>
#include "constexprhash.h"

// dummy check that hashes is compile time constants
void check(uint32_t h) {
    switch (h) {
    case djb2a("123456789"):
    case fnv1a("123456789"):
    case crc32("123456789"):
    case "123456789"_H:
        break;
    }
}

// regular version of murmur hash 3
namespace MurmurHash3 {
constexpr uint32_t body(const uint32_t *k, size_t n, uint32_t h) {
    return n < 1 ? h : body(k + 1, n - 1, hmix(h, *k));
}
} // namespace MurmurHash3

uint32_t murmur3(const void *p, size_t size, uint32_t seed) {
    using namespace MurmurHash3;
    uint32_t h = seed;
    h = body(static_cast<const uint32_t *>(p), size / 4, h);
    h = tail(static_cast<const char *>(p) + (size & ~3), size & 3, h);
    return fmix(h ^ size);
}

// simple hash map with linear lookup
template <typename T, size_t N>
struct dict {
    uint32_t keys[N] = {};
    T values[N];

    T &operator[](uint32_t key) {
        return setdefault(key, T());
    }

    T &setdefault(uint32_t key, T &&defval) {
        auto i = key % N;
        for (; keys[i]; i = (i + 1) % N)
            if (keys[i] == key)
                return values[i];
        keys[i] = key;
        return values[i] = std::move(defval);
    }

    void remove(uint32_t key) {
        auto i = key % N;
        for (; keys[i] != key; i = (i + 1) % N)
            if (keys[i] == 0)
                return;
        for (auto j = (i + 1) % N; keys[j]; j = (j + 1) % N) {
            auto k = keys[j] % N;
            if (i <= j ? (i >= k) || (k > j) : (i >= k) || (k > j)) {
                keys[i] = keys[j];
                values[i] = std::move(values[j]);
                i = j;
            }
        }
        keys[i] = 0;
    }

    const T &lookup(uint32_t key, const T &defval) const {
        for (auto i = key % N; keys[i]; i = (i + 1) % N)
            if (keys[i] == key)
                return values[i];
        return defval;
    }

    const T *find(uint32_t key) const {
        for (auto i = key % N; keys[i]; i = (i + 1) % N)
            if (keys[i] == key)
                return values + i;
        return nullptr;
    }
};

int main(int argc, char **argv) {
    uint32_t seed = 0;
    int benchmark = 0;
    int hex = 0;

    int ch;
    while ((ch = getopt(argc, argv, "s:b:h")) != -1) {
        switch (ch) {
        case 's': {
            char *endptr;
            seed = strtol(optarg, &endptr, 0);
            if (*endptr) {
                fprintf(stderr, "%s: bad seed value -- %s\n", argv[0], optarg);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 'b':
            benchmark = strtol(optarg, nullptr, 0);
            break;
        case 'h':
            hex = 1;
            break;
        case '?':
        default:
            fprintf(stderr, "usage: %s [-s seed] [-b count] [-h] [strings ...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (benchmark) {
        size_t size = benchmark * (1 << 20);
        void *p = malloc(size);
        memset(p, rand() % 64, size);
        auto start = std::chrono::high_resolution_clock::now();
        auto hash = murmur3(p, size, seed);
        auto end = std::chrono::high_resolution_clock::now();
        free(p);
        std::chrono::duration<double> elapsed = end - start;
        fprintf(stderr, "benchmark hashing: %d MiB (%#.8x)\nelapsed time: %f (%.2f MiB/sec)\n", benchmark, hash, elapsed.count(), size / elapsed.count() / (1 << 20));
    }

    dict<const char *, 1 << 16> map;
    for (int arg = optind; arg < argc; ++arg) {
        uint32_t h = murmur3(argv[arg], strlen(argv[arg]), seed);
        printf(hex ? "%#.8x %s\n" : "%10u %s\n", h, argv[arg]);
        if (map.find(h)) {
            if (strcmp(map[h], argv[arg]))
                printf("! %s %s\n", map[h], argv[arg]);
            continue;
        }
        map[h] = argv[arg];
    }

    return 0;
}
