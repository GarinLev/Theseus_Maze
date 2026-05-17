import socket
import os
import imgui_bundle
from imgui_bundle import imgui, immapp, hello_imgui

# Настройки сетки лабиринта
GRID_SIZE = 16  

class CellState:
    def __init__(self):
        self.status = 'unexplored'  # 'unexplored', 'explored', 'start'
        self.walls = [False, False, False, False]  # [Вверх, Снизу, Слева, Справа]

class MonitorState:
    def __init__(self):
        self.lab = [[CellState() for _ in range(GRID_SIZE)] for _ in range(GRID_SIZE)]
        self.lab[8][8].status = 'start'
        self.robot_x, self.robot_y = 8, 8
        self.robot_dir = 'up'
        self.input_logs = ["Система полноэкранного мониторинга запущена."]
        self.found_victims = []  # Храним только текстовую информацию о буквах

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

        if cmd == 'pos':
            coords = parts[1].split(',')
            mon_state.robot_x = int(coords[0])
            mon_state.robot_y = int(coords[1])
            mon_state.robot_dir = parts[2]
            
            curr_cell = mon_state.lab[mon_state.robot_y][mon_state.robot_x]
            if curr_cell.status == 'unexplored':
                curr_cell.status = 'explored'
                
        elif cmd == 'wall':
            coords = parts[1].split(',')
            cx, cy = int(coords[0]), int(coords[1])
            wall_list = parts[2].split(',')
            
            if 0 <= cx < GRID_SIZE and 0 <= cy < GRID_SIZE:
                mon_state.lab[cy][cx].walls = [
                    'up' in wall_list,
                    'down' in wall_list,
                    'left' in wall_list,
                    'right' in wall_list
                ]

        elif cmd == 'victim':
            # Пришла метка: "victim:8,7:H:H.jpg"
            vic_coords = parts[1]
            vic_label = parts[2]
            
            # Сохраняем ТОЛЬКО текст, никакой графики!
            mon_state.found_victims.append({
                'pos': vic_coords, 
                'label': vic_label
            })
            print(f"[Map] Успешно зарегистрирована буква {vic_label} в клетке {vic_coords}")

    except BlockingIOError:
        pass

# =====================================================================
# ОСНОВНОЙ ЦИКЛ ГРАФИКИ
# =====================================================================
def gui_frame():
    check_network()
    
    display_size = imgui.get_io().display_size
    scr_w = display_size.x
    scr_h = display_size.y
    
    # -----------------------------------------------------------------
    # 1. ЛЕВАЯ ЧАСТЬ: КАРТА ЛАБИРИНТА
    # -----------------------------------------------------------------
    imgui.set_next_window_pos(imgui.ImVec2(10, 10))
    imgui.set_next_window_size(imgui.ImVec2(scr_w * 0.5 - 15, scr_h - 20))
    imgui.begin("Map", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
    
    draw_list = imgui.get_window_draw_list()
    screen_pos = imgui.get_cursor_screen_pos()
    
    map_available_w = (scr_w * 0.5 - 40)
    map_available_h = (scr_h - 60)
    cell_pixels = min(map_available_w, map_available_h) / GRID_SIZE
    
    color_unexplored = imgui.get_color_u32(imgui.ImVec4(0.08, 0.08, 0.1, 1.0))
    color_explored   = imgui.get_color_u32(imgui.ImVec4(0.18, 0.18, 0.22, 1.0))
    color_start      = imgui.get_color_u32(imgui.ImVec4(0.4, 0.15, 0.6, 1.0))
    color_robot      = imgui.get_color_u32(imgui.ImVec4(0.0, 0.45, 0.9, 1.0))
    color_wall       = imgui.get_color_u32(imgui.ImVec4(1.0, 0.4, 0.0, 1.0))
    color_grid_line  = imgui.get_color_u32(imgui.ImVec4(0.14, 0.14, 0.16, 1.0))
    
    for y in range(GRID_SIZE):
        for x in range(GRID_SIZE):
            x1 = screen_pos.x + x * cell_pixels
            y1 = screen_pos.y + y * cell_pixels
            x2 = x1 + cell_pixels
            y2 = y1 + cell_pixels
            
            cell = mon_state.lab[y][x]
            
            if cell.status == 'start':     bg_color = color_start
            elif cell.status == 'explored': bg_color = color_explored
            else:                           bg_color = color_unexplored
                
            draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), bg_color)
            draw_list.add_rect(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), color_grid_line, 0.0, 1.0)
            
            if cell.status != 'unexplored':
                w_top, w_bottom, w_left, w_right = cell.walls
                wall_thickness = 4.0
                
                if w_top:    draw_list.add_line(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y1), color_wall, wall_thickness)
                if w_bottom: draw_list.add_line(imgui.ImVec2(x1, y2), imgui.ImVec2(x2, y2), color_wall, wall_thickness)
                if w_left:   draw_list.add_line(imgui.ImVec2(x1, y1), imgui.ImVec2(x1, y2), color_wall, wall_thickness)
                if w_right:  draw_list.add_line(imgui.ImVec2(x2, y1), imgui.ImVec2(x2, y2), color_wall, wall_thickness)

    padding = cell_pixels * 0.15
    rx1 = screen_pos.x + mon_state.robot_x * cell_pixels + padding
    ry1 = screen_pos.y + mon_state.robot_y * cell_pixels + padding
    rx2 = rx1 + cell_pixels - (padding * 2)
    ry2 = ry1 + cell_pixels - (padding * 2)
    draw_list.add_rect_filled(imgui.ImVec2(rx1, ry1), imgui.ImVec2(rx2, ry2), color_robot, 6.0)
    
    arrows = {'up': "^", 'down': "v", 'left': "<", 'right': ">"}
    white_color = imgui.get_color_u32(imgui.ImVec4(1.0, 1.0, 1.0, 1.0))
    draw_list.add_text(imgui.ImVec2(rx1 + (cell_pixels*0.35), ry1 + (cell_pixels*0.15)), white_color, arrows[mon_state.robot_dir])

    imgui.end()

    # -----------------------------------------------------------------
    # 2. ПРАВАЯ ВЕРХНЯЯ ЧАСТЬ: ТОЛЬКО ТЕКСТОВЫЕ МЕТКИ
    # -----------------------------------------------------------------
    right_x = scr_w * 0.5 + 5
    right_w = scr_w * 0.5 - 15
    cam_h = scr_h * 0.4 - 10
    
    imgui.set_next_window_pos(imgui.ImVec2(right_x, 10))
    imgui.set_next_window_size(imgui.ImVec2(right_w, cam_h))
    imgui.begin("AI Victims Log", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
    
    if not mon_state.found_victims:
        imgui.text_disabled("No victims detected yet. Waiting for robot AI data...")
    else:
        for vic in reversed(mon_state.found_victims):
            imgui.spacing()
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.4, 1.0), f"[CRITICAL] Found Type '{vic['label']}' at cell [{vic['pos']}]")
            imgui.separator()
            
    imgui.end()

    # -----------------------------------------------------------------
    # 3. ПРАВАЯ НИЖНЯЯ ЧАСТЬ: ТЕРМИНАЛ UDP ЛОГОВ
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
        elif "victim" in log:
            imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.2, 1.0), log)
        else:
            imgui.text_colored(imgui.ImVec4(0.3, 0.7, 1.0, 1.0), log)
    imgui.end_child()
    
    imgui.end()

def main():
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