# -*- codeing = utf-8 -*-
# @Time : 2021/3/31 14:23
# @Author : zyz
# @File : encode.py
# @Software : PyCharm

import cv2
import numpy as np
import random
import string
import CRC
import os

class QR_code:
    # 编码
    def encode(self, data):
        return ' '.join([bin(ord(ch)).replace('0b', '') for ch in data])

    # 解码
    def decode(self, data):
        return ''.join([chr(i) for i in [int(b, 2) for b in data.split(' ')]])

    # 随机字符生成
    def ranstr(self, num):
        file = open("text.txt", "w")
        data = ""
        for i in range(50):
            data += ''.join(random.sample(string.ascii_letters + string.digits, num))
        file.write(data)
        file.close()

    # 读取文件信息
    def get_data(self,file_name,i):
        # self.ranstr(62)
        binfile = open(file_name, "r")
        # binfile = open(file_name, "a+")
        binfile.seek(i*3101,0)
        data = binfile.read(3101)
        data = self.encode(data)
        cur = ''
        Data = []
        cnt = 0
        for i in data:
            if i == ' ':
                while len(cur) < 8:
                    cur = '0' + cur
                Data.append(cur)
                cur = ''
                cnt += 1
            else:
                cur += i
        while len(cur) < 8:
            cur += '0'
        Data.append(cur)
        binfile.close()
        return Data

    def process_data(self, Data):
        data = ""
        for x in Data:
            crc = CRC.CRC(x, 4)
            for y in crc.code:
                if y == 1:
                    data += "1"
                else:
                    data += "0"

        return data

    # 初始化定位码
    def init(self, dst):
        # 左上定位码
        cv2.rectangle(dst, (20, 20), (90, 90), (0, 0, 1), -1)
        cv2.rectangle(dst, (30, 30), (80, 80), (255, 255, 255), -1)
        cv2.rectangle(dst, (40, 40), (70, 70), (0, 0, 1), -1)

        # 右上定位码
        cv2.rectangle(dst, (960, 20), (1030, 90), (0, 0, 1), -1)
        cv2.rectangle(dst, (970, 30), (1020, 80), (255, 255, 255), -1)
        cv2.rectangle(dst, (980, 40), (1010, 70), (0, 0, 1), -1)

        # 左下定位码
        cv2.rectangle(dst, (20, 960), (90, 1030), (0, 0, 1), -1)
        cv2.rectangle(dst, (30, 970), (80, 1020), (255, 255, 255), -1)
        cv2.rectangle(dst, (40, 980), (70, 1010), (0, 0, 1), -1)

        # 右下
        cv2.rectangle(dst, (950, 950), (1000, 1000), (0, 0, 1), -1)
        cv2.rectangle(dst, (960, 960), (990, 990), (255, 255, 255), -1)
        cv2.rectangle(dst, (970, 970), (980, 980), (0, 0, 1), -1)

    def draw(self, data, dst):
        idx = 0
        for x in range(20, 1030, 10):
            for y in range(20, 1030, 10):
                if (x >= 20 and x <= 90 and y >= 20 and y <= 90):
                    continue
                elif (x >= 20 and x <= 90 and y >= 950 and y <= 1030):
                    continue
                elif (x >= 950 and x <= 1030 and y >= 20 and y <= 90):
                    continue
                elif (x >= 950 and x <= 1030 and y >= 950 and y <= 1030):
                    continue
                else:
                    c = [0, 0, 0]
                    for i in range(3):
                        if data[idx] == '1':
                            c[i] = 1
                        idx += 1
                    cv2.rectangle(dst, (y, x), (y + 10, x + 10), (0 + 255 * c[0], 0 + 255 * c[1], 0 + 255 * c[2]), -1)
                    # if data[idx] == "1":
                    #     cv2.rectangle(dst, (x, y), (x + 10, y + 10), (0, 0, 1), -1)
                    # idx += 1

    # 绘制3x3模块
    def draw33(self, data, x, y, dst):
        row, col = [0, 0, 0], [0, 0, 0]
        flagr, flagc = False, False
        if x > y:
            flagr = True
        elif x < y:
            flagc = True
        for i in range(3):
            for j in range(3):
                if data[i * 3 + j] == '0':
                    continue
                else:
                    cv2.rectangle(dst, (y + 10 * j, x + 10 * i), (y + 10 * (j + 1), x + 10 + 10 * i), (0, 0, 1), -1)
                    col[j] += 1
                    row[i] += 1

        if flagr:
            for i in range(3):
                if row[i] % 2:
                    continue
                else:
                    cv2.rectangle(dst, (y + 30, x + 10 * i), (y + 50, x + 10 + 10 * i), (0, 0, 1), -1)

        if flagc:
            for i in range(3):
                if col[i] % 2:
                    continue
                else:
                    cv2.rectangle(dst, (y + 10 * i, x + 30), (y + 10 + 10 * i, x + 50), (0, 0, 1), -1)

    # 获取当前二维码编号
    def get_id(self, num):
        id = ""
        if num == 0:
            id = 9 * '0'
            return id
        temp = []
        while num:
            temp.append(num % 2)
            num //= 2
        while temp:
            id += str(temp.pop())
        while len(id) < 9:
            id = '0' + id
        return id

    # 绘制二维码
    def Draw(self, id, dst):
        self.Drawenv(id,dst)
        self.init(dst)

    # 绘制版本模块
    def Drawenv(self, id, dst):
        self.draw33(id, 950, 1000, dst)
        self.draw33(id, 1000, 950, dst)
        self.draw33(id, 1000, 1000, dst)

    # if __name__ == "__main__":
    #     newImg = (1080, 1080, 3)
    #     dst = np.zeros(newImg, np.uint8)
    #     dst.fill(255)
    #     Draw()
    #     data = get_data()
    #     data = process_data(data)
    #     draw(data)
    #     cv2.imshow("encode", dst)
    #     cv2.waitKey()
    #     cv2.imwrite("1.png", dst)

    def __init__(self, file_name):
        cur_dir = os.getcwd()
        folder_name = 'outputimg'
        if os.path.isdir(cur_dir):
            os.mkdir(os.path.join(cur_dir, folder_name))  # 新建文件
        # file_name=cur_dir+'/'+file_name
        # path=os.path.abspath()
        id = []
        # print("开始转图片")
        for i in range(15):
            id.append(self.get_id(i))
        for i in range(15):
            newImg = (1080, 1080, 3)
            dst = np.zeros(newImg, np.uint8)
            dst.fill(255)
            self.Draw(id[i], dst)
            data = self.get_data(file_name,i)
            data = self.process_data(data)
            self.draw(data,dst)
            file_img = cur_dir + "/outputimg/" + 'code' + str(i) + '.jpg'
            cv2.imwrite(file_img, dst)
