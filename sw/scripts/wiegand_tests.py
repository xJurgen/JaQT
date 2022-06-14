class WiegandTests:
    def basicStart(test):
        test = test.__enter__()
        print(test.wiegand_read("start"))
        test.close()

    def basicStop(test):
        test = test.__enter__()
        print(test.wiegand_read("stop"))
        test.close()

    def continuousOverLimitStart(test):
        test = test.__enter__()
        print(test.wiegand_read("start", 300, "continuous"))
        test.close()

    def continuousInLimitStart(test):
        test = test.__enter__()
        print(test.wiegand_read("start", 150, "continuous"))
        test.close()

    def continuousUnderLimitStop(test):
        test = test.__enter__()
        print(test.wiegand_read("stop", 0, "continuous"))
        test.close()

