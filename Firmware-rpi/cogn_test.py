import cv2
import numpy as np
from collections import Counter
import math

def get_color_name(hsv_pixel):
    h, s, v = hsv_pixel
    # Смягчили порог для черного
    if v < 60: 
        return "black"
    # Смягчили порог насыщенности (S) и яркости (V) до 50
    if (h < 10 or h > 170) and s > 50 and v > 50: 
        return "red"
    if 20 <= h <= 35 and s > 50 and v > 50: 
        return "yellow"
    if 36 <= h <= 85 and s > 50 and v > 50: 
        return "green"
    if 90 <= h <= 130 and s > 50 and v > 50: 
        return "blue"
    return "unknown"

def cogn_detect(crop):
    if crop is None or crop.size == 0:
        return None
    
    # Получаем реальные размеры текущего кропа
    h, w = crop.shape[:2]
    center_x, center_y = w // 2, h // 2
    
    # Вычисляем максимальный радиус (минимальная сторона делить на 2)
    # Это позволяет нам вписаться в круг цели, даже если кроп не квадратный
    max_radius = min(center_x, center_y)
    
    # Делим радиус на 5 сегментов
    step = max_radius / 5
    # Берем середину каждого сегмента, чтобы попасть в центр колец
    radii = [step * 0.5, step * 1.5, step * 2.5, step * 3.5, step * 4.5]
    
    hsv_roi = cv2.cvtColor(crop, cv2.COLOR_BGR2HSV)
    angles = [0, 72, 144, 216, 288]
    
    color_values = {"black": -2, "red": -1, "yellow": 0, "green": 1, "blue": 2}
    
    total_sum = 0
    print(f"--- Дебаг: размер кропа {w}x{h}, макс.радиус {max_radius} ---")
    
    for r in radii:
        zone_colors = []
        for angle in angles:
            rad = math.radians(angle)
            px = int(center_x + r * math.cos(rad))
            py = int(center_y + r * math.sin(rad))
            
            # Проверка границ
            if 0 <= px < w and 0 <= py < h:
                color_name = get_color_name(hsv_roi[py, px])
                if color_name != "unknown":
                    zone_colors.append(color_name)
        
        if zone_colors:
            counts = Counter(zone_colors)
            most_common_color, frequency = counts.most_common(1)[0]
            # Если 3 из 5 лучей сошлись во мнении — считаем это кольцом
            if frequency >= 3:
                val = color_values.get(most_common_color, 0)
                total_sum += val
                print(f"Радиус {r:.1f} (пикселей): цвет {most_common_color}, значение {val}")
            else:
                print(f"Радиус {r:.1f}: цвет неопределен, голоса: {counts}")
    
    print(f"--- Итоговая сумма: {total_sum} ---")
    return total_sum

# ТЕСТОВАЯ ЧАСТЬ
img_path = r'C:\Users\garin\Documents\Git\Maze-2025\rasp\0009_critical_-2.png'
img = cv2.imread(img_path)

if img is not None:
    result = cogn_detect(img)
    if result == 0:
        print("здоровая")
    elif result == 1:
        print("50/50")
    elif result == 2:
        print("ойойой")
    else:
        print("никакая")
else:
    print(f"Ошибка: файл не найден")