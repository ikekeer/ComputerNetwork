# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import time
from encode1 import QR_code
from Video2Picture import Video2Pic,Pic2Video
import os
import sys
from Detect import detect_decode
sys.path.append(os.path.dirname(sys.path[0]))
cur_dir = os.getcwd() #获取当前路径
if __name__ == "__main__":
    #读取文件，转换为二维码

    QR_code(sys.argv[1])
     #将二维码序列转换为视频
    Pic2Video(sys.argv[2], 15)  # FPS=15
    Video2Pic(sys.argv[3])
    #获得图片后转为二进制文件
    detect_decode()
