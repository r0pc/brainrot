#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "generator.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

using namespace std;

void check_file_type(string filename);

int main(int argc, char *argv[]) {
  // checks if input file is provided
  if (argc != 2) {
    cerr << "No File Provided" << endl;
    return EXIT_FAILURE;
  }

  check_file_type(argv[1]);

  string contents;
  // adds file contents to contents
  {
    stringstream contents_stream;
    fstream input(argv[1], ios::in);
    contents_stream << input.rdbuf();
    contents = contents_stream.str();
  }

  cout << contents << endl;

  Tokenizer tokenizer(std::move(contents));
  vector<Token> tokens = tokenizer.tokenize();

  Parser parser(std::move(tokens));
  optional<NodeProg> prog = parser.parse_prog();

  if (!prog) {
    cerr << "Invalid Program" << endl;
    exit(EXIT_FAILURE);
  }

  {
    Generator generator(prog.value());
    fstream file("out.asm", ios::out);
    file << generator.gen_prog();
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");

  return EXIT_SUCCESS;
}

void check_file_type(string filename) {
  if (filename.length() >= 4 &&
      filename.substr(filename.length() - 4) == ".rot") {
    return;
  }

  cerr << "Incorrect file" << endl;
  exit(EXIT_FAILURE);
}
