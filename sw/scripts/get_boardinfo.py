#!/usr/bin/env python3

import sys

from os import wait
from jaqt2 import JustAQuickTest2

def getPortInfo(test):
    test = test.__enter__()
    print(test.port_info())
    test.close()

def getBoardno(test):
    test = test.__enter__()
    print(test.get_boardno())
    test.close()

def getBoardInfo(test):
    getBoardno(test)
    getPortInfo(test)

def main():
    if len(sys.argv) == 1:
        test = JustAQuickTest2("/dev/ttyACM3")
    else:
        test = JustAQuickTest2(sys.argv[1])

    getBoardInfo(test)

main()


