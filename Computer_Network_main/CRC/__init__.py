# -*- codeing = utf-8 -*-
# @Time : 2021/3/31 14:47
# @Author : zyz
# @File : CRC.py
# @Software : PyCharm

class CRC:
    def __init__(self, info, crc_n=4):
        if type(info).__name__ == 'str':
            data = info
            info = []
            for x in data:
                if x == '0':
                    info.append(0)
                else:
                    info.append(1)
        self.info = info
        if crc_n == 4:
            loc = [4, 3, 0]

        # 列表解析转换为多项式比特序列
        p = [0 for i in range(crc_n + 1)]
        for i in loc:
            p[i] = 1
        p = p[::-1]

        info = self.info.copy()
        times = len(info)
        n = crc_n + 1

        # 左移补零即乘积
        for i in range(crc_n):
            info.append(0)

        # 乘积除以多项式比特序列
        q = []
        for i in range(times):
            if info[i] == 1:
                q.append(1)
                for j in range(n):
                    info[j + i] = info[j + i] ^ p[j]
            else:
                q.append(0)

        # 余数即为CRC编码
        check_code = info[-crc_n::]

        code = self.info.copy()
        for i in check_code:
            code.append(i)

        self.crc_n = crc_n
        self.p = p
        self.q = q
        self.check_code = check_code
        self.code = code

    def print_format(self):
        """格式化输出结果"""

        print('{:10}\t{}'.format('发送数据比特序列：', self.info))
        print('{:10}\t{}'.format('生成多项式比特序列：', self.p))
        print('{:15}\t{}'.format('商：', self.q))
        print('{:10}\t{}'.format('余数（即CRC校验码）：', self.check_code))
        print('{:5}\t{}'.format('带CRC校验码的数据比特序列：', self.code))

    def np2str(self):
        res = ""
        for x in self.code:
            if x == 0:
                res += '0'
            else:
                res += '1'
        return res