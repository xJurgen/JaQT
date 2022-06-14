import sys, time
from os import wait
from jaqt2 import JustAQuickTest2

def main(argv):
    reset = JustAQuickTest2(argv)
    reset = reset.__enter__()
    reset.reset_device()
    time.sleep(1)


main(sys.argv[1])