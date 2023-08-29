#include <string.h>
#include "aes_256_xts.h"
#include "Options.h"

int kDirectIOAlignment = 4096;
int kTestDataBytes = 4096;
int kCryptoSectorSize = 4096;


void dump(const std::string key, const std::vector<uint8_t> data){
  int cnt= 0;
  std::cout << key << std::endl;
  for(const uint8_t byte : data){
    if(cnt%16 == 0) {
      std::cout << std::endl;
      std::cout << " 0x" << std::setw(5) << std::setfill('0') << cnt/16 << "0  ";
    }
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " "; 
    cnt++;
  }
  std::cout << std::endl;
}

bool ReadBlockDevice(const std::string &blk_device, size_t count,
                            std::vector<uint8_t> *data) {
  std::cout << "Reading " << count << " bytes from " << blk_device << std::endl;
  std::unique_ptr<void, void (*)(void *)> buf_mem(
      malloc(count), free);

  if (buf_mem == nullptr) {
    std::cerr << "out of memory" << std::endl;
    return false;
  }
  uint8_t *buffer = static_cast<uint8_t *>(buf_mem.get());

  FILE *fd = 
      fopen(blk_device.c_str(), "r");
  if (fd < 0) {
    std::cerr << "Failed to open " << blk_device << std::endl;
    return false;
  }
  for(int i=0; i < (count-1)/kDirectIOAlignment + 1; i++){
    int rsize = fread(buffer + i * kDirectIOAlignment, 1, kDirectIOAlignment, fd);
    if(rsize > 0){
      if( rsize != kDirectIOAlignment) {
        //std::cerr << "read size is "<< rsize << ". Info: count is not align " << kDirectIOAlignment << std::endl;
      }
    }else{
      std::cerr << "Read failed";
      return false;
    }
  }

  *data = std::vector<uint8_t>(buffer, buffer + count);
  fclose(fd);
  return true;
}

std::string raw_blk_device_ = "raw.dat";
std::string dm_device_path_ = "dec.dat";
std::string key_file = "key.dat";
enum class Mode { UNKNOWN, ENCRYPT, DECRYPT };
int main(int argc, char** argv){
  Aes256XtsCipher cipher;
  Mode mode = Mode::UNKNOWN;

  std::vector<uint8_t> decrypted_data;
  std::vector<uint8_t> key;

  // Parse Args
  Option opts(argc, argv);

  if(opts.GetArg("r") != ""){
    mode = Mode::DECRYPT;
  }else if(opts.GetArg("d") != ""){
    mode = Mode::ENCRYPT;
  }
   
  kTestDataBytes = stoi(opts.GetArg("l","4096"));
  int sector = strtol(opts.GetArg("s", "0").c_str(), NULL , 0);
  std::cout << "Start Sector is " << sector << std::endl;
  //opts.GetArg("o", "");

  ReadBlockDevice(opts.GetArg("k"), cipher.keysize(), &key);
  //dump("key.dat", key);

  uint64_t* iv = (uint64_t*)malloc(cipher.ivsize());
  memset(iv, 0, cipher.ivsize());
  *iv = ((uint64_t)sector);

  if(mode == Mode::DECRYPT){
    std::vector<uint8_t> raw_data;
    std::vector<uint8_t> decrypted_data(kTestDataBytes);
    ReadBlockDevice(opts.GetArg("r"), kTestDataBytes, &raw_data);
    //dump("raw data", raw_data);

    for (size_t i = 0; i < kTestDataBytes; i += kCryptoSectorSize) {
      cipher.Decrypt(key, reinterpret_cast<const uint8_t *>(iv),
                               &raw_data[i], &decrypted_data[i],
                               kCryptoSectorSize);
      // Update the IV by incrementing the crypto sector number.
      *iv = ((uint64_t)(*iv) + 1);
    }
    
    dump("decrypt data", decrypted_data);
  }else if(mode == Mode::ENCRYPT){
    std::vector<uint8_t> decrypted_data;
    std::vector<uint8_t> encrypted_data(kTestDataBytes);
    ReadBlockDevice(opts.GetArg("d"), kTestDataBytes, &decrypted_data);
    //dump("decrypted data", decrypted_data);

    for (size_t i = 0; i < kTestDataBytes; i += kCryptoSectorSize) {
      cipher.Encrypt(key, reinterpret_cast<const uint8_t *>(iv),
                               &decrypted_data[i], &encrypted_data[i],
                               kCryptoSectorSize);
      // Update the IV by incrementing the crypto sector number.
      *iv = ((uint64_t)(*iv) + 1);
    }

    dump("Estimated enc data", encrypted_data);
  }
}
