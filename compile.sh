clang -Wall -c -gdwarf-4 -O0 ./main.c 
clang -Wall -c -gdwarf-4 -O0 ./lexer.c 
clang -Wall -c -gdwarf-4 -O0 ./dynamic_string.c 
clang -Wall -c -gdwarf-4 -O0 ./parser.c 
clang -Wall -c -gdwarf-4 -O0 ./code_generation.c 
clang -Wall -c -gdwarf-4 -O0 ./colors.c

clang colors.o code_generation.o parser.o dynamic_string.o lexer.o main.o -Wall -gdwarf-4 -O0 -o acc

rm dynamic_string.o
rm lexer.o
rm main.o
rm parser.o
rm code_generation.o
rm colors.o
