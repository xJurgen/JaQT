import sys, time
from os import wait
from jaqt2 import JustAQuickTest2

#
# Author: Jiří Veverka
# Script for reseting device - can be used for flashing purposes
#

def main(argv):
    reset = JustAQuickTest2(argv)
    reset = reset.__enter__()
    reset.reset_device()
    time.sleep(1)


main(sys.argv[1])
