import cv2
import time
import numpy as np
from ultralytics import YOLO
import serial
from collections import Counter
import math
import socket
import json
import os
import glob
import threading
import select

class BackgroundCamera:
    def __init__(self, src):
        self.cap = cv2.VideoCapture(src)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        self.ret, self.frame = False, None
        self.started = False
        
    def start(self):
        self.started = True
        threading.Thread(target=self.update, daemon=True).start()
        return self
        
    def update(self):
        while self.started:
            ret, frame = self.cap.read()
            if ret:
                self.ret, self.frame = ret, frame
            time.sleep(0.01)
            
    def read(self):
        return self.ret, self.frame

cam1 = BackgroundCamera(0).start()
cam2 = BackgroundCamera(2).start()

os.environ["QT_LOGGING_RULES"] = "*.debug=false;qt.qpa.font=false"

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
MONITOR_IP = "127.0.0.1"
MONITOR_PORT = 5002

camera_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
camera_server.bind(('127.0.0.1', 5004))
camera_server.setblocking(False)

is_paused = False

detector_model = YOLO('/home/maze/MINSK/best.pt')
detector_model.to('cpu')

check_res1 = None
check_res2 = None
crop1 = None
crop2 = None
curr_cell_x = 15
curr_cell_y = 15
last_cell_x = None
last_cell_y = None
reported = set()

# cam1 = "dev/cam_left"
# cam1 = cv2.VideoCapture(2)
# cam2 = cv2.VideoCapture(0)
# cam2 = "dev/cam_right"

# cam1.set(cv2.CAP_PROP_BUFFERSIZE, 1)
# cam2.set(cv2.CAP_PROP_BUFFERSIZE, 1)
def load_data_from_monitor():
    filename = "maze_shared.json"
    if os.path.exists(filename) and os.path.getsize(filename) > 0:
        try:
            with open(filename, "r", encoding="utf-8") as f:
                content = f.read().strip()
            if content.startswith("{") and content.endswith("}"):
                data = json.loads(content)
                if isinstance(data, dict):
                    return data  # <--- Возвращаем весь словарь, а не кортеж!
        except Exception:
            pass
    return {}

def send():
    global check_res1, check_res2

    if check_res1 == "0":
        ser.write(b"c")
        sock_out.sendto(b"victim:l:0", (MONITOR_IP, MONITOR_PORT))
        print("isend0")
        reported.add((curr_cell_x, curr_cell_y, 1))
    
    if check_res2 == "0":
        ser.write(b"x")
        sock_out.sendto(b"victim:r:0", (MONITOR_IP, MONITOR_PORT))
        print("send_box0")
        reported.add((curr_cell_x, curr_cell_y, 2))

def check():
    global check_res1, check_res2, cogn_x1, cogn_x2, cogn_y1, cogn_y2, crop1, crop2
    check_res1 = check_res2 = cogn_x1 = cogn_x2 = cogn_y1 = cogn_y2 = crop1 = crop2 = None

    ret1, img1 = cam1.read()
    ret2, img2 = cam2.read()
    
    if not ret1 or not ret2:
        return
    
    res1 = None
    res2 = None
    data1 = None
    data2 = None

    if (curr_cell_x, curr_cell_y, 1) not in reported:
        res1 = detector_model.predict(
            source=img1, 
            show=False, 
            save=False, 
            conf=0.85,
            imgsz=320,
            save_txt=False, 
            save_crop=False, 
            verbose=False, 
            device='cpu'
        )[0]
        data1 = res1

    if (curr_cell_x, curr_cell_y, 2) not in reported:
        res2 = detector_model.predict(
            source=img2, show=False, save=False, conf=0.85,
            save_txt=False, save_crop=False, verbose=False, device='cpu', half=False
        )[0]
        data2 = res2

    findings = []
    if data1: findings.append({'camera': 1, 'data': data1})
    if data2: findings.append({'camera': 2, 'data': data2})

    for item in findings:
        cam = item['camera']
        name = item['data']['name']
        crop = item['data']['crop']

        if name == 'H':
            if cam == 1: check_res1 = "0"
            else: check_res2 = "0"
        elif name == 'S':
            if cam == 1: check_res1 = "0"
            else: check_res2 = "0"

    if res1 is not None:
        cv2.imshow("neyronka1", res1.plot())
    if res2 is not None:
        cv2.imshow("neyronka2", res2.plot())
    cv2.waitKey(1)

histories = {1: [], 2: []}

def update_history(cam_num, res):
    if not res:
        return None
    hist = histories[cam_num]
    hist.append(res)
    if len(hist) > 3:
        hist.pop(0)
    if len(hist) >= 2:
        counts = Counter(hist)
        best, cnt = counts.most_common(1)[0]
        if cnt >= 2:          # достаточно 2 одинаковых из последних 3
            return best
    return None

while True:
    msgs = []
    while select.select([camera_server], [], [], 0)[0]:
        data_udp, _ = camera_server.recvfrom(1024)
        msgs.append(data_udp.decode('utf-8').strip())
    
    if len(msgs) > 2:
        msgs.pop()
        
    for msg in msgs:
        if msg == "pause":
            is_paused = True
        elif msg == "resume":
            is_paused = False

    if is_paused:
        time.sleep(0.02)
        continue

    data = load_data_from_monitor()

    if "robot_x" in data and "robot_y" in data:
        curr_cell_x = data["robot_x"] // 2
        curr_cell_y = data["robot_y"] // 2

    if curr_cell_x != last_cell_x or curr_cell_y != last_cell_y:
        reported.discard((curr_cell_x, curr_cell_y, 1))
        reported.discard((curr_cell_x, curr_cell_y, 2))
        histories[1].clear()
        histories[2].clear()
        last_cell_x = curr_cell_x
        last_cell_y = curr_cell_y

    # if isinstance(data, dict):
    #     cell_key = f"{curr_cell_x},{curr_cell_y}"
    #     if any(v.get("pos") == cell_key for v in data.get("victims", [])):
    #         reported.add((curr_cell_x, curr_cell_y, 1))
    #         reported.add((curr_cell_x, curr_cell_y, 2))

    check()
    
    send()

    time.sleep(0.02)
