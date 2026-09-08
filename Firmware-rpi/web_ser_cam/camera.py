import cv2
import time
import numpy as np
from ultralytics import YOLO
import serial
from collections import Counter
import math
import socket
import os
import json

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
MONITOR_IP = "127.0.0.1"
MONITOR_PORT = 5002

detector_model = YOLO('/home/maze/MINSK/YOLOmazeMINSK/runs/detect/train/weights/best.pt')
detector_model.to('cpu')

check_res1 = None
check_res2 = None
crop1 = None
crop2 = None
curr_cell_x = 8
curr_cell_y = 8
last_cell_x = None
last_cell_y = None
reported = set()

cam1 = cv2.VideoCapture(0)
cam2 = cv2.VideoCapture(2)

cam1.set(cv2.CAP_PROP_BUFFERSIZE, 1)
cam2.set(cv2.CAP_PROP_BUFFERSIZE, 1)

def send():
    global check_res1, check_res2

    if check_res1 == "0":
        ser.write(b"c")
        sock_out.sendto(b"victim:l:0", (MONITOR_IP, MONITOR_PORT))
        print("0")
        reported.add((curr_cell_x, curr_cell_y, 1))
    elif check_res1 == "1":
        ser.write(b"v")
        sock_out.sendto(b"victim:l:1", (MONITOR_IP, MONITOR_PORT))
        print("1")
        reported.add((curr_cell_x, curr_cell_y, 1))
    elif check_res1 == "2":
        ser.write(b"n")
        sock_out.sendto(b"victim:l:2", (MONITOR_IP, MONITOR_PORT))
        print("2")
        reported.add((curr_cell_x, curr_cell_y, 1))
    
    if check_res2 == "0":
        ser.write(b"c")
        sock_out.sendto(b"victim:r:0", (MONITOR_IP, MONITOR_PORT))
        print("0")
        reported.add((curr_cell_x, curr_cell_y, 2))
    elif check_res2 == "1":
        ser.write(b"b")
        sock_out.sendto(b"victim:r:1", (MONITOR_IP, MONITOR_PORT))
        print("1")
        reported.add((curr_cell_x, curr_cell_y, 2))
    elif check_res2 == "2":
        ser.write(b"m")
        sock_out.sendto(b"victim:r:2", (MONITOR_IP, MONITOR_PORT))
        print("2")
        reported.add((curr_cell_x, curr_cell_y, 2))

def results(res, img, min_size=50):
    boxes = res.boxes
    if len(boxes) == 0:
        return None
    
    sorted_boxes = sorted(boxes, key=lambda x: x.conf.item(), reverse=True)
    box = sorted_boxes[0]  
    x1, y1, x2, y2 = map(int, box.xyxy[0])
    class_name = res.names[int(box.cls[0])]
    w = x2 - x1
    h = y2 - y1

    if w < min_size or h < min_size:
        return None
            
    return {
        'name': class_name,
        'crop': img[y1:y2, x1:x2],
        'x': (x1 + x2) // 2,
        'y': (y1 + y2) // 2
    }

def check():
    global check_res1, check_res2, cogn_x1, cogn_x2, cogn_y1, cogn_y2, crop1, crop2
    check_res1 = check_res2 = cogn_x1 = cogn_x2 = cogn_y1 = cogn_y2 = crop1 = crop2 = None

    for _ in range(4): 
        cam1.grab()
    ret1, img1 = cam1.retrieve()

    for _ in range(4): 
        cam2.grab()
    ret2, img2 = cam2.retrieve()
    
    if not ret1 or not ret2:
        return
    
    res1 = None
    res2 = None
    data1 = None
    data2 = None

    if (curr_cell_x, curr_cell_y, 1) not in reported:
        res1 = detector_model.predict(
            source=img1, show=False, save=False, conf=0.8,
            save_txt=False, save_crop=False, verbose=False, device='cpu', half=False
        )[0]
        data1 = results(res1, img1)

    if (curr_cell_x, curr_cell_y, 2) not in reported:
        res2 = detector_model.predict(
            source=img2, show=False, save=False, conf=0.6,
            save_txt=False, save_crop=False, verbose=False, device='cpu', half=False
        )[0]
        data2 = results(res2, img2)

    findings = []
    if data1: findings.append({'camera': 1, 'data': data1})
    if data2: findings.append({'camera': 2, 'data': data2})

    for item in findings:
        cam = item['camera']
        name = item['data']['name']
        crop = item['data']['crop']

        if name == 'cogn':
            if cam == 1: 
                check_res1 = "cogn"
                cogn_x1 = item['data']['x']
                cogn_y1 = item['data']['y']
                crop1 = crop
            else: 
                check_res2 = "cogn"
                cogn_x2 = item['data']['x']
                cogn_y2 = item['data']['y']
                crop2 = crop
        elif name == 'OM':
            if cam == 1: check_res1 = "0"
            else: check_res2 = "0"
        elif name == 'nedoF':
            if cam == 1: check_res1 = "1"
            else: check_res2 = "1"
        elif name == 'F':
            if cam == 1: check_res1 = "2"
            else: check_res2 = "2"

    if res1 is not None:
        cv2.imshow("neyronka1", res1.plot())
    if res2 is not None:
        cv2.imshow("neyronka2", res2.plot())
    cv2.waitKey(1)

def get_color_name(hsv_pixel):
    h, s, v = int(hsv_pixel[0]), int(hsv_pixel[1]), int(hsv_pixel[2])
    
    if v < 70 or (s < 30 and v > 150):
        return "black"

    if (0 <= h <= 10) or (165 <= h <= 180):
        if s > 50 and v > 40: return "red"
    elif 11 <= h <= 34:
        if s > 50 and v > 40: return "yellow"
    elif 35 <= h <= 85:
        if s > 40 and v > 40: return "green"
    elif 86 <= h <= 130:
        if s > 40 and v > 40: return "blue"

    return "unknown"

def cogn_detect(crop, cam_num, bbox=None):
    if crop is None or crop.size == 0:
        return None
        
    gray = cv2.cvtColor(crop, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, thresh = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    h, w = crop.shape[:2]
    center_x, center_y = w // 2, h // 2
    max_radius = min(center_x, center_y)
    
    if contours:
        largest_contour = max(contours, key=cv2.contourArea)
        M = cv2.moments(largest_contour)
        if M["m00"] != 0:
            center_x = int(M["m10"] / M["m00"])
            center_y = int(M["m01"] / M["m00"])
            max_radius = math.sqrt(M["m00"] / math.pi)
        
    img_debug = crop.copy()
    hsv_roi = cv2.cvtColor(crop, cv2.COLOR_BGR2HSV)
    
    step = max_radius / 5
    radii = [step * 0.4, step * 1.6, step * 2.6, step * 3.6, step * 4.4]
    
    angles = [0, 72, 144, 216, 288]
    color_values = {"black": -2, "red": -1, "yellow": 0, "green": 1, "blue": 2}
    total_sum = 0
    
    for r_idx, r in enumerate(radii):
        zone_colors = []
        for angle in angles:
            rad = math.radians(angle)
            px = int(center_x + r * math.cos(rad))
            py = int(center_y + r * math.sin(rad))
            
            if 0 <= px < w and 0 <= py < h:
                color_name = get_color_name(hsv_roi[py, px])
                if color_name != "unknown":
                    zone_colors.append(color_name)
                    cv2.circle(img_debug, (px, py), 3, (0, 255, 0), -1)
        
        if zone_colors:
            counts = Counter(zone_colors)
            most_common_color, frequency = counts.most_common(1)[0]
            
            threshold = 3 if most_common_color == "black" else 4
            if frequency >= threshold:
                val = color_values.get(most_common_color, 0)
                total_sum += val
                
    cv2.imshow(f"Scan_Cam{cam_num}", img_debug)
    cv2.waitKey(1)
    
    if total_sum == 0: return "0"
    elif total_sum == 1: return "1"
    elif total_sum == 2: return "2"
    elif total_sum < 0 or total_sum > 2: return "0"
    return None

histories = {1: [], 2: []}

def update_history(cam_num, res):
    if not res:
        return None
    hist = histories[cam_num]
    hist.append(res)
    if len(hist) > 3: 
        hist.pop(0)
    if len(hist) == 3 and hist.count(hist[0]) == 3:
        return hist[0]
    return None

while True:
    # Прямое чтение координат без try-except
    if os.path.exists("maze_shared.json") and os.path.getsize("maze_shared.json") > 0:
        with open("maze_shared.json", "r", encoding="utf-8") as f:
            data = json.load(f)
            
        if isinstance(data, dict) and "robot_x" in data and "robot_y" in data:
            rx_idx = data["robot_x"]
            ry_idx = data["robot_y"]
            curr_cell_x = rx_idx // 2
            curr_cell_y = ry_idx // 2
            
            # Проверяем, отмечена ли текущая клетка в общей матрице как обработанная жертва (4)
            matrix = data.get("matrix", [])
            if 0 <= ry_idx < len(matrix) and 0 <= rx_idx < len(matrix[0]):
                if matrix[ry_idx][rx_idx] == 4:
                    reported.add((curr_cell_x, curr_cell_y, 1))
                    reported.add((curr_cell_x, curr_cell_y, 2))

    if curr_cell_x != last_cell_x or curr_cell_y != last_cell_y:
        reported.discard((curr_cell_x, curr_cell_y, 1))
        reported.discard((curr_cell_x, curr_cell_y, 2))
        histories[1].clear()
        histories[2].clear()
        last_cell_x = curr_cell_x
        last_cell_y = curr_cell_y

    check()
    
    if check_res1 == "cogn":
        res1 = cogn_detect(crop1, cam_num=1)
        check_res1 = update_history(1, res1)
                
    if check_res2 == "cogn":
        res2 = cogn_detect(crop2, cam_num=2)
        check_res2 = update_history(2, res2)
    
    send()