#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdint.h>
#include <string>
#include <vector>
#define ADD_FAILURE() std::cerr

class Cipher {
 public:
  virtual ~Cipher() {}
  bool Encrypt(const std::vector<uint8_t> &key, const uint8_t *iv,
               const uint8_t *src, uint8_t *dst, int nbytes) const {
    if (key.size() != keysize()) {
      ADD_FAILURE() << "Bad key size";
      return false;
    }
    return DoCrypt(key.data(), iv, src, dst, nbytes, true);
  }
  bool Decrypt(const std::vector<uint8_t> &key, const uint8_t *iv,
               const uint8_t *src, uint8_t *dst, int nbytes) const {
    if (key.size() != keysize()) {
      ADD_FAILURE() << "Bad key size";
      return false;
    }
    return DoCrypt(key.data(), iv, src, dst, nbytes, false);
  }
  virtual int keysize() const = 0;
  virtual int ivsize() const = 0;

 protected:
  virtual bool DoCrypt(const uint8_t *key, const uint8_t *iv,
                       const uint8_t *src, uint8_t *dst, int nbytes,
                       bool encrypt) const = 0;
};

// aes_256_xts.cpp

constexpr int kAesBlockSize = 16;
constexpr int kAes256KeySize = 32;
constexpr int kAes256XtsKeySize = 2 * kAes256KeySize;

class Aes256XtsCipher : public Cipher {
 public:
  int keysize() const { return kAes256XtsKeySize; }
  int ivsize() const { return kAesBlockSize; }

 private:
  bool DoCrypt(const uint8_t *key, const uint8_t *iv, const uint8_t *src,
               uint8_t *dst, int nbytes, bool encrypt) const;
};

