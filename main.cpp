#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <string.h>
#include "dm.h"

#define ADD_FAILURE() std::cerr
#define GTEST_LOG_(x) std::cout

using namespace android::dm;

constexpr int kAes256KeySize = 32;
constexpr int kAes256XtsKeySize = 2 * kAes256KeySize;
int kCryptoSectorSize = 4096;
//int kTestDataSectors = 256;
int kTestDataSectors = 4096;
int kTestDataBytes = kTestDataSectors * kCryptoSectorSize;
int kDmApiSectorSize = 512;

std::string test_dm_device_name_;
std::string raw_blk_device_;
std::string dm_device_path_;
DeviceMapper *dm_;

std::string BytesToHex(const std::vector<uint8_t> &bytes) {
  std::ostringstream o;
  for (uint8_t b : bytes) {
    o << std::hex << std::setw(2) << std::setfill('0') << (int)b;
  }
  return o.str();
}

void RandomBytesForTesting(std::vector<uint8_t> &bytes) {
  for (size_t i = 0; i < bytes.size(); i++) {
    bytes[i] = rand();
  }
}

std::vector<uint8_t> GenerateTestKey(size_t size) {
  std::vector<uint8_t> key(size);
  RandomBytesForTesting(key);
  return key;
}

void SetUp() {
  dm_ = &DeviceMapper::Instance();

  test_dm_device_name_ = 
     "DmDefaultKeyTest";

  if(raw_blk_device_.empty()){
    std::cout << "Use /dev/sdb1 for block device because block device is not input." << std::endl;
    raw_blk_device_ = "/dev/sdb1";
  }

  dm_->DeleteDeviceIfExists(test_dm_device_name_.c_str());
}

// Creates the test dm-default-key mapping using the given key and settings.
// If the dm device creation fails, then it is assumed the kernel doesn't
// support the given encryption settings, and a failure is not added.
bool CreateTestDevice(const std::string &cipher,
                                        const std::vector<uint8_t> &key,
                                        bool is_wrapped_key) {
//  static_assert(kTestDataBytes % kDmApiSectorSize == 0);
  std::unique_ptr<DmTargetDefaultKey> target =
      std::make_unique<DmTargetDefaultKey>(0, kTestDataBytes / kDmApiSectorSize,
                                           cipher.c_str(), BytesToHex(key),
                                           raw_blk_device_, 0);
  target->SetSetDun();
  if (is_wrapped_key) target->SetWrappedKeyV0();

  DmTable table;
  if (!table.AddTarget(std::move(target))) {
    ADD_FAILURE() << "Failed to add default-key target to table";
    return false;
  }
  if (!table.valid()) {
    ADD_FAILURE() << "Device-mapper table failed to validate";
    return false;
  }
  if (!dm_->CreateDevice(test_dm_device_name_, table, &dm_device_path_,
                         std::chrono::seconds(0))) {
    GTEST_LOG_(INFO) << "Unable to create default-key mapping" << " <errono>  "
                     << ".  Assuming that the encryption settings cipher=\""
                     << cipher << "\", is_wrapped_key=" << is_wrapped_key
                     << " are unsupported and skipping the test.";
    return false;
  }
  GTEST_LOG_(INFO) << "Created default-key mapping at " << dm_device_path_
                   << " using cipher=\"" << cipher
                   << "\", key=" << BytesToHex(key)
                   << ", is_wrapped_key=" << is_wrapped_key;
  return true;
}



int main(int argc, char *argv[]){
  std::vector<uint8_t> key;
  int opt;
  opterr = 0;
  while((opt = getopt(argc, argv, "d:k:")) != -1){
    switch(opt){
      case 'd':
        {
          raw_blk_device_ = std::string(optarg);;
          break;
        }
      case 'k':
        {
          std::ifstream fin( optarg, std::ios::in | std::ios::binary );
          if(!fin){
            std::cout << "cannot open " << optarg << std::endl;
            return 1;
          }
          key = std::vector<uint8_t>((std::istreambuf_iterator<char>(fin)), (std::istreambuf_iterator<char>()));
          if(key.size() != kAes256XtsKeySize){
            std::cout << "input key needs 64byte. but your key is " << key.size() << "byte." << std::endl;
            key = std::vector<uint8_t>();
          }
          break;
        }
      default:
        std::cout << "Usage: " << argv[0] << " [-d raw_blk_device] [-k key]" << std::endl;
        return 0;
    }
  }
  SetUp();

  if(key.empty()){
    std::cout << "Use random key because correct length key is not input." << std::endl;
    key = GenerateTestKey(kAes256XtsKeySize);
    std::string key_string = std::string(key.begin(), key.end());
    std::cout << key_string << std::endl;
  }

  if(!CreateTestDevice("aes-xts-plain64", key, false)) {
    std::cerr << "CreateTestDevice Failed";
    return -1;
  }

  std::cout << "CreatedTestDevice" ;

  return 0;

}
