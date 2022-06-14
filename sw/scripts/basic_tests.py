class BasicTests:
    def TestHelp(test):
        test = test.__enter__()
        print(test.help())
        test.close()

    def getBoardno(test):
        test = test.__enter__()
        print(test.get_boardno())
        test.close()

    def TestAllGpioOn(test):
        test = test.__enter__()
        test.gpio_set("all", 1)
        test.close()

    def TestAllGpioOff(test):
        test = test.__enter__()
        test.gpio_set("all", 0)
        test.close()

    def TestAllGpioOffSlow(test):
        i = 1
        while i < 15:
            test = test.__enter__()
            test.gpio_set(i, 0)
            i+=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestAllGpioOnSlow(test):
        i = 1
        while i < 15:
            test = test.__enter__()
            test.gpio_set(i, 1)
            i+=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestAllGpioOnSlowReverse(test):
        i = 14
        while i > 0:
            test = test.__enter__()
            test.gpio_set(i, 1)
            i-=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1
    
    def TestAllGpioOffSlowReverse(test):
        i = 14
        while i > 0:
            test = test.__enter__()
            test.gpio_set(i, 0)
            i-=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestAllI2COn(test):
        i = 14
        while i > 6:
            test = test.__enter__()
            test.gpio_set(i, 1)
            i-=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestAllI2COff(test):
        i = 14
        while i > 6:
            test = test.__enter__()
            test.gpio_set(i, 0)
            i-=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestGetAllGpioStatus(test):
        i = 1
        while i < 15:
            test = test.__enter__()
            print(test.gpio_get(i))
            i+=1
            test.close()

    def TestOnLeft(test):
        i = 1
        while i <= 18:
            print("left")
            print(i)
            #input()
            if i < 15:
                test = test.__enter__()
                test.gpio_set(i, 1)
                test.close()
            if (i - 4) > 0:
                test = test.__enter__()
                test.gpio_set(i - 4, 0)
                test.close()

            i+=1
            test.close()
            wait = 0
            while wait < 800000:
                wait+=1

    def TestOnRight(test):
        i = 18
        while i >= 0:
            print("right")
            print(i)
            #input()
            if i < 15:
                test = test.__enter__()
                test.gpio_set(i, 0)
                test.close()

            if (i - 4) > 0:
                test = test.__enter__()
                test.gpio_set(i - 4, 1)
                test.close()
            
            i-=1
            wait = 0
            while wait < 800000:
                wait+=1

    def KnightRider(test):
        BasicTests.TestOnLeft(test)
        BasicTests.TestOnRight(test)


    def TestClear(test):
        test = test.__enter__()
        test.clear_screen()
        test.close()

    def TestReset(test):
        test = test.__enter__()
        print(test.reset_device())
