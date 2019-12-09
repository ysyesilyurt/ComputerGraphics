#!/usr/bin/env python3

import os
import shutil
from subprocess import Popen
import time

ABS_PATH = "/home/yavuz/CLionProjects/ComputerGraphics/ForwardRenderingPipeline/inputs"


def runTests():
    print("Compiling...")
    p = Popen(['make', 'all'])
    retCode = p.wait()
    if retCode:
        print("Oops, couldn't make all, sth went wrong...")
        exit(0)
    p = Popen(['mv', 'rasterizer', 'outputs/our_outputs'])
    p.wait()
    os.chdir('outputs/our_outputs')
    DIRS = os.listdir(ABS_PATH)
    for folder in DIRS:
        if not os.path.exists(folder):
            os.mkdir(folder)
        inps = os.listdir(ABS_PATH + '/' + folder)
        for i in inps:
            print("Rendering {}...".format(folder + '/' + i))
            old = time.time()
            p = Popen(['./rasterizer', ABS_PATH + '/' + folder + '/' + i])
            retCode = p.wait()
            elapsed = time.time() - old
            if retCode:
                print("Oops, couldn't render {}, sth went wrong...".format(i))
                exit(0)
            print('{0} took {1:.3f} ms = {2:.3f} s = {3:.3f} mins.'.format(i, elapsed * 1000.0, elapsed, elapsed/60))


if __name__ == "__main__":
    runTests()