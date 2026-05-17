import cv2
import time
import numpy as np
import serial
from ultralytics import YOLO
import cv2
import threading

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)

walls = [0, 0, 0, 0, 0, 0]
lab = np.zeros((40, 40))
lab[20, 20] = 3
x = 20
y = 20
corn = 0
stat_ezda = 0
aa = 0
statv = 0
chek = 0

detector_model = YOLO('/home/maze/YOLOv8_maze/train_code/runs/classify/train10/weights/best.pt')
detector_model.to('cpu')


class VideoStream:
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.stream.read()
        self.stopped = False
    def start(self):
        threading.Thread(target=self.update, args=(), daemon=True).start()
        return self
    def update(self):
        while not self.stopped:
            (self.grabbed, self.frame) = self.stream.read()
    def read(self):
        return self.frame
    

vs1 = VideoStream(0).start()
vs2 = VideoStream(1).start()


def camera():
    
    global statv

    img = vs1.read()
    img2 = vs2.read()

    if img is None or img2 is None:
        return

    # img = cv2.cvtColor(img, cv2.COLOR_2)
    res = detector_model.predict(source=img,#'/Users/bw/Documents/Hachathons/ROS2026/classification_dataset/val/Octopus/1ffdfc23-1774106735_0.png',
                            show=False,
                            save=False,
                            conf=0.875,
                            save_txt=False,
                            save_crop=False,
                            verbose=False,
                            device='cpu',
                            half=False,
                            #   color=(255,0,0)
                            )[0]
    # a = [res.names[res.probs.top1[i]] for i in range(5)]
    # print(res.names[res.probs.top1])
    a = res.names[res.probs.top1]
    # print(res)

    # img = cv2.cvtColor(img, cv2.COLOR_2)
    res2 = detector_model.predict(source=img2,#'/Users/bw/Documents/Hachathons/ROS2026/classification_dataset/val/Octopus/1ffdfc23-1774106735_0.png',
                            show=False,
                            save=False,
                            conf=0.875,
                            save_txt=False,
                            save_crop=False,
                            verbose=False,
                            device='cpu',
                            #   color=(255,0,0)
                            )[0]
    # a2 = [res2.names[res2.probs.top5[i]] for i in range(5)]
    # print(res2.names[res2.probs.top1])
    a2 = res2.names[res2.probs.top1]
    # print(res2)

    x_oldc = x
    y_oldc = y

    if a != "nothing": print(a)
    if a == "nedoF":
        if lab[y, x] != 4 and lab[y, x] != 5:
            send("rescue_1l")
            print("1l")
            lab[y, x] = 4
            time.sleep(420)
    if a == "F":
        if lab[y, x] != 4 and lab[y, x] != 5:
            send("rescue_2l")
            print("2l")
            lab[y, x] = 5
            time.sleep(420)

    if a2 != "nothing": print(a2)
    if a2 == "nedoF":
        if lab[y, x] != 4 and lab[y, x] != 5:
            send("rescue_1r")
            print("1r")
            lab[y, x] = 4
            time.sleep(420)
    if a2 == "F":
        if lab[y, x] != 4 and lab[y, x] != 5:
            send("rescue_2r")
            print("2r")
            time.sleep(420)


    # cv2.imshow('neyronka1', res.plot())
    # cv2.imshow('neyronka2', res2.plot())

    # # Обязательная строка для работы окон
    # cv2.waitKey(1)
    time.sleep(0.01)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        cv2.destroyAllWindows()



print("ready")

stat_lab = 2
stat = 1
n = 0

def send(action):
        global x_old, y_old
        if action == "straight": ser.write(b"u"), print("1")
        if action == "back": ser.write(b"d")
        if action == "left": ser.write(b"l")
        if action == "right": ser.write(b"r")
        if action == "end": ser.write(b"e")
        if action == "start": 
            ser.write(b"s")
            x_old = 20
            y_old = 20
        if action == "rescue_1r": ser.write(b"v")
        if action == "rescue_1l": ser.write(b"b")
        if action == "rescue_2r": ser.write(b"n")
        if action == "rescue_2l": ser.write(b"m")


def read(read_obj):
    global walls, s
    if read_obj == "walls":
        print("read_walls")
        rest = ser.read(9).decode('ascii')
        full_str = s + rest
        walls = [int(digit) for digit in full_str]
    else:
        byte = ser.read(1)
        s = byte.decode('ascii')
        while s != read_obj:
            time.sleep(1)
            print(byte)
            byte = ser.read(1)
            s = byte.decode('ascii')
            print(f"{byte.decode('ascii')}")

read("s")
print("begin")
send("start")

def camera_loop():
    while True:
        camera()
        time.sleep(0.01)

camera_thread = threading.Thread(target=camera_loop, daemon=True)
camera_thread.start()

#while True:
# send("straight")
# time.sleep(10)
# send("straight")
# time.sleep(10)
# send("straight")
# time.sleep(10)
# send("left")

def ezda_r():
    global x, y, corn

    if corn == 0:
        print("right")
        send("right")
        corn = 90
        x = x+2
    else:
        if corn == -90:
            print("straight")
            send("straight")
            y = y-2
            corn = 0
        else:
            if corn == 90:
                print("back")
                send("back")
                y = y+2
                corn = 180
            else:
                if corn == 180:
                    print("left")
                    send("left")
                    corn = -90
                    x = x-2


def ezda_l():
    global x, y, corn

    if corn == 0:
        print("left")
        send("left")
        corn = -90
        x = x-2
    else:
        if corn == 90:
            print("straight")
            send("straight")
            corn = 0
            y = y-2
        else:
            if corn == -90:
                print("back")
                send("back")
                corn = 180
                y = y+2
            else:
                if corn == 180:
                    print("right")
                    send("right")
                    corn = 90
                    x = x+2


def path_lab(sent):
    global y, x, n, stat_lab, corn, stat_ezda, aa
    
    stat_lab = 0
    if (n == 3) or (lab[y, x] == 1):

        lab[y, x] = 1
        print("path")
        print("path")
        found = False
        neimber = [[0, -2], [2, 0], [0, 2], [-2, 0]]
        neimber_wall = [[0, -1], [1, 0], [0, 1], [-1, 0]]
        
        queue = [[x, y]]
        path = [-1]
        curr = 0
        
        while curr < len(queue):
            nx, ny = queue[curr][0], queue[curr][1]
            
            if lab[ny, nx] == sent and (nx != x or ny != y):
                found = True
                break
                
            for ab in range(len(neimber)):
                tx = nx + neimber[ab][0]
                ty = ny + neimber[ab][1]
                wx = nx + neimber_wall[ab][0]
                wy = ny + neimber_wall[ab][1]
                
                if 0 <= ty < 40 and 0 <= tx < 40:
                    if lab[wy, wx] == 0 and [tx, ty] not in queue:
                        queue.append([tx, ty])
                        path.append(curr)
            curr += 1

        if found == True:
            target_idx = curr
            actual_path = []
            while target_idx != -1:
                actual_path.append(queue[target_idx])
                target_idx = path[target_idx]
            actual_path.reverse()
            for i in range(1, len(actual_path)):

                tx, ty = actual_path[i]
                dx, dy = tx - x, ty - y

                if dy > 0:
                    print("back")
                    if corn == 0: send("back")
                    elif corn == 180: send("straight")
                    elif corn == 90: send("right")
                    elif corn == 270: send("left")
                    corn = 180
                    byte = ser.read(1)
                    ss = byte.decode('ascii')
                    while ss not in ['0', '1']:
                        byte = ser.read(1)
                        ss = byte.decode('ascii')
                elif dy < 0:
                    print("straight")
                    if corn == 0: send("straight")
                    elif corn == 180: send("back")
                    elif corn == 90: send("left")
                    elif corn == 270: send("right")
                    corn = 0
                    byte = ser.read(1)
                    s = byte.decode('ascii')
                    while s not in ['0', '1']:
                        byte = ser.read(1)
                        s = byte.decode('ascii')
                elif dx > 0:
                    print("right")
                    if corn == 0: send("right")
                    elif corn == 180: send("left")
                    elif corn == 90: send("straight")
                    elif corn == 270: send("back")
                    corn = 90
                    byte = ser.read(1)
                    s = byte.decode('ascii')
                    while s not in ['0', '1']:
                        byte = ser.read(1)
                        s = byte.decode('ascii')
                elif dx < 0:
                    print("left")
                    if corn == 0: send("left")
                    elif corn == 180: send("right")
                    elif corn == 90: send("back")
                    elif corn == 270: send("straight")
                    corn = 270
                    byte = ser.read(1)
                    s = byte.decode('ascii')
                    while s not in ['0', '1']:
                        byte = ser.read(1)
                        s = byte.decode('ascii')
                x, y = tx, ty
                if i < len(actual_path) - 1:
                    lab[y, x] = 1
            time.sleep(1)
            aa = 1
            stat_lab = 1

        # print("Path found:", actual_path)

        if found == False:
            stat_ezda = 1


def get_walls_lab(walls_1):
    walls_lab1 = [0, 0, 0, 0]
    sensor_offsets = [0, 90, 180, 270]
    
    for i in range(len(walls_1)):
        if walls_1[i] == 1:
            abs_angle = (corn + sensor_offsets[i]) % 360
            idx = abs_angle // 90
            walls_lab1[idx] = 1
            
    return walls_lab1



def my_map():
    global walls, read_res, walls_lab, lab_chek
    global n, y, x, y_old, x_old, chek

    read("walls")
    
    walls2 = [walls[0], walls[1], walls[2], walls[3]]
    walls_lab = get_walls_lab(walls2)

    print(walls)
    print(walls_lab)
    
    if walls[4] == 1:
        chek = 1
        lab_chek = lab

    if walls[5] == 1:
        y_old = y
        x_old = x
        if walls_lab[0] == 1: 
            n = n + 1
            lab[y-1, x] = 2
        else: lab[y-1, x] = 0

        if walls_lab[1] == 1: 
            lab[y, x+1] = 2
            n = n + 1
        else: lab[y, x+1] = 0
        
        if walls_lab[2] == 1: 
            n = n + 1
            lab[y+1, x] = 2
        else: lab[y+1, x] = 0

        if walls_lab[3] == 1: 
            n = n + 1
            lab[y, x-1] = 2
        else: lab[y, x-1] = 0

        print(lab[y-1, x], lab[y, x+1], lab[y+1, x], lab[y, x-1])
    
    elif walls[5] == 0:
        x = x_old
        y = y_old

    elif walls[5] == 2:
        lab[y, x] = 1
        if y_old == y:
            lab[y+1, x] = 1
            lab[y-1, x] = 1
            if x_old > x: x = x-2
            elif x_old < x: x = x+2
        elif x_old == x:
            lab[y, x+1] = 1
            lab[y, x-1] = 1
            if y_old > y: y = y-2
            elif y_old < y: y = y+2
    # if walls[4] == 1: lab[y, x] = 5
    # if walls[5] == 1: lab[y, x] = 6


def right():
    print('step_right')
    global y, x, walls, corn, x_old, y_old


    x_old = x
    y_old = y

    lab[y_old, x_old] = 1

    if walls[1] == 0:
        send("right")
        print("right")
        if corn == 0:
            x = x+2
            corn = 90
        elif corn == 90:
            y = y+2
            corn = 180
        elif corn == 180:
            x = x-2
            corn = 270
        elif corn == 270:
            y = y-2
            corn = 0
    elif walls[0] == 0:
        send("straight")
        print("straight")
        if corn == 0:
            y = y-2
            corn = 0
        elif corn == 90:
            x = x+2
            corn = 90
        elif corn == 180:
            y = y+2
            corn = 180
        elif corn == 270:
            x = x-2
            corn = 270
    elif walls[3] == 0:
        send("left")
        print("left")
        if corn == 0:
            x = x-2
            corn = 270
        elif corn == 90:
            y = y-2
            corn = 0
        elif corn == 180:
            x = x+2
            corn = 90
        elif corn == 270:
            y = y+2
            corn = 180
        
def pause():
    global lab, lab_chek
    byte = ser.read(1)
    s = byte.decode('ascii')
    if s == "p":
        print("pause")
        byte = ser.read(1)
        s = byte.decode('ascii')
        while s != "c":
            byte = ser.read(1)
            s = byte.decode('ascii')
        if chek == 1:
            lab = lab_chek
        elif chek == 0:
            x = 20
            y = 20
    
    s = 0

while True:
    global found

    lab[20, 20] = 3
    
    # camera()
    
    pause()

    if aa == 1:
        aa = 0
        ser.reset_input_buffer()

    print("stat", statv)
    byte = ser.read(1)
    s = byte.decode('ascii')
    #if input() == lab:
    #   np.set_printoptions(threshold=np.inf)
    #   print(lab)
    
    #print('goodx1')
    print("it is:", s)
    if s in ['0', '1']:
        print('goodx2')
        my_map()

        print(n)
        if stat_ezda == 2:
            print("goodx3")
            path_lab(0)
            if stat_ezda == 1:
                path_lab(3)
                print("end") 
                send("end") 

        if stat_lab == 0 or stat_ezda == 0:
            print('many')
            print(corn)
            right()
            stat_ezda = 2
            np.set_printoptions(threshold=np.inf)
            print(lab)

    n = 0
    b = 0

