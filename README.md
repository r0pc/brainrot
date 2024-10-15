***Helloworld How ARE YOU***
```cpp
    #include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "tokenizer.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  //checks if input file is provided
  if (argc != 2) {
    cerr << "No File Provided" << endl;
    return EXIT_FAILURE;
  }

  string contents;
  //adds file contents to contents
  {
    stringstream contents_stream;
    fstream input(argv[1], ios::in);
    contents_stream<<input.rdbuf();
    contents = contents_stream.str();
  }

  return EXIT_SUCCESS;
}
```
