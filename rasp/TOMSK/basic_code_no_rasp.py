import cv2
import time
import numpy as np
import serial
from ultralytics import YOLO
import cv2

walls = [0, 0, 0, 0]
lab = np.zeros((40, 40))
lab[20, 20] = 3
x = 20
y = 20
corn = 0
stat_ezda = 0

while input() != "begin":
    time.sleep(0.01)

print("ready")

stat_lab = 2
stat = 1
n = 0

       

def ezda_r():
    global x, y, corn

    if corn == 0:
        print("right")
        corn = 90
        x = x+2
    else:
        if corn == -90:
            print("straight")
            y = y-2
            corn = 0
        else:
            if corn == 90:
                print("back")
                y = y+2
                corn = 180
            else:
                if corn == 180:
                    print("left")
                    corn = -90
                    x = x-2


def ezda_l():
    global x, y, corn

    if corn == 0:
        print("left")
        corn = -90
        x = x-2
    else:
        if corn == 90:
            print("straight")
            corn = 0
            y = y-2
        else:
            if corn == -90:
                print("back")
                corn = 180
                y = y+2
            else:
                if corn == 180:
                    print("right")
                    corn = 90
                    x = x+2


def path_lab(sent):
    global y, x, n, stat_lab, corn, stat_ezda

    stat_lab = 0
    if (n == 3) or (lab[y, x] == 1):

        lab[y, x] = 1

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
                    corn = 180
                elif dy < 0:
                    print("straight")
                    corn = 0
                elif dx > 0:
                    print("right")
                    corn = 90
                elif dx < 0:
                    print("left")
                    corn = -90

                x, y = tx, ty
                if i < len(actual_path) - 1:
                    lab[y, x] = 1
            
            stat_lab = 1

        # print("Path found:", actual_path)

        if found == False:
            stat_ezda = 1


def my_map():
    global walls
    global n, y, x

    print("sensors")
    sensors = input()
    walls = [int(digit) for digit in str(sensors)]

    print(walls)

    if walls[0] == 1: 
        n = n + 1
        lab[y-1, x] = 2
    else: lab[y-1, x] = 0

    if walls[1] == 1: 
        lab[y, x+1] = 2
        n = n + 1
    else: lab[y, x+1] = 0
    
    if walls[2] == 1: 
        n = n + 1
        lab[y+1, x] = 2
    else: lab[y+1, x] = 0

    if walls[3] == 1: 
        n = n + 1
        lab[y, x-1] = 2
    else: lab[y, x-1] = 0

    print(lab[y-1, x], lab[y, x+1], lab[y+1, x], lab[y, x-1])


    # if walls[4] == 1: lab[y, x] = 5
    # if walls[5] == 1: lab[y, x] = 6


def right():
    global y, x, walls, corn

    x_old = x
    y_old = y

    lab[y_old, x_old] = 1

    if walls[1] == 0: 
        ezda_r()     
    else: 
        if walls[0] == 0: 
            print("straight")
            y = y-2
            corn = 0
        else:
            if walls[3] == 0: 
                ezda_l()
        




while True:
    global found

    lab[20, 20] = 3
    stat_sending = input()
    # while stat_sending != "start" and stat == 1:
    #     stat_sending = input()
    #     time.sleep(0.01)

    # stat = 0
    if stat_sending == "lab":
        np.set_printoptions(threshold=np.inf)
        print(lab)
    
    my_map()

    print(n)
    if stat_ezda == 2:
        path_lab(0)
        if stat_ezda == 1:
            path_lab(3)
            print("end")  

    if stat_lab == 0 or stat_ezda == 0:
        print(corn)
        right()
        stat_ezda = 2

    n = 0