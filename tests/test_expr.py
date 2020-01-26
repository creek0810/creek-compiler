import os


def compile_and_run(test_data):
    with open("test.c", "w") as file:
        file.write(test_data)
    os.system("./compiler test.c > test.s")
    os.system("gcc -o test test.s")
    result = os.system("./test")
    return result >> 8


def test_jmp():
    result = compile_and_run("""
    int main(){
        int i=0;
        int sum = 0;
        int j = 10;
        while(i < j) {
            if(i == 5) {
                i += 1;
                continue;
            }
            sum += i;
            i+=1;
        }
        return sum;
    }""")
    assert result == 40

    result = compile_and_run("""
    int main(){
        int i=0;
        while(i >= 0) {
            if(i == 5) {
                break;
            }
            i+=1;
        }
        return i;
    }""")
    assert result == 5


def test_loop():
    result = compile_and_run("""
    int main(){
        int a = 0;
        int b = 2;
        while(a < 10) {
            b += 2;
            a += 1;
        }
        return b;
    }
    """)
    assert result == 22

    result = compile_and_run("""
    int main(){
        int a = 2;
        int b = 2;
        for(a = 0; a<10; a+=1) {
            b += 2;
        }
        return b;
    }
    """)
    assert result == 22

    result = compile_and_run("""
    int main(){
        int a = 2;
        int b = 0;
        for(a = 0; a<10; a+=1) {
            b += a;
        }
        return b;
    }
    """)
    assert result == 45

    result = compile_and_run("""
    int main(){
        int a = 2;
        int b = 0;
        do {
            b = 10;
        } while(a < 2);
        return b;
    }
    """)
    assert result == 10


def test_assign():
    # test assign op
    mapping_table = {
        "=": 2,
        "*=": 20,
        "/=": 5,
        "%=": 0,
        "+=": 12,
        "-=": 8,
        "<<=": 40,
        ">>=": 2,
        "&=": 2,
        "^=": 8,
        "|=": 10
    }
    for op in mapping_table:
        result = compile_and_run("""
        int main(){{
            int a;
            a = 10;
            a {op} 2;
            return a;
        }}
        """.format(op=op))
        assert result == mapping_table[op]

    # test association
    result = compile_and_run("""
    int main(){
        int a;
        int b;
        a = 1;
        b = 2;
        a = b = 3;
        return a;
    }
    """)
    assert result == 3

    result = compile_and_run("""
    int main(){
        int a;
        int b;
        a = 1;
        b = 2;
        a = b = 3;
        return b;
    }
    """)
    assert result == 3


def test_control():
    result = compile_and_run("""
    int main(){
        int a;
        a = 1;
        return a ? 2 : 5;
    }
    """)
    assert result == 2

    result = compile_and_run("""
    int main(){
        int a;
        a = 0;
        return a ? 2 : 5;
    }
    """)
    assert result == 5

    result = compile_and_run("""
    int main(){
        int a;
        a = 2;
        return a ? 2 : 5;
    }
    """)
    assert result == 2

    result = compile_and_run("""
    int main(){
        if(1) {
            return 2;
        }
        return 0;
    }
    """)
    assert result == 2

    result = compile_and_run("""
    int main(){
        if(0) {
            return 2;
        }
        return 0;
    }
    """)
    assert result == 0


def test_return():
    result = compile_and_run("int main(){return 5 + 2 * 10;}")
    assert result == 25
    result = compile_and_run("int main() {return (5 + 2) * 10;}")
    assert result == 70


def test_lvar():
    # test init
    result = compile_and_run("""
        int main(){
            int a = 2;
            return a
        }
    """)
    assert result == 2
    result = compile_and_run("""
        int main(){
            int a = 2;
            {
                int a = 3;
                return a;
            }
            return a;
        }
    """)
    assert result == 3
    # test block
    result = compile_and_run("""
        int main(){
            int a;
            a = 1;
            {
                a = a + 1;
                return a;
            }
        }
    """)
    assert result == 2

    result = compile_and_run("""
        int main(){
            int a;
            a = 1;
            {
                int a;
                a = 10
                a = a + 1;
                return a;
            }
        }
    """)
    assert result == 11

    result = compile_and_run("""
        int main(){
            int a;
            a = 1;
            {
                int a;
                a = 10;
                a = a + 1;
            }
            return a;
        }
    """)
    assert result == 1

""" only expr
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
"""