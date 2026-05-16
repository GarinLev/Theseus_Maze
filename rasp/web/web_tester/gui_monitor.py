import socket
import os
import imgui_bundle
from imgui_bundle import imgui, immapp, hello_imgui

# Настройки сетки (как на видео)
GRID_SIZE = 16  # Делаем карту, например, 16х16 больших клеток
CELL_PIXELS = 32.0  # Размер одной клетки на экране в пикселях

class CellState:
    def __init__(self):
        self.status = 'unexplored'  # может быть 'unexplored', 'explored', 'start'
        # Стены вокруг этой клетки: [Вверх, Снизу, Слева, Справа]
        self.walls = [False, False, False, False]

class MonitorState:
    def __init__(self):
        # Создаем карту из объектов клеток
        self.lab = [[CellState() for _ in range(GRID_SIZE)] for _ in range(GRID_SIZE)]
        
        # Ставим старт ровно по центру
        self.lab[8][8].status = 'start'
        
        # Координаты робота и его направление
        self.robot_x, self.robot_y = 8, 8
        self.robot_dir = 'up'
        
        # Списки для логов инпута и найденных меток
        self.input_logs = ["Система запущена. Ожидание команд от main_robot.py..."]
        self.found_victims = []

mon_state = MonitorState()
IMAGE_FOLDER = "captured_images"
os.makedirs(IMAGE_FOLDER, exist_ok=True)

# Настройка сети
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', 5002))
sock.setblocking(False)

# =====================================================================
# РАЗБОР СЕТЕВЫХ КОМАНД ИЗ ИНПУТА
# =====================================================================
def check_network():
    try:
        data, addr = sock.recvfrom(1024)
        text = data.decode('utf-8')
        parts = text.split(':')
        cmd = parts[0]

        # Добавляем всё, что пришло в инпут, в нижний лог
        mon_state.input_logs.append(f"[Ввод]: {text}")
        if len(mon_state.input_logs) > 30:
            mon_state.input_logs.pop(0)

        if cmd == 'pos':
            # Пришли координаты: "pos:8,7:up"
            coords = parts[1].split(',')
            mon_state.robot_x = int(coords[0])
            mon_state.robot_y = int(coords[1])
            mon_state.robot_dir = parts[2]
            
            # Помечаем клетку как исследованную
            curr_cell = mon_state.lab[mon_state.robot_y][mon_state.robot_x]
            if curr_cell.status == 'unexplored':
                curr_cell.status = 'explored'
                
        elif cmd == 'wall':
            # Пришла стена: "wall:8,7:up,down"
            coords = parts[1].split(',')
            cx, cy = int(coords[0]), int(coords[1])
            wall_list = parts[2].split(',') # список стен через запятую
            
            if 0 <= cx < GRID_SIZE and 0 <= cy < GRID_SIZE:
                # Сбрасываем старые стены для этой ячейки и ставим новые
                mon_state.lab[cy][cx].walls = [
                    'up' in wall_list,
                    'down' in wall_list,
                    'left' in wall_list,
                    'right' in wall_list
                ]

        elif cmd == 'victim':
            # Пришла метка: "victim:8,7:H:test_h.jpg"
            vic_coords = parts[1]
            vic_label = parts[2]
            vic_filename = parts[3]
            
            full_path = os.path.join(IMAGE_FOLDER, vic_filename)
            if os.path.exists(full_path):
                texture_id = hello_imgui.load_texture_vulkan_or_opengl(full_path)
                if texture_id:
                    mon_state.found_victims.append({
                        'pos': vic_coords, 'label': vic_label, 'texture': texture_id
                    })

    except BlockingIOError:
        pass

# =====================================================================
# РЕНДЕР ИНТЕРФЕЙСА (IMGUI)
# =====================================================================
def gui_frame():
    check_network()
    
    # -----------------------------------------------------------------
    # 1. ЛЕВАЯ ЧАСТЬ ОКНА: КАРТА ЛАБИРИНТА
    # -----------------------------------------------------------------
    imgui.set_next_window_pos(imgui.ImVec2(10, 10))
    imgui.set_next_window_size(imgui.ImVec2(540, 540))
    imgui.begin("Лабиринт (В стиле РобоКап)", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move)
    
    # Получаем инструменты ImGui для ручного рисования линий и квадратов
    draw_list = imgui.get_window_draw_list()
    # Точка, откуда начинается само окно на экране
    screen_pos = imgui.get_cursor_screen_pos()
    
    # Конвертируем цвета в формат ImGui ImColor
    color_unexplored = imgui.get_color_u32_rgba(0.12, 0.12, 0.14, 1.0)
    color_explored = imgui.get_color_u32_rgba(0.22, 0.22, 0.25, 1.0)
    color_start = imgui.get_color_u32_rgba(0.4, 0.15, 0.6, 1.0)
    color_robot = imgui.get_color_u32_rgba(0.0, 0.45, 0.9, 1.0)
    color_wall = imgui.get_color_u32_rgba(1.0, 0.4, 0.0, 1.0)      # Оранжевая стена
    color_grid_line = imgui.get_color_u32_rgba(0.16, 0.16, 0.18, 1.0) # Сетка
    
    # Перебираем все клетки лабиринта
    for y in range(GRID_SIZE):
        for x in range(GRID_SIZE):
            # Вычисляем экранные пиксели для текущей клетки
            x1 = screen_pos.x + x * CELL_PIXELS
            y1 = screen_pos.y + y * CELL_PIXELS
            x2 = x1 + CELL_PIXELS
            y2 = y1 + CELL_PIXELS
            
            cell = mon_state.lab[y][x]
            
            # Определяем цвет фона клетки
            if cell.status == 'start':
                bg_color = color_start
            elif cell.status == 'explored':
                bg_color = color_explored
            else:
                bg_color = color_unexplored
                
            # Рисуем подложку ячейки
            draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), bg_color)
            # Рисуем тонкую серую сетку вокруг ячейки
            draw_list.add_rect(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), color_grid_line, 0.0, 0, 1.0)
            
            # Если клетка открыта — рисуем её стены поверх стыков
            if cell.status != 'unexplored':
                w_top, w_bottom, w_left, w_right = cell.walls
                wall_thickness = 3.5  # Толщина оранжевой линии
                
                if w_top:    draw_list.add_line(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y1), color_wall, wall_thickness)
                if w_bottom: draw_list.add_line(imgui.ImVec2(x1, y2), imgui.ImVec2(x2, y2), color_wall, wall_thickness)
                if w_left:   draw_list.add_line(imgui.ImVec2(x1, y1), imgui.ImVec2(x1, y2), color_wall, wall_thickness)
                if w_right:  draw_list.add_line(imgui.ImVec2(x2, y1), imgui.ImVec2(x2, y2), color_wall, wall_thickness)

    # Рисуем синий квадрат робота на его текущей позиции
    rx1 = screen_pos.x + mon_state.robot_x * CELL_PIXELS + 4
    ry1 = screen_pos.y + mon_state.robot_y * CELL_PIXELS + 4
    rx2 = rx1 + CELL_PIXELS - 8
    ry2 = ry1 + CELL_PIXELS - 8
    draw_list.add_rect_filled(imgui.ImVec2(rx1, ry1), imgui.ImVec2(rx2, ry2), color_robot, 4.0)
    
    # Текстовый маркер направления робота (куда он смотрит)
    arrows = {'up': "^", 'down': "v", 'left': "<", 'right': ">"}
    draw_list.add_text(imgui.ImVec2(rx1 + 8, ry1 + 4), imgui.get_color_u32_rgba(1,1,1,1), arrows[mon_state.robot_dir])

    imgui.end()

    # -----------------------------------------------------------------
    # 2. ПРАВАЯ ЧАСТЬ ОКНА: КАРТОЧКИ И ФОТОГРАФИИ С КАМЕРЫ
    # -----------------------------------------------------------------
    imgui.set_next_window_pos(imgui.ImVec2(560, 10))
    imgui.set_next_window_size(imgui.ImVec2(370, 740))
    imgui.begin("Камера / Метки ИИ", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move)
    
    if not mon_state.found_victims:
        imgui.text_disabled("Камера робота сканирует лабиринт...")
    else:
        for vic in reversed(mon_state.found_victims):
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.4, 1.0), f"[!] Обнаружена метка '{vic['label']}'")
            imgui.same_line()
            imgui.text_disabled(f" в клетке {vic['pos']}")
            
            # Выводим нативную OpenGL картинку метки
            imgui.image(vic['texture'], imgui.ImVec2(340, 220))
            imgui.separator()
            imgui.spacing()
            
    imgui.end()

    # -----------------------------------------------------------------
    # 3. НИЖНЯЯ ЧАСТЬ ОКНА: СЫРОЙ ЛОГ ТОГО, ЧТО ПРИХОДИТ В INPUT
    # -----------------------------------------------------------------
    imgui.set_next_window_pos(imgui.ImVec2(10, 560))
    imgui.set_next_window_size(imgui.ImVec2(540, 190))
    imgui.begin("Терминал Ввода (Входящие UDP пакеты)", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move)
    
    imgui.begin_child("LogScrollZone", imgui.ImVec2(0, 0), True, imgui.WindowFlags_.horizontal_scrollbar)
    for log in reversed(mon_state.input_logs):
        # Подсвечиваем разные типы пакетов для наглядности в полете
        if "wall" in log:
            imgui.text_colored(imgui.ImVec4(1.0, 0.6, 0.2, 1.0), log)
        elif "victim" in log:
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.2, 1.0), log)
        else:
            imgui.text_colored(imgui.ImVec4(0.3, 0.7, 1.0, 1.0), log)
    imgui.end_child()
    
    imgui.end()

def main():
    params = hello_imgui.RunnerParams()
    params.app_window_params.window_title = "Advanced Rescue Maze Monitor"
    params.app_window_params.window_geometry.size = (940, 760)
    params.callbacks.show_gui = gui_frame
    immapp.run(params)

if __name__ == "__main__":
    main()