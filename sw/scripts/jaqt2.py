import serial

#
# Author: Jiří Veverka
# implementation of base class for testing
#

class JustAQuickTest2:
    def __init__(self, tty):
        self.tty = tty
  
    def __enter__(self):
        try:
            self.serial = serial.Serial(    
                port = self.tty,
                baudrate =115200,
                timeout = 0.1,
            )
        except Exception:
            print("\033[1;31;40m[Error]:\033[0m Can't open serial port")
            exit()

        return self

    def __exit__(self, type, value, traceback):
        self.serial.close()

    def close(self):
        self.serial.close()

    def transcieve(self, request):
        lines = ""

        request += "\n"
        request = request.encode("utf-8")

        self.serial.write(request)      
        echo = self.serial.read_until() 
        echo = echo.decode("utf-8").strip()

        if echo.startswith(str(request.decode("utf-8").strip())):
            echo = echo[len(request):]  

        lines += echo
        count = 0
        while count < 1:
            response = self.serial.read_until("\r\n")
            
            if response == "".encode("utf-8"):
                break #delete if problems with missing output occurs
                count += 1
                response = self.serial.read_until()

            response = response.decode("utf-8").strip()

            lines += response

      
        return lines
    def transcieveNoResp(self, request):
        request += "\n"
        request = request.encode("utf-8")

        self.serial.write(request)      
      
        return


    def info(self):
        return self.transcieve("info") #???

    def help(self):
        return self.transcieve("help")  

    def gpio_set(self, number, state):
        if state == 1:
            return self.transcieveNoResp(f"gpio {number} on")
        elif state == 0:
            return self.transcieveNoResp(f"gpio {number} off")
        else:
            raise Exception(state)

    def gpio_get(self, number):
       return self.transcieve(f"gpio {number} get")

    def wiegand_read(self, state, count=26, mode=""):
        if state == "start" or state == "stop":
            if mode == "":
                return self.transcieve(f"wiegand {count} {state}")
            
            elif mode == "continuous":
                return self.transcieve(f"wiegand {mode} {count} {state}")

            else:
                raise Exception(mode)

        else:
            raise Exception(state)
    
    def reset_device(self):
        self.transcieveNoResp("reset device")
        return "DONE"

    def clear_screen(self):
        return self.transcieve("clear")

    def get_boardno(self):
        return self.transcieve("board_no get")

    def port_info(self):
        return self.transcieve("port info")

    def listen(self, line, state):
        if state == "start" or state == "stop":
            if line == 232 or line == 485 or line == "debug" or line == "all":
                return self.transcieve(f"read {line} {state}")
            else:
                raise Exception(line)
        else:
            raise Exception(state)

    def readShell(self, linesCount):
        i = 0
        lines = ""
        while i < linesCount:
            response = self.serial.read_until()
            if response == "\n".encode("utf-8"):
                response = self.serial.read_until()

            response = response.decode("utf-8").strip()

            lines += response
            lines += "\n"
            i+= 1

        return lines

    
