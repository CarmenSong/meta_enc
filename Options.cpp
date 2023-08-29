#include "Options.h"

void Option::Parse(const std::size_t argc, const std::string* const options){
  for(std::size_t i = 0; i <argc; i++){
    if(options[i][0] == '-'){
          optKeyValuePair.insert(std::map<std::string, std::string>::value_type(options[i].substr(1,1), options[i+1]));
    }
  }
}

Option::Option(std::size_t argc, char** argv){
  std::string* options = new std::string[argc];
  for(std::size_t i = 0; i < argc; i++){
    options[i] = argv[i];
  }
  Parse(argc, options);
  delete[] options;
}

std::string Option::GetArg(const std::string key, const std::string def){
  auto found = optKeyValuePair.find(key);
  if(found == end(optKeyValuePair)){
    return def;
  }
  return found->second;
}
