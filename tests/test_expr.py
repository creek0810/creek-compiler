import os


def compile_and_run(test_data):
    with open("test.c", "w") as file:
        file.write(test_data)
    os.system("./compiler test.c > test.s")
    os.system("gcc -o test test.s")
    result = os.system("./test")
    return result >> 8


def test_arithmetic(compiler):
    result = compile_and_run("5+10;")
    assert result == 15
    result = compile_and_run("10-4;")
    assert result == 6


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