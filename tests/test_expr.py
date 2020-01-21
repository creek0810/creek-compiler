import os


def compile_and_run(test_data):
    with open("test.c", "w") as file:
        file.write(test_data)
    os.system("./compiler test.c > test.s")
    os.system("gcc -o test test.s")
    result = os.system("./test")
    return result >> 8


def test_logic(compiler):
    # and
    result = compile_and_run("1&&2;")
    assert result == 1
    result = compile_and_run("1&&1;")
    assert result == 1
    result = compile_and_run("1&&0;")
    assert result == 0
    result = compile_and_run("0 && 0;")
    assert result == 0
    # or
    result = compile_and_run("1||2;")
    assert result == 1
    result = compile_and_run("1||1;")
    assert result == 1
    result = compile_and_run("1||0;")
    assert result == 1
    result = compile_and_run("0||0;")
    assert result == 0


def test_bit(compiler):
    # and
    result = compile_and_run("5&1;")
    assert result == 1
    result = compile_and_run("4&1;")
    assert result == 0
    result = compile_and_run("7&5;")
    assert result == 5
    # or
    result = compile_and_run("0|5;")
    assert result == 5
    result = compile_and_run("2|5;")
    assert result == 7
    # xor
    result = compile_and_run("2^5;")
    assert result == 7
    result = compile_and_run("7^5;")
    assert result == 2


def test_arithmetic(compiler):
    result = compile_and_run("5+10;")
    assert result == 15
    result = compile_and_run("10-4;")
    assert result == 6
    result = compile_and_run("5*10;")
    assert result == 50
    result = compile_and_run("20/10;")
    assert result == 2
    result = compile_and_run("10/10;")
    assert result == 1
    result = compile_and_run("10%2;")
    assert result == 0
    result = compile_and_run("11%2;")
    assert result == 1

    # complex test
    result = compile_and_run("2+5*2;")
    assert result == 12
    result = compile_and_run("(2+5)*2;")
    assert result == 14


def test_cmp(compiler):
    result = compile_and_run("8<2;")
    assert result == 0
    result = compile_and_run("8 > 2;")
    assert result == 1
    result = compile_and_run("8 > 8;")
    assert result == 0
    result = compile_and_run("8 >= 8;")
    assert result == 1
    result = compile_and_run("8 < 8;")
    assert result == 0
    result = compile_and_run("8 <= 8;")
    assert result == 1
    result = compile_and_run("8 == 8;")
    assert result == 1
    result = compile_and_run("8 == 9;")
    assert result == 0
    result = compile_and_run("8 != 9;")
    assert result == 1
    result = compile_and_run("8 != 8;")
    assert result == 0


def test_shift(compiler):
    result = compile_and_run("8 >> 1;")
    assert result == 4
    result = compile_and_run("8 << 2;")
    assert result == 32
    result = compile_and_run("3 >> 1;")
    assert result == 1
    result = compile_and_run("5 << 1")
    assert result == 10
