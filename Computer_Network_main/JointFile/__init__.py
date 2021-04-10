import string

class jointfile:

    def fileTest1(num):

        b = ".txt"
        outfile = open("result.txt", "w")
        frame = 1;
        for i in range(num):
            a = str(i)
            c = a + b
            myfile = open(c, "r")
            no = myfile.readline()  # 读第一行获取帧编号
            if (int(no) == frame):
                frame = frame + 1
                str = myfile.readline()
                while (str):
                    outfile.write(str)
                    str = myfile.readline()
            myfile.close()
        outfile.close()

    def __init__(self, num):
        self.fileTest1(num)







