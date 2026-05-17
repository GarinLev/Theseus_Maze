import cv2
import time
import numpy as np
import serial
from ultralytics import YOLO
import threading
from collections import deque, Counter

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)

# Переменные лабиринта
walls = [0, 0, 0, 0, 0, 0]
lab = np.zeros((40, 40))
lab[20, 20] = 3
x, y = 20, 20
corn = 0

# Загрузка модели
detector_model = YOLO('/home/maze/YOLOv8_maze/train_code/runs/classify/train9/weights/best.pt')
detector_model.to('cpu')

# Исправление для Python 3.13 и атрибута 'bn'
def dummy_fuse(*args, **kwargs):
    return detector_model.model
detector_model.model.fuse = dummy_fuse
detector_model.model.eval()

class Color:
    def __init__(self, name, hsv_threshold, weight):
        self.hue_name = name
        self.hue_hsv_threshold = hsv_threshold
        self.hue_weight = weight

colors = (
    Color("red", ((0, 60, 40), (12, 255, 255), (165, 60, 40), (180, 255, 255)), -1),
    Color("blue", ((100, 100, 40), (130, 255, 255)), 2),
    Color("green", ((45, 60, 40), (85, 255, 255)), 1),
    Color("black", ((0, 0, 0), (180, 255, 45)), -2),
    Color("yellow", ((20, 80, 80), (35, 255, 255)), 0)
)

class MedianFilter:
    def __init__(self, size=12):
        self.buffer = deque(maxlen=size)
    def add(self, val):
        self.buffer.append(val)
    def get_median(self):
        valid_vals = [v for v in self.buffer if v is not None]
        if not valid_vals or len(valid_vals) < (self.buffer.maxlen // 2):
            return 0
        return int(np.median(valid_vals))

filter_l = MedianFilter(size=12)
filter_r = MedianFilter(size=12)

def get_instant_sum(frame):
    if frame is None: return None
    small = cv2.resize(frame, (160, 120))
    gray = cv2.cvtColor(small, cv2.COLOR_BGR2GRAY)
    _, mask = cv2.threshold(gray, 70, 255, cv2.THRESH_BINARY_INV)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    for cnt in contours:
        if cv2.contourArea(cnt) < 150: continue
        x_r, y_r, w_r, h_r = cv2.boundingRect(cnt)
        if max(w_r, h_r) / min(w_r, h_r) > 1.6: continue
        roi = small[y_r:y_r+h_r, x_r:x_r+w_r]
        if roi.size == 0: continue
        roi = cv2.resize(roi, (80, 80))
        roi_hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
        cx, cy = 40, 40
        tmp_sum, v_rings = 0, 0
        for r in [32, 20, 8]:
            sample = roi_hsv[cy + r - 3 : cy + r + 3, cx - 3 : cx + 3]
            if sample.size == 0: continue
            avg_px = np.mean(sample.reshape(-1, 3), axis=0)
            for color in colors:
                t = color.hue_hsv_threshold
                if (len(t) == 2 and np.all(avg_px >= t[0]) and np.all(avg_px <= t[1])) or \
                   (len(t) == 4 and ((np.all(avg_px >= t[0]) and np.all(avg_px <= t[1])) or 
                                     (np.all(avg_px >= t[2]) and np.all(avg_px <= t[3])))):
                    tmp_sum += color.hue_weight
                    v_rings += 1
                    break
        if v_rings >= 2: return tmp_sum
    return None

class VideoStream:
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        self.stream.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        self.stream.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
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

def send(action):
    if action == "rescue_1l": ser.write(b"b")
    if action == "rescue_2l": ser.write(b"m")
    if action == "rescue_1r": ser.write(b"v")
    if action == "rescue_2r": ser.write(b"n")

def camera():
    img = vs1.read()
    img2 = vs2.read()
    if img is None or img2 is None: return

    # YOLO Предикт без изменений (как в твоем примере)
    res = detector_model.predict(source=img, show=False, save=False, conf=0.2, 
                                 save_txt=False, save_crop=False, verbose=False, 
                                 device='cpu', half=False)[0]
    a = res.names[res.probs.top1]

    res2 = detector_model.predict(source=img2, show=False, save=False, conf=0.8, 
                                  save_txt=False, save_crop=False, verbose=False, 
                                  device='cpu')[0]
    a2 = res2.names[res2.probs.top1]

    # Расчет медианной суммы
    filter_l.add(get_instant_sum(img))
    filter_r.add(get_instant_sum(img2))
    stable_sum_l = filter_l.get_median()
    stable_sum_r = filter_r.get_median()

    # Отрисовка
    h, w = img.shape[:2]
    cv2.putText(img, f"Sum L: {stable_sum_l}", (20, h - 40), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)
    cv2.putText(img2, f"Sum R: {stable_sum_r}", (20, h - 40), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)

    # Логика спасения Левая камера
    if a == "nedoF" or stable_sum_l == 2:
        if lab[y, x] not in [4, 5]:
            send("rescue_1l")
            lab[y, x] = 4
    elif a == "F" or stable_sum_l == 2: # Используем elif чтобы не срабатывало дважды
        if lab[y, x] not in [4, 5]:
            send("rescue_2l")
            lab[y, x] = 5

    # Логика спасения Правая камера
    if a2 == "nedoF" or stable_sum_r == 1:
        if lab[y, x] not in [4, 5]:
            send("rescue_1r")
            lab[y, x] = 4
    elif a2 == "F" or stable_sum_r == 2:
        if lab[y, x] not in [4, 5]:
            send("rescue_2r")
            lab[y, x] = 5

    cv2.imshow("Left", img)
    cv2.imshow("Right", img2)
    cv2.waitKey(1)

while True:
    camera()