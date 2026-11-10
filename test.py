from ultralytics import YOLO
import socket
import json
import time

#初始化udp网络套接字
def udpSocketInit(ip,port):
    #创建一个udp网络套接字 
    #AF_INET：使用IPv4协议  SOCK_DGRAM：使用UDP协议 
    udpSocket =socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    saddr = (ip,port)
    udpSocket.bind(saddr)#绑定ip和port
    return udpSocket


#初始化tcp网络套接字
def tcpSocketInit(ip,port):
    tcpSocket =socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    saddr = (ip,port)#服务器地址（ip+端口）
    tcpSocket.connect(saddr)#连接到服务器
    time.sleep(1)#等待一秒
    msg = "YOLO"
    tcpSocket.send(msg.encode('utf-8'))#发送一条消息“YOLO”
    return tcpSocket


def tcpTest(tcpSocket):
    # msg = "YOLO"
    # tcpSocket.send(msg.encode('utf-8'))
    print("wait recv...")
    #注意流式套接字粘包问题
    #recvData=tcpSocket.recv(1024)
    recvData=tcpSocket.recv(4)#先接受四个字节
    #这4个字节是Qt提前发过来的图片文件的大小，
    #先知道图片文件大小再接收图片
    if not recvData:#如果没收到数据说明对方已经断开
        tcpSocket.close()
        print("YOLO exit...")
        exit()
    fileSize =int.from_bytes(recvData,byteorder='little',signed=True)
    #把4字节的二进制转成整数（真正的文件大小）
    #from_bytes（二进制，小端序，有符号）->整数
    print(f"fileSize:{fileSize}")

    with open("./ultralytics/assets/t1.png","wb") as f:
    #以写模式打开文件，起个小名为f
        curSize =0 #当前已接受的字节数
        while True:#循环接收，直到收完
            recvData=tcpSocket.recv(fileSize)#接受一段数据
            f.write(recvData)#将这段数据写入f中
            curSize+=len(recvData)#累计已接收的长度
            print(f"file size:{len(recvData)}")
            if curSize==fileSize:#如果接受的总大小等于文件大小，退出循环
                break



#基于udp接受图片数据，并将图片数据保存
def recvImageUdp(udpSocket):
    print("recvfrom...")
    recvData,caddr= udpSocket.recvfrom(80000)
    #udpSocket.recvfrom这个函数收到的是两个东西，所以要有两个变量来承接
    #recvData接受的是图片二进制数据 caddr接收到的是发送方（QT）的地址
    #中译中：接收Qt发送过来的UDP数据，一次最多接受80000个字节
    #一个接受图片数据，一个接受发送方地址
    if recvData==b"stop":
       print("exit...")
       exit()
    #如果收到了“stop”指令就退出程序
    with open("./ultralytics/assets/t1.png","wb") as f:
    #wb：写入模式
    #中译中：打开这个路径下的图片文件（如果没有则自动创建），准备写入
    # 并且给这个文件起个小名叫f，方便后续操作
        f.write(recvData)
        #对f这个文件进行写入，完成这一步后，电脑上该路径下就会有t1.png
        print(f"file size:{len(recvData)}")
        #输出文件大小
    return caddr  #返回客户端地址  


#识别图片数据
def detectImage(yolo):
        #让YOLO加载并识别t1图片
        results = yolo(source="./ultralytics/assets/t1.png")
        #results = yolo(source="./datasets/images/train/390.jpg")
        result =results[0]#取识别的第一个结果
        result.save(filename ="./t1/t1.png")#保存识别结果到这个路径下
        return result #返回识别结果 
#这里为什么要取results[0]，也就是第一个元素，是由YOLO的识别特性决定的。YOLO
#可以一次性识别很多张，会将这些生成结果存放在一个列表里，读取结果的时候是从列表
#中读取.虽然我们这个项目只传了一张图片，识别了一张图片，但是识别结果还是存放在列
#表中，所以用results[0]


#基于udp返回识别结果
def returnResultUdp(result,caddr,udpSocket):
    #参数表示：什么东西，发回哪，用什么
    #中译中：把yolo识别出来的结果，发回Qt界面，用udp
    idx=-1#设置标识量，如果idx=-1表示没有识别出来，idx！=-1说明识别出来了
    #idx会在for循环里改变
    if result.boxes is not None:
        print(f"原始图片大小(h,w):{result.orig_shape}")
        boxes =result.boxes
        #result.boxes是yolo识别出来的所有目标的检测框的集合
    for idx,box in enumerate(boxes):
        #.cpu是把数据从GPU搬到CPU
        #.numpy是把张量变成numpy数组
        #numpy()[0]是因为numpy会返回二维数组[[x1,y1,x2,y2]]加上[0]会变成
        #[x1,y1,x2,y2]
        xyxy=box.xyxy.cpu().numpy()[0]#左上角x 左上角y，右下角x 右下角y
        xywh=box.xywh.cpu().numpy()[0]#中心点的（x，y）坐标，宽度和高度
        confidence=box.conf.cpu().numpy()[0]#置信度
        cls_idx=int(box.cls.cpu().numpy()[0])#类别索引号
        #int是用来强转的，把浮点型变为整型。张量tensor[0.]是浮点型，
        #加上int变成0
        cls_name=result.names[cls_idx]#类别名称
        
        print(f"\n目标{idx+1}:")
        print(f"类别:{cls_idx}:{cls_name}")
        print(f"置信度:{confidence:.4f}")
        print(f"坐标(xyxy):{xyxy.round(2)}")
        print(f"坐标(xywh):{xywh.round(2)}")

        #将结果通过json格式返回qt
        data={#字典格式
            "cls_idx":cls_idx,
            "cls_name":cls_name,
            "conf":round(float(confidence),2),
            #中译中：置信度强转为浮点型，保留两位小数
            "lx":round(float(xyxy.round(2)[0]),2),
            #中译中：数组里的第一个元素变为浮点型，保留两位小数，是左上角x的坐标
            "ly":round(float(xyxy.round(2)[1]),2),
            "rx":round(float(xyxy.round(2)[2]),2),
            "ry":round(float(xyxy.round(2)[3]),2) 
        }#round函数保存几位小数
        print(data)
        json_str=json.dumps(data)#.dumps是把python字典变为json格式字符串
        send_data=json_str.encode('utf-8')
        #utf-8是一种把文件变为字节的标准格式，支持中文，英文，符号
        #.encode是用来把字符串变为字节，因为udp只能传输字节
        udpSocket.sendto(send_data,caddr)
        #sendto（数据，目标地址）
        #中译中：把识别出来的数据，发送到Qt
    if idx==-1:  
        udpSocket.sendto(b"-1",caddr) 
        #循环没有执行->说明没有识别到物体
     

#基于tcp返回识别结果
def returnResultTcp(result,tcpSocket):
    idx=-1
    if result.boxes is not None:
        print(f"原始图片大小(h,w):{result.orig_shape}")
        boxes =result.boxes
        num=len(boxes)#目标数量（有几个检测出来的物体）
        print(f"num:{num}")
        b_num=num.to_bytes(4,byteorder='little',signed=True)
        #把数字转变为4字节二进制（小端序，有符号）
        tcpSocket.send(b_num)#发送给Qt


    for idx,box in enumerate(boxes):
        xyxy=box.xyxy.cpu().numpy()[0]
        xywh=box.xywh.cpu().numpy()[0]
        confidence=box.conf.cpu().numpy()[0]
        cls_idx=int(box.cls.cpu().numpy()[0])
        cls_name=result.names[cls_idx]
        
        print(f"\n目标{idx+1}:")
        print(f"类别:{cls_idx}:{cls_name}")
        print(f"置信度:{confidence:.4f}")
        print(f"坐标(xyxy):{xyxy.round(2)}")
        print(f"坐标(xywh):{xywh.round(2)}")

        #将结果通过json格式返回qt
        data={
            "cls_idx":cls_idx,
            "cls_name":cls_name,
            "conf":round(float(confidence),2),
            "lx":round(float(xyxy.round(2)[0]),2),
            "ly":round(float(xyxy.round(2)[1]),2),
            "rx":round(float(xyxy.round(2)[2]),2),
            "ry":round(float(xyxy.round(2)[3]),2)
         
        }
        print(data)
        json_str=json.dumps(data)
        send_data=json_str.encode('utf-8')
        tcpSocket.send(send_data)#把识别出来的结果数据通过tcp发送
        time.sleep(1)#间隔1秒进行，以防流式套接字粘包问题
        #就是识别出来很多结果，一直不间隔发送，会粘到一起，让系统识别为一个结果
    # if idx==-1:  
    #     tcpSocket.send(b"-1")


if __name__=="__main__":
    tcpSocket=tcpSocketInit("192.168.234.52",9999)
    #服务器的ip地址和端口号

    #tcpTest(tcpSocket)
    #yolo = YOLO(model = "./yolo11n.pt")#官方自带的yolo模型
    yolo=YOLO(model="./runs/detect/train/weights/best.pt")
    #自己训练的模型

    #udpSocket=udpSocketInit("127.0.0.1",6666)
while True:#程序启动后永不退出，
    #一直循环：接受图片，检测物体，回传结果
    #caddr=recvImageUdp(udpSocket)
    tcpTest(tcpSocket)#接受图片数据
    result=detectImage(yolo)#识别图片
    returnResultTcp(result,tcpSocket)#返回识别结果


