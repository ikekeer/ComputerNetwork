import cv2 as cv
import numpy as np
import os

num_in_roll = 101  # 一行有的小方块数量
# 仿射变换
S_size = 505  # 变换后的二维码大小
S_out_Size = 600  # 二维码输出大小


class detect_decode:
    def affine_transformation(self,points,input_img):
        ptr1 = points

        ptr2 = np.float32([[0, 0], [S_size, 0], [S_size, S_size], [0, S_size]])
        # print(type(ptr1), type(ptr2))
        M = cv.getPerspectiveTransform(ptr1, ptr2)
        res = cv.warpPerspective(input_img, M, (S_out_Size, S_out_Size))
        # print(res)
        return res


    def pixel2binary(self,res):  # 读取图片信息
        stance = S_size / num_in_roll  # 每一个小方块的边长#取指尽量将变长转换成整数
        # cv.imshow("res",res)
        center = (int)(stance / 2)  # 每个小像素是5x5取中点值读取rgb值
        # pixel = [[0 for x in range(num_in_roll)] for y in range(num_in_roll)]
        pixel = []
        # pixel=np.arange(41*41).reshape((41,41))
        # for i in range(num_in_roll):  # 行读取
        #     i_x = center + i * stance
        #     for j in range(num_in_roll):
        #
        #         j_y = center + j * stance
        #         rgb = res[(int)(i_x), (int)(j_y)]
        #         print(rgb)
        #
        #         if rgb[2] > 150:  # 白色
        #             pixel[i][j] = 0
        #         else:
        #             pixel[i][j] = 1
        version = ""
        for x in range(num_in_roll):
            i_x = center + x * stance
            for y in range(num_in_roll):
                j_y = center + y * stance
                # print(res[(int)(i_x), (int)(j_y)])
                rgb = res[(int)(i_x), (int)(j_y)]
                if x < 8 and y < 8:
                    continue
                elif x > 92 and y < 8:
                    continue
                elif x < 8 and y > 92:
                    continue
                elif x > 92 and y > 92:
                    if(rgb> 150):
                        version += "0"
                    else:
                        version += "1"
                else:

                    # print(rgb)


                    if (rgb < 255 / 2):
                            pixel.append(0)
                    else:
                            pixel.append(1)
                    # if(rgb[2]>150):pixel.append(0)
                    # else :pixel.append(1)
        # for i in range(8):
        #     for j in range(8):
        #         print(version[8*i+j], end="")
        #     print()
        ver1, ver2, ver3 = "", "", ""
        for i in range(5, 8):
            for j in range(3):
                ver1 += version[8*i+j]
        for i in range(5, 8):
            for j in range(5, 8):
                ver3 += version[8 * i + j]
        for i in range(3):
            for j in range(5, 8):
                ver2 += version[8*i+j]

        if ver1 != ver2 or ver1 != ver3:
            return -1, pixel
        # return ver1, pixel
        ver1 = "0b" + ver1
        return int(ver1, 2), pixel


    # def get_Little_Version(res, x, y):
    #     version = ""
    #     for j in range(3):
    #         for i in range(3):
    #             rgb = res[x + i][y + j]
    #             if rgb[2] > 255 / 2:
    #                 version += "0"
    #             else:
    #                 version += "1"
    #     return version
    #
    #
    # def get_env(res):
    #     stance = S_size / num_in_roll  # 每一个小方块的边长#取指尽量将变长转换成整数
    #     # cv.imshow("res",res)
    #     center = (int)(stance / 2)  # 每个小像素是5x5取中点值读取rgb值
    #
    #     temp1 = get_Little_Version(res, 462, 492)
    #     temp2 = get_Little_Version(res, 492, 462)
    #     temp3 = get_Little_Version(res, 493, 493)
    #     print(temp1, temp2, temp3)
    #     if (temp1 != temp2 or temp1 != temp3):
    #         return -1
    #
    #     return ''.join([chr(i) for i in [int(b, 2) for b in temp1.split(' ')]])


    def Write(self,pixel):
        file = open("detect_txt.txt", "w", encoding='utf-8')
        for i in pixel:
            file.write(str(i))
            # file.write('\n')
        file.close()


    def solve(self,pixel,ou,bu):
        n = len(pixel)
        p = 25

        right, s = "", ""
        for i in range(0, n - 3, 12):
            two = 2048
            sum = 0
            for j in range(12):
                if(i+j>=len(pixel)):
                    break
                sum = sum + two * pixel[i + j]
                # print(i, j, i + j, pixel[i + j])
                two = two // 2
            if (sum % p == 0):
                right = right + '1'
            else:
                right = right + '0'
            sum = sum // 16
            s += chr(sum)
        bu.write(right)
        ou.write(s)



    def __init__(self):

        img_root = os.getcwd() #获取当前路径
        imgPath = img_root+"/output/"  # 保存图片路径
        count=0
        ou = open("out.txt", "w", encoding="utf-8")
        bu = open("vout.txt", "w", encoding="utf-8")
        for filename in os.listdir(imgPath):
            input_img = cv.imread(imgPath+filename)
            count += 1
            gray_img = cv.cvtColor(input_img, cv.COLOR_BGR2GRAY)  # 将图片通过库转换成灰度图像
            # gray_img=cv.cvtColor(gray_img1,cv.COLOR_GRAY2BGR)
            ret, binary_img = cv.threshold(gray_img, 127, 255, cv.THRESH_BINARY)  # 将图片转换成黑白二值化
            contours, hierarchy = cv.findContours(binary_img, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
            points = []
            qrcoder = cv.QRCodeDetector()
            ret, points = qrcoder.detect(binary_img)
            if(ret==0):
                continue
        # # 仿射变换
            res = self.affine_transformation(points,binary_img)

        # # 转换成二进制文件
            pixel = []
            version, pixel = self.pixel2binary(res)  # 图片转二进制
            self.solve(pixel,ou,bu)  # 解码
        bu.close()
        ou.close()
        cv.waitKey(0)
        cv.destroyAllWindows()
