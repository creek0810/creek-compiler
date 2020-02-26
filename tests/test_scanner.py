import os
import filecmp
TEST_PATH = "tests/data/scanner/in"
OUT_PATH = "tests/data/scanner/out"

# TODO: constant(float) test, punctuator test
"""
lexical elements:
1. keyword
2. identifier
3. constant
4. string-literal
5. punctuator
"""

def run(file_name):
    result = os.system(f"./compiler {TEST_PATH}/{file_name} scan > tmp.out")
    return result >> 8

def test_keyword():
    result = run("keyword.in")
    assert filecmp.cmp("tmp.out", f"{OUT_PATH}/keyword.out") == True

def test_ident():
    result = run("ident.in")
    assert filecmp.cmp("tmp.out", f"{OUT_PATH}/ident.out") == True

def test_constant():
    """
    constant type:
    1. int
    2. float
    3. enum
    4. char
    """
    test_case = [
        "char", "int"
    ]
    for name in test_case:
        result = run(f"{name}.in")
        assert filecmp.cmp("tmp.out", f"{OUT_PATH}/{name}.out") == True

def test_str():
    result = run("str.in")
    assert filecmp.cmp("tmp.out", f"{OUT_PATH}/str.out") == True