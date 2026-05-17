import socket
import os
import imgui_bundle
from imgui_bundle import imgui, immapp, hello_imgui

LAB_SIZE = 33  

class MonitorState:
    def __init__(self):
        # 0: unexplored, 1: explored, 2: wall, 3: start, 4: wall with victim
        self.lab = [[0 for _ in range(LAB_SIZE)] for _ in range(LAB_SIZE)]
        
        # Стартовая позиция робота (в чётных координатах)
        self.robot_x, self.robot_y = 16, 16
        self.lab[self.robot_y][self.robot_x] = 3  # Помечаем как старт
        
        # ИСПОЛЬЗУЕМ CORN: по умолчанию 0 градусов (вверх)
        self.corn = 0
        self.input_logs = ["System thin walls 33x33 started."]
        self.found_victims = []

mon_state = MonitorState()

# Настройка сети UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', 5002))
sock.setblocking(False)

def check_network():
    try:
        data, addr = sock.recvfrom(1024)
        text = data.decode('utf-8')
        parts = text.split(':')
        cmd = parts[0]

        mon_state.input_logs.append(f"[UDP]: {text}")
        if len(mon_state.input_logs) > 40:
            mon_state.input_logs.pop(0)

        vx, vy = mon_state.robot_x, mon_state.robot_y

        if cmd == 'pos':
            coords = parts[1].split(',')
            mon_state.robot_x = int(coords[0]) * 2
            mon_state.robot_y = int(coords[1]) * 2
            # ПРИНИМАЕМ УГОЛ ИЗ ПАКЕТА (переводим в int)
            mon_state.corn = int(parts[2])
            
            if mon_state.lab[mon_state.robot_y][mon_state.robot_x] == 0:
                mon_state.lab[mon_state.robot_y][mon_state.robot_x] = 1
                
        elif cmd == 'wall':
            coords = parts[1].split(',')
            cx, cy = int(coords[0]) * 2, int(coords[1]) * 2
            wall_list = parts[2].split(',')
            
            if 0 <= cx < LAB_SIZE and 0 <= cy < LAB_SIZE:
                if 'up' in wall_list and cy - 1 >= 0:
                    if mon_state.lab[cy - 1][cx] != 4: mon_state.lab[cy - 1][cx] = 2
                if 'down' in wall_list and cy + 1 < LAB_SIZE:
                    if mon_state.lab[cy + 1][cx] != 4: mon_state.lab[cy + 1][cx] = 2
                if 'left' in wall_list and cx - 1 >= 0:
                    if mon_state.lab[cy][cx - 1] != 4: mon_state.lab[cy][cx - 1] = 2
                if 'right' in wall_list and cx + 1 < LAB_SIZE:
                    if mon_state.lab[cy][cx + 1] != 4: mon_state.lab[cy][cx + 1] = 2

        elif cmd == 'victim':
            vic_label = parts[2]
            mon_state.found_victims.append({'pos': f"{vx//2},{vy//2}", 'label': vic_label})
            
            # Автоматическая метка перед носом по углу corn
            if (mon_state.corn == 0 or mon_state.corn == 360) and vy - 1 >= 0:    mon_state.lab[vy - 1][vx] = 4
            elif mon_state.corn == 180 and vy + 1 < LAB_SIZE:                     mon_state.lab[vy + 1][vx] = 4
            elif mon_state.corn == 270 and vx - 1 >= 0:                           mon_state.lab[vy][vx - 1] = 4
            elif mon_state.corn == 90 and vx + 1 < LAB_SIZE:                      mon_state.lab[vy][vx + 1] = 4
            print(f"[Map] Метка {vic_label} выставлена перед роботом.")

        elif cmd == 'vl':
            # ЧЁТКАЯ ОТНОСИТЕЛЬНАЯ ЛОГИКА СЛЕВА ОТ СТРЕЛОЧКИ ПО УГЛУ CORN
            if mon_state.corn == 0 or mon_state.corn == 360:
                if vx - 1 >= 0: mon_state.lab[vy][vx - 1] = 4
            elif mon_state.corn == 180:
                if vx + 1 < LAB_SIZE: mon_state.lab[vy][vx + 1] = 4
            elif mon_state.corn == 270:
                if vy + 1 < LAB_SIZE: mon_state.lab[vy + 1][vx] = 4
            elif mon_state.corn == 90:
                if vy - 1 >= 0: mon_state.lab[vy - 1][vx] = 4
            print(f"[Map] Тонкая зелёная стена СЛЕВА (угол {mon_state.corn}°).")

        elif cmd == 'vr':
            # ЧЁТКАЯ ОТНОСИТЕЛЬНАЯ ЛОГИКА СПРАВА ОТ СТРЕЛОЧКИ ПО УГЛУ CORN
            if mon_state.corn == 0 or mon_state.corn == 360:
                if vx + 1 < LAB_SIZE: mon_state.lab[vy][vx + 1] = 4
            elif mon_state.corn == 180:
                if vx - 1 >= 0: mon_state.lab[vy][vx - 1] = 4
            elif mon_state.corn == 270:
                if vy - 1 >= 0: mon_state.lab[vy - 1][vx] = 4
            elif mon_state.corn == 90:
                if vy + 1 < LAB_SIZE: mon_state.lab[vy + 1][vx] = 4
            print(f"[Map] Тонкая зелёная стена СПРАВА (угол {mon_state.corn}°).")

    except BlockingIOError:
        pass

# =====================================================================
# ОСНОВНОЙ ЦИКЛ ГРАФИКИ
# =====================================================================
def gui_frame():
    # # Заведите статическую переменную для хранения времени (добавьте это в начало gui_frame)
    # if not hasattr(gui_frame, "last_print_time"):
    #     gui_frame.last_print_time = 0.0

    # # Проверяем, прошла ли 1 секунда с момента последнего принта
    # current_time = imgui.get_time()
    # if current_time - gui_frame.last_print_time > 1.0:
    #     gui_frame.last_print_time = current_time
        
    #     print("\n=== ТЕКУЩИЙ МАССИВ ЛАБИРИНТА ===")
    #     for row in mon_state.lab:
    #         # Красивый вывод строки через пробелы
    #         print(" ".join(str(cell) for cell in row))
    #     print("=================================")
    check_network()
    
    display_size = imgui.get_io().display_size
    scr_w = display_size.x
    scr_h = display_size.y
    
    #ЛЕВАЯ ЧАСТЬ ЭКРАНА
    imgui.set_next_window_pos(imgui.ImVec2(10, 10))
    imgui.set_next_window_size(imgui.ImVec2(scr_w * 0.5 - 15, scr_h - 20))
    imgui.begin("Maze Live", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
    
    draw_list = imgui.get_window_draw_list()
    screen_pos = imgui.get_cursor_screen_pos()
    
    map_available_w = (scr_w * 0.5 - 40)
    map_available_h = (scr_h - 60)
    cell_pixels = min(map_available_w, map_available_h) / LAB_SIZE
    
    # ЦВЕТА
    color_unexplored = imgui.get_color_u32(imgui.ImVec4(0.08, 0.08, 0.1, 1.0)) 
    color_explored   = imgui.get_color_u32(imgui.ImVec4(0.22, 0.22, 0.28, 1.0)) 
    color_wall       = imgui.get_color_u32(imgui.ImVec4(1.0, 0.4, 0.0, 1.0))    # Оранжевый
    color_victim_wall= imgui.get_color_u32(imgui.ImVec4(0.0, 1.0, 0.3, 1.0))    # Зелёный
    color_start      = imgui.get_color_u32(imgui.ImVec4(0.4, 0.15, 0.6, 1.0))   
    color_robot      = imgui.get_color_u32(imgui.ImVec4(0.0, 0.45, 0.9, 1.0))   
    
    # --- ДИНАМИЧЕСКИЙ РАСЧЕТ НЕРАВНОМЕРНОЙ СЕТКИ ---
    # Пропорция: стены в 4-5 раз тоньше основных клеток пола
    cell_thin = cell_pixels * 0.3   # Толщина нечетных элементов (стен)
    cell_thick = cell_pixels * 1.7  # Ширина четных элементов (комнат)

    # Заполняем массивы опорных экранных координат для сетки
    xs = [screen_pos.x]
    ys = [screen_pos.y]
    for i in range(LAB_SIZE):
        size = cell_thick if i % 2 == 0 else cell_thin
        xs.append(xs[-1] + size)
        ys.append(ys[-1] + size)

    # Заливаем базовый фон по крайним вычисленным точкам
    draw_list.add_rect_filled(imgui.ImVec2(xs[0], ys[0]), imgui.ImVec2(xs[LAB_SIZE], ys[LAB_SIZE]), color_unexplored)

    # Отрисовка элементов лабиринта по новой сетке координат
    for y in range(LAB_SIZE):
        for x in range(LAB_SIZE):
            x1, x2 = xs[x], xs[x + 1]
            y1, y2 = ys[y], ys[y + 1]
            
            val = mon_state.lab[y][x]
            
            # 1. Рисуем пол клеток (четные координаты)
            if x % 2 == 0 and y % 2 == 0:
                bg_color = color_start if val == 3 else (color_explored if val == 1 else color_unexplored)
                draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), bg_color)
                # Бледная сетка-ориентир для больших клеток
                draw_list.add_rect(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), imgui.get_color_u32(imgui.ImVec4(0.14, 0.14, 0.16, 0.5)), 0.0, 1.0)

            # 2. Отрисовка тонких стен (заливаем суженную нечетную ячейку)
            elif val == 2 or val == 4:
                current_color = color_victim_wall if val == 4 else color_wall
                draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), current_color)
                                
    # Рисуем робота с привязкой к динамической сетке
    rx1, rx2 = xs[mon_state.robot_x], xs[mon_state.robot_x + 1]
    ry1, ry2 = ys[mon_state.robot_y], ys[mon_state.robot_y + 1]
    robot_w = rx2 - rx1
    
    draw_list.add_rect_filled(imgui.ImVec2(rx1, ry1), imgui.ImVec2(rx2, ry2), color_robot, 2.0)

    # ПРИВЯЗЫВАЕМ СТРЕЛОЧКУ К УГЛАМ CORN И ЦЕНТРИРУЕМ В КЛЕТКЕ
    arrows = {0: "^", 360: "^", 90: ">", 180: "v", 270: "<"}
    white_color = imgui.get_color_u32(imgui.ImVec4(1.0, 1.0, 1.0, 1.0))
    current_arrow = arrows.get(mon_state.corn, "^")
    draw_list.add_text(imgui.ImVec2(rx1 + (robot_w * 0.35), ry1 + (robot_w * 0.2)), white_color, current_arrow)

    imgui.end()

    # -----------------------------------------------------------------
    # 2. ПРАВАЯ ВЕРХНЯЯ ЧАСТЬ: ЛОГ НАЙДЕННЫХ БУКВ
    # -----------------------------------------------------------------
    right_x = scr_w * 0.5 + 5
    right_w = scr_w * 0.5 - 15
    cam_h = scr_h * 0.4 - 10
    
    imgui.set_next_window_pos(imgui.ImVec2(right_x, 10))
    imgui.set_next_window_size(imgui.ImVec2(right_w, cam_h))
    imgui.begin("Victims", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
    
    if not mon_state.found_victims:
        imgui.text_disabled("No victims detected yet. Waiting for robot AI data...")
    else:
        for vic in reversed(mon_state.found_victims):
            imgui.spacing()
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.4, 1.0), f"[CRITICAL] Found Type '{vic['label']}' at cell [{vic['pos']}]")
            imgui.separator()
            
    imgui.end()

    # -----------------------------------------------------------------
    # 3. ПРАВАЯ НИЖНЯЯ ЧАСТЬ: ЛОГ UDP ПАКЕТОВ
    # -----------------------------------------------------------------
    log_y = cam_h + 20
    log_h = scr_h - log_y - 10
    
    imgui.set_next_window_pos(imgui.ImVec2(right_x, log_y))
    imgui.set_next_window_size(imgui.ImVec2(right_w, log_h))
    imgui.begin("UDP Packets Terminal", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
    
    imgui.begin_child("ScrollZone", imgui.ImVec2(0, 0), True, imgui.WindowFlags_.horizontal_scrollbar)
    for log in reversed(mon_state.input_logs):
        if "wall" in log:
            imgui.text_colored(imgui.ImVec4(1.0, 0.6, 0.2, 1.0), log)
        elif "victim" in log or "vl" in log or "vr" in log:
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.2, 1.0), log)
        else:
            imgui.text_colored(imgui.ImVec4(0.3, 0.7, 1.0, 1.0), log)
    imgui.end_child()
    
    imgui.end()

def main():
    # Проверьте, чтобы перед этой строкой НЕ было пробелов или табов:
    params = hello_imgui.RunnerParams()
    params.app_window_params.window_title = "Rescue Maze Monitor"
    
    try:
        params.app_window_params.window_geometry.full_screen_mode = hello_imgui.FullScreenMode.full_screen_desktop
    except AttributeError:
        params.app_window_params.window_geometry.size = (1400, 900)
    
    params.callbacks.show_gui = gui_frame
    immapp.run(params)

if __name__ == "__main__":
    main()