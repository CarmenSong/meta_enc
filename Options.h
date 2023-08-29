#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <map>

class Option{
  private:
    std::map<const std::string, const std::string> optKeyValuePair;
    void Parse(const std::size_t argc, const std::string* const options);
  public:
    Option(std::size_t argc, char** argv);
    ~Option(){}
    std::string GetArg(const std::string key, const std::string def = "");
};
