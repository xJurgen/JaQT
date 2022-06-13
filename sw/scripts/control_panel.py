#!/usr/bin/env python3

#
# Author: Jiří Veverka
# Example usage of test class
#

import sys

from os import wait
from jaqt2 import JustAQuickTest2
from wiegand_tests import WiegandTests
from basic_tests import BasicTests

def dobasic(test):
    BasicTests.TestHelp(test)

    BasicTests.TestAllGpioOn(test)    
    BasicTests.TestAllGpioOffSlow(test)

    BasicTests.TestAllI2COn(test)
    BasicTests.TestGetAllGpioStatus(test)
    BasicTests.TestAllI2COff(test)

    BasicTests.TestAllGpioOnSlow(test)
    BasicTests.TestAllGpioOff(test)


def getBoardInfo(test):
    BasicTests.getBoardno(test)

def basicTests(test):
    #dobasic(test)
    while True:
        BasicTests.KnightRider(test)


def wiegandTests(test):
    #WiegandTests.basicStart(test)
    #WiegandTests.basicStop(test)
    WiegandTests.continuousInLimitStart(test)
    #WiegandTests.continuousUnderLimitStop(test)


def main():
    if len(sys.argv) == 1:
        test = JustAQuickTest2("/dev/ttyACM3")
    else:
        test = JustAQuickTest2(sys.argv[1])

    getBoardInfo(test)

    #wiegandTests(test)

    #basicTests(test)

    #test.reset_device()

main()


