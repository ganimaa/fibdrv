
import sys

expect = [0, 1]
result = []

def fib_gen(n):
    for i in range(2, n + 1):
        expect.append(expect[i - 1] + expect[i - 2])
        

def fib_check():
    with open('out', 'r') as f:
        for line in f.readlines():
            result = line.split(',')
            nth, res = result
            if int(res) != expect[int(nth)]:
                print(f"Wrong answer at Fib({nth}): {res} != {expect}")
                return
        print(f"Fib(0) to Fib({nth}) passed!")        


fib_gen(int(sys.argv[1]))
fib_check()
