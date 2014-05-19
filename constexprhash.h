#pragma once

#include <stdint.h>

constexpr uint32_t djb2a(const char *s, uint32_t h = 5381) {
	return !*s ? h : djb2a(s + 1, 33 * h ^ *s);
}

constexpr uint32_t fnv1a(const char *s, uint32_t h = 0x811C9DC5) {
	return !*s ? h : fnv1a(s + 1, (h ^ *s) * 0x01000193);
}

constexpr uint32_t CRC32_TABLE[] = {
	0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
	0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C};
constexpr uint32_t crc32_4(char c, uint32_t h) {
	return (h >> 4) ^ CRC32_TABLE[(h & 0xF) ^ c];
}
constexpr uint32_t crc32(const char *s, uint32_t h = ~0) {
	return !*s ? ~h : crc32(s + 1, crc32_4(*s >> 4, crc32_4(*s & 0xF, h)));
}

namespace MurmurHash3 {
constexpr uint32_t rotl(uint32_t x, int8_t r) {
	return (x << r) | (x >> (32 - r));
}
constexpr uint32_t kmix(uint32_t k) {
	return rotl(k * 0xCC9E2D51, 15) * 0x1B873593;
}
constexpr uint32_t hmix(uint32_t h, uint32_t k) {
	return rotl(h ^ kmix(k), 13) * 5 + 0xE6546B64;
}
constexpr uint32_t shlxor(uint32_t x, int8_t l) {
	return (x >> l) ^ x;
}
constexpr uint32_t fmix(uint32_t h) {
	return shlxor(shlxor(shlxor(h, 16) * 0x85EBCA6B, 13) * 0xC2B2AE35, 16);
}
constexpr uint32_t body(const char *s, size_t n, uint32_t h) {
	return n < 4 ? h : body(s + 4, n - 4, hmix(h, s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24)));
}
constexpr uint32_t tail(const char *s, size_t n, uint32_t h) {
	return h ^ kmix(n == 3 ? s[0] | (s[1] << 8) | (s[2] << 16) : n == 2 ? s[0] | (s[1] << 8) : n == 1 ? s[0] : 0);
}
constexpr uint32_t shash(const char *s, size_t n, uint32_t seed) {
	return fmix(tail(s + (n & ~3), n & 3, body(s, n, seed)) ^ n);
}
} // namespace MurmurHash3

constexpr uint32_t operator"" _H(const char *s, size_t size) {
	return MurmurHash3::shash(s, size, 0);
}
