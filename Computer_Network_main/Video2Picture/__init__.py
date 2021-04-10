import cv2
from cv2 import VideoWriter, VideoWriter_fourcc, imread, resize
import os
from PIL import Image
from encode1 import QR_code
import sys

img_root = os.getcwd()  # 获取当前路径


def Pic2Video(video_name, set_fps=15):
    imgPath = img_root + "/outputimg/"  # 读取图片路径
    videoPath = img_root + '/' + video_name  # 保存视频路径
    images = os.listdir(imgPath)
    fps = set_fps  # 每秒默认15帧数
    # VideoWriter_fourcc为视频编解码器 ('I', '4', '2', '0') —>(.avi) 、('P', 'I', 'M', 'I')—>(.avi)、('X', 'V', 'I', 'D')—>(.avi)、('T', 'H', 'E', 'O')—>.ogv、('F', 'L', 'V', '1')—>.flv、('m', 'p', '4', 'v')—>.mp4
    fourcc = VideoWriter_fourcc(*"mp4v")
    size = (620, 620)
    image = Image.open(imgPath + images[0])
    # print(image.size)
    videoWriter = cv2.VideoWriter(videoPath, fourcc, fps, frameSize=image.size)
    # print("开始转视频")
    for im_name in range(len(images)):
        frame = cv2.imread(imgPath + images[im_name])  # 这里的路径只能是英文路径
        # frame = cv2.imdecode(np.fromfile((imgPath + images[im_name]), dtype=np.uint8), 1)  # 此句话的路径可以为中文路径
        # print(im_name)
        videoWriter.write(frame)
    print("图片转视频成功")
    videoWriter.release()
    cv2.destroyAllWindows()


def Video2Pic(videopath):

    videoPath = img_root + '/' + videopath  # 读取视频路径
    folder_name = 'output'  # 定义新建文件夹名
    if os.path.isdir(img_root):
        os.mkdir(os.path.join(img_root, folder_name))  # 新建文件
    imgPath = img_root + "/output/"  # 保存图片路径

    cap = cv2.VideoCapture(videoPath)
    fps = cap.get(cv2.CAP_PROP_FPS)  # 获取帧率
    # print("video2pic",fps)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))  # 获取宽度
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))  # 获取高度
    if not cap.isOpened():  # 是否成功打开
        print("Please check the path.")
    frame_count = 0
    cnt = 0
    while 1:
        suc, frame = cap.read()
        cnt += 1
        frame_count += 1
        # cv2.imwrite(imgPath + str(frame_count).zfill(4), frame)
        if not suc:
            break
        cv2.imwrite(imgPath + "%d.png" % frame_count, frame)
        cv2.waitKey(1)
    print("视频转图片成功")
    cap.release()




if __name__ == '__main__':
    a = QR_code()
    Pic2Video(sys.argv[1], int(sys.argv[2]))
    Video2Pic()