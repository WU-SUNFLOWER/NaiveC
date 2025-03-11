#/bin/zsh

cd build
ninja
cd ..
./bin/lexer-test
./bin/parser-test
./bin/codegen-test