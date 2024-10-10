INCLUDE_PATH=.././include
SRC_PATH=.././src

rm -f test
rm -f a.out
g++ -g -I$INCLUDE_PATH  -std=c++11 $SRC_PATH/file_writer.cpp ./main.cpp -o a.out 
gdb ./a.out
