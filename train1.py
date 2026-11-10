# import cv2
# import matplotlib.pyplot as plt

# num=0
# step=30
# video=cv2.VideoCapture("./datasets/BVN.mp4")
# while True:
#     ret,fram=video.read()
#     print(ret)
#     if ret==False:
#         print("抽帧出错")
#         break
#     num+=1
#     if num%step==0:
#         cv2.imwrite("./datasets/images/"+str(num)+".jpg",fram)
#     # plt.imshow(fram)


import warnings
warnings.filterwarnings("ignore")

from ultralytics import YOLO

model=YOLO("yolo11n.pt")

result=model.train(
    data="bvn.yaml",
    epochs=50,
    workers=0
)

