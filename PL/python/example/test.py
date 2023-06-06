from ctypes import *

def myprintf(args):
  libc = cdll.LoadLibrary("./libs/mylib.so")  
  print("call mylib.myprintf")
  print(libc.mylib_printf)
  libc.mylib_printf(b"Hello World!")

if __name__ == "__main__":
  myprintf("hello")
