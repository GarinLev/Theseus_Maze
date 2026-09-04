import cv2
import numpy as np
from collections import Counter
import math

def get_color_name(hsv_pixel):
    h, s, v = int(hsv_pixel[0]), int(hsv_pixel[1]), int(hsv_pixel[2])
    
    if v < 60:
        return "black"

    colors = {
        "red1": (0, 255, 255),
        "red2": (179, 255, 255),
        "yellow": (30, 255, 255),
        "green": (60, 255, 255),
        "blue": (120, 255, 255)
    }

    min_dist = float('inf')
    closest = "black"

    for name, (ref_h, ref_s, ref_v) in colors.items():
        dh = min(abs(h - ref_h), 180 - abs(h - ref_h))
        ds = abs(s - ref_s)
        dv = abs(v - ref_v)
        
        # Считаем Евклидово расстояние (оттенок в приоритете x2)
        dist = (dh * 2)**2 + ds**2 + dv**2
        
        if dist < min_dist:
            min_dist = dist
            closest = "red" if "red" in name else name

    return closest

def warp_perspective_to_square(img, bbox):
    x1, y1, x2, y2 = bbox
    pts1 = np.float32([[x1, y1], [x2, y1], [x1, y2], [x2, y2]])
    size = 200
    pts2 = np.float32([[0, 0], [size, 0], [0, size], [size, size]])
    matrix = cv2.getPerspectiveTransform(pts1, pts2)
    result = cv2.warpPerspective(img, matrix, (size, size))
    return result

def cogn_detect(crop, bbox=None):
    if crop is None or crop.size == 0:
        return None
    
    # Если bbox не передан, берем размеры самой картинки
    if bbox is None:
        bbox = [0, 0, crop.shape[1], crop.shape[0]]
        
    roi_squared = warp_perspective_to_square(crop, bbox)
    hsv_roi = cv2.cvtColor(roi_squared, cv2.COLOR_BGR2HSV)
    
    h, w = roi_squared.shape[:2]
    center_x, center_y = w // 2, h // 2
    max_radius = min(center_x, center_y)
    step = max_radius / 5
    radii = [step * 0.5, step * 1.5, step * 2.5, step * 3.5, step * 4.5]
    
    angles = [0, 72, 144, 216, 288]
    color_values = {"black": -2, "red": -1, "yellow": 0, "green": 1, "blue": 2}
    total_sum = 0
    
    for r in radii:
        zone_colors = []
        for angle in angles:
            rad = math.radians(angle)
            px = int(center_x + r * math.cos(rad))
            py = int(center_y + r * math.sin(rad))
            
            if 0 <= px < w and 0 <= py < h:
                color_name = get_color_name(hsv_roi[py, px])
                # unknown здесь больше никогда не появится, но проверку оставляем для безопасности
                if color_name != "unknown": 
                    zone_colors.append(color_name)
        
        if zone_colors:
            counts = Counter(zone_colors)
            most_common_color, frequency = counts.most_common(1)[0]
            if frequency >= 3:
                total_sum += color_values.get(most_common_color, 0)
                print(f"Радиус {r:.1f}: определен цвет {most_common_color}")
                
    if total_sum == 0:
        return "0"
    elif total_sum == 1:
        return "1"
    elif total_sum == 2:
        return "2"
    return None

# ==========================================
# ТЕСТОВАЯ ЗОНА
# ==========================================
if __name__ == "__main__":
    # Вставь сюда путь к любой вырезанной метке





    
    img_path = r'C:\Users\garin\Documents\Git\Maze-2025\rasp\photo_43.jpg' 
    
    print(f"Загружаем изображение: {img_path}")
    test_img = cv2.imread(img_path)

    if test_img is not None:
        print("--- Начинаем сканирование ---")
        result = cogn_detect(test_img)
        
        print("\n--- Итог ---")
        print(f"Результат функции: {result}")
        
        if result == "2":
            print("Расшифровка: Поврежденная жертва (2)")
        elif result == "1":
            print("Расшифровка: Стабильная жертва (1)")
        elif result == "0":
            print("Расшифровка: Невредимая жертва (0)")
        else:
            print("Расшифровка: Цель не распознана (None)")
            
        cv2.imshow("Test Image", test_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    else:
        print("Ошибка: Не удалось загрузить картинку. Проверь путь к файлу!")