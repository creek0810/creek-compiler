./compiler test.c > test.s
gcc test.s
./a.out
echo $?
