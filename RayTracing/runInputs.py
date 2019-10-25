#!/usr/bin/env python3

import os
from subprocess import Popen
import time

ABS_PATH = "/home/yavuz/CLionProjects/ComputerGraphics/RayTracing/inputs"


def runTests():
    print("Compiling...")
    p = Popen(['make', 'all'])
    retCode = p.wait()
    if retCode:
        print("Oops, couldn't make all, sth went wrong...")
        exit(0)
    p = Popen(['mv', 'raytracer', 'outputs'])
    p.wait()
    os.chdir('outputs')
    inputs = os.listdir(ABS_PATH)
    for i in inputs:
        print("Rendering {}...".format(i))
        old = time.time()
        p = Popen(['./raytracer', "../inputs/{}".format(i)])
        retCode = p.wait()
        elapsed = time.time() - old
        if retCode:
            print("Oops, couldn't render {}, sth went wrong...".format(i))
            exit(0)
        print('{0} took {1:.3f} ms = {2:.3f} s.'.format(i, elapsed * 1000.0, elapsed))


if __name__ == "__main__":
    runTests()