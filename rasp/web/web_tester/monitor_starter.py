import socket
import os
import imgui_bundle
from imgui_bundle import imgui, immapp, hello_imgui
import json
import time

LAB_SIZE = 33

class MonitorState:
    def __init__(self):
        # 0: unexplored, 1: explored, 2: wall, 3: start, 4: wall with victim
        self.lab = [[0 for _ in range(LAB_SIZE)] for _ in range(LAB_SIZE)]
        
        # Стартовая позиция робота (в чётных координатах)
        self.robot_x, self.robot_y = 16, 16
        self.lab[self.robot_y][self.robot_x] = 3  # Помечаем как старт
        
        # Направление: по умолчанию 0 градусов (вверх)
        self.corn = 0
        self.input_logs = ["System thin walls 33x33 started."]
        self.found_victims = []


# =====================================================================
# ПРОГРАММА 1: ПОДПРОГРАММЫ ОТРИСОВКИ ВСЕХ ОБЪЕКТОВ
# =====================================================================
class MazeRenderer:
    
    @staticmethod
    def draw_grid_and_walls(state, draw_list, xs, ys):
        """Подпрограмма отрисовки сетки и стенок"""
        color_unexplored = imgui.get_color_u32(imgui.ImVec4(0.08, 0.08, 0.1, 1.0)) 
        color_explored   = imgui.get_color_u32(imgui.ImVec4(0.22, 0.22, 0.28, 1.0)) 
        color_wall       = imgui.get_color_u32(imgui.ImVec4(1.0, 0.4, 0.0, 1.0))    
        color_victim_wall= imgui.get_color_u32(imgui.ImVec4(0.0, 1.0, 0.3, 1.0))    
        color_start      = imgui.get_color_u32(imgui.ImVec4(0.4, 0.15, 0.6, 1.0))   

        # Заливка фона (неисследованная область)
        draw_list.add_rect_filled(imgui.ImVec2(xs[0], ys[0]), imgui.ImVec2(xs[LAB_SIZE], ys[LAB_SIZE]), color_unexplored)

        for y in range(LAB_SIZE):
            for x in range(LAB_SIZE):
                x1, x2 = xs[x], xs[x + 1]
                y1, y2 = ys[y], ys[y + 1]
                
                val = state.lab[y][x]
                
                if x % 2 == 0 and y % 2 == 0:
                    bg_color = color_start if val == 3 else (color_explored if val == 1 else color_unexplored)
                    draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), bg_color)
                    draw_list.add_rect(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), imgui.get_color_u32(imgui.ImVec4(0.14, 0.14, 0.16, 0.5)), 0.0, 1.0)

                elif val == 2 or val == 4:
                    current_color = color_victim_wall if val == 4 else color_wall
                    draw_list.add_rect_filled(imgui.ImVec2(x1, y1), imgui.ImVec2(x2, y2), current_color)

    @staticmethod
    def draw_robot(state, draw_list, xs, ys):
        """Подпрограмма отрисовки робота и его направления"""
        color_robot = imgui.get_color_u32(imgui.ImVec4(0.0, 0.45, 0.9, 1.0))   
        rx1, rx2 = xs[state.robot_x], xs[state.robot_x + 1]
        ry1, ry2 = ys[state.robot_y], ys[state.robot_y + 1]
        robot_w = rx2 - rx1
        
        draw_list.add_rect_filled(imgui.ImVec2(rx1, ry1), imgui.ImVec2(rx2, ry2), color_robot, 2.0)

        arrows = {0: "^", 360: "^", 90: ">", 180: "v", 270: "<"}
        white_color = imgui.get_color_u32(imgui.ImVec4(1.0, 1.0, 1.0, 1.0))
        current_arrow = arrows.get(state.corn, "^")
        draw_list.add_text(imgui.ImVec2(rx1 + (robot_w * 0.35), ry1 + (robot_w * 0.2)), white_color, current_arrow)

    @staticmethod
    def draw_manual_controls(send_cmd_func, ctrl_y):
        """Подпрограмма отрисовки панели телеуправления"""
        imgui.set_cursor_pos(imgui.ImVec2(20, ctrl_y))
        imgui.text_colored(imgui.ImVec4(1.0, 0.8, 0.2, 1.0), "Manual Teleop (WASD): Type 'move' in Robot Console")

        btn_size = imgui.ImVec2(40, 40)
        
        imgui.set_cursor_pos(imgui.ImVec2(70, ctrl_y + 25))
        if imgui.button(" W ", btn_size) or imgui.is_key_pressed(imgui.Key.w): send_cmd_func('w')

        imgui.set_cursor_pos(imgui.ImVec2(25, ctrl_y + 70))
        if imgui.button(" A ", btn_size) or imgui.is_key_pressed(imgui.Key.a): send_cmd_func('a')
        
        imgui.same_line()
        if imgui.button(" S ", btn_size) or imgui.is_key_pressed(imgui.Key.s): send_cmd_func('s')
        
        imgui.same_line()
        if imgui.button(" D ", btn_size) or imgui.is_key_pressed(imgui.Key.d): send_cmd_func('d')

    @staticmethod
    def draw_logs(state, scr_w, scr_h):
        """Подпрограмма отрисовки логов ИИ и UDP-терминала"""
        right_x = scr_w * 0.5 + 5
        right_w = scr_w * 0.5 - 15
        cam_h = scr_h * 0.4 - 10
        
        # Окно логов жертв
        imgui.set_next_window_pos(imgui.ImVec2(right_x, 10))
        imgui.set_next_window_size(imgui.ImVec2(right_w, cam_h))
        imgui.begin("AI Victims Log", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
        
        if not state.found_victims:
            imgui.text_disabled("No victims detected yet. Waiting for robot AI data...")
        else:
            for vic in reversed(state.found_victims):
                imgui.spacing()
                imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.4, 1.0), f"[CRITICAL] Found Type '{vic['label']}' at cell [{vic['pos']}]")
                imgui.separator()
                
        imgui.end()

        # Окно сетевых пакетов
        log_y = cam_h + 20
        log_h = scr_h - log_y - 10
        
        imgui.set_next_window_pos(imgui.ImVec2(right_x, log_y))
        imgui.set_next_window_size(imgui.ImVec2(right_w, log_h))
        imgui.begin("UDP Packets Terminal", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
        
        imgui.begin_child("ScrollZone", imgui.ImVec2(0, 0), True, imgui.WindowFlags_.horizontal_scrollbar)
        for log in reversed(state.input_logs):
            if "wall" in log:     imgui.text_colored(imgui.ImVec4(1.0, 0.6, 0.2, 1.0), log)
            elif "victim" in log or "vl" in log or "vr" in log: imgui.text_colored(imgui.ImVec4(0.2, 1.0, 0.2, 1.0), log)
            elif "GUI" in log:    imgui.text_colored(imgui.ImVec4(0.9, 0.4, 0.8, 1.0), log)
            else:                 imgui.text_colored(imgui.ImVec4(0.3, 0.7, 1.0, 1.0), log)
        imgui.end_child()
        imgui.end()

    @classmethod
    def render_full_frame(cls, state, send_cmd_func):
        """Главный метод рендера, распределяющий работу по подпрограммам"""
        display_size = imgui.get_io().display_size
        scr_w, scr_h = display_size.x, display_size.y
        
        # Левая часть экрана (карта)
        imgui.set_next_window_pos(imgui.ImVec2(10, 10))
        imgui.set_next_window_size(imgui.ImVec2(scr_w * 0.5 - 15, scr_h - 20))
        imgui.begin("Maze Monitor", None, imgui.WindowFlags_.no_resize | imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse)
        
        draw_list = imgui.get_window_draw_list()
        screen_pos = imgui.get_cursor_screen_pos()
        
        map_available_w = (scr_w * 0.5 - 40)
        map_available_h = (scr_h - 160)
        cell_pixels = min(map_available_w, map_available_h) / LAB_SIZE
        
        cell_thin = cell_pixels * 0.3  
        cell_thick = cell_pixels * 1.7 

        # Вычисление координат сетки
        xs, ys = [screen_pos.x], [screen_pos.y]
        for i in range(LAB_SIZE):
            size = cell_thick if i % 2 == 0 else cell_thin
            xs.append(xs[-1] + size)
            ys.append(ys[-1] + size)

        # Вызовы подпрограмм отрисовки
        cls.draw_grid_and_walls(state, draw_list, xs, ys)
        cls.draw_robot(state, draw_list, xs, ys)
        
        ctrl_y = ys[-1] + 15
        cls.draw_manual_controls(send_cmd_func, ctrl_y)

        imgui.end()

        # Правая часть экрана (логи)
        cls.draw_logs(state, scr_w, scr_h)


# =====================================================================
# ПРОГРАММА 2: ПРИНЯТИЕ ЗНАЧЕНИЙ И УПРАВЛЕНИЕ СЕТЬЮ
# =====================================================================
class NetworkController:
    def __init__(self, state: MonitorState):
        self.state = state
        
        # Настройка сети UDP (ВХОДЯЩИЕ пакеты ОТ робота)
        self.sock_in = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock_in.bind(('127.0.0.1', 5002))
        self.sock_in.setblocking(False)

        # Настройка сети UDP (ИСХОДЯЩИЕ команды К роботу)
        self.sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send_robot_cmd(self, cmd):
        """Отправляет команду ручного управления роботу"""
        try:
            self.sock_out.sendto(f"cmd:{cmd}".encode('utf-8'), ('127.0.0.1', 5003))
            self.state.input_logs.append(f"[GUI -> Robot]: Action '{cmd.upper()}'")
            if len(self.state.input_logs) > 40:
                self.state.input_logs.pop(0)
        except Exception:
            pass

    def check_network_and_update(self):
        new_data_received = False

        try:
            data, addr = self.sock_in.recvfrom(1024)
            text = data.decode('utf-8')
            parts = text.split(':')
            cmd = parts[0]

            self.state.input_logs.append(f"[UDP]: {text}")
            if len(self.state.input_logs) > 40:
                self.state.input_logs.pop(0)

            vx, vy = self.state.robot_x, self.state.robot_y

            if cmd == 'pos':
                new_data_received = True 
                coords = parts[1].split(',')
                self.state.robot_x = int(coords[0]) * 2
                self.state.robot_y = int(coords[1]) * 2
                self.state.corn = int(parts[2])
                    
            elif cmd == 'wall':
                new_data_received = True
                coords = parts[1].split(',')
                cx, cy = int(coords[0]) * 2, int(coords[1]) * 2
                wall_list = parts[2].split(',')
                
                if 0 <= cx < LAB_SIZE and 0 <= cy < LAB_SIZE:
                    if 'up' in wall_list and cy - 1 >= 0:
                        if self.state.lab[cy - 1][cx] != 4: self.state.lab[cy - 1][cx] = 2
                    if 'down' in wall_list and cy + 1 < LAB_SIZE:
                        if self.state.lab[cy + 1][cx] != 4: self.state.lab[cy + 1][cx] = 2
                    if 'left' in wall_list and cx - 1 >= 0:
                        if self.state.lab[cy][cx - 1] != 4: self.state.lab[cy][cx - 1] = 2
                    if 'right' in wall_list and cx + 1 < LAB_SIZE:
                        if self.state.lab[cy][cx + 1] != 4: self.state.lab[cy][cx + 1] = 2

            elif cmd == 'victim':
                new_data_received = True 
                vic_label = parts[2]
                self.state.found_victims.append({'pos': f"{vx//2},{vy//2}", 'label': vic_label})
                
                if (self.state.corn == 0 or self.state.corn == 360) and vy - 1 >= 0:    self.state.lab[vy - 1][vx] = 4
                elif self.state.corn == 180 and vy + 1 < LAB_SIZE:                      self.state.lab[vy + 1][vx] = 4
                elif self.state.corn == 270 and vx - 1 >= 0:                            self.state.lab[vy][vx - 1] = 4
                elif self.state.corn == 90 and vx + 1 < LAB_SIZE:                       self.state.lab[vy][vx + 1] = 4
                print(f"[Map] Метка {vic_label} выставлена перед роботом.")

            elif cmd == 'vl':
                new_data_received = True 
                if self.state.corn == 0 or self.state.corn == 360:
                    if vx - 1 >= 0: self.state.lab[vy][vx - 1] = 4
                elif self.state.corn == 180:
                    if vx + 1 < LAB_SIZE: self.state.lab[vy][vx + 1] = 4
                elif self.state.corn == 270:
                    if vy + 1 < LAB_SIZE: self.state.lab[vy + 1][vx] = 4
                elif self.state.corn == 90:
                    if vy - 1 >= 0: self.state.lab[vy - 1][vx] = 4

            elif cmd == 'vr':
                new_data_received = True 
                if self.state.corn == 0 or self.state.corn == 360:
                    if vx + 1 < LAB_SIZE: self.state.lab[vy][vx + 1] = 4
                elif self.state.corn == 180:
                    if vx - 1 >= 0: self.state.lab[vy][vx - 1] = 4
                elif self.state.corn == 270:
                    if vy - 1 >= 0: self.state.lab[vy - 1][vx] = 4
                elif self.state.corn == 90:
                    if vy + 1 < LAB_SIZE: self.state.lab[vy + 1][vx] = 4

        except BlockingIOError: pass
        except OSError as e:
            if e.errno == 10035 or e.winerror == 10035: pass
            else: print(f"[GUI Сеть] Ошибка сокета: {e}")
        except Exception as e: print(f"[GUI Ошибка] Критический сбой сети: {e}")

        if new_data_received:
            try:
                data_to_save = {
                    "matrix": self.state.lab,
                    "robot_x": self.state.robot_x,  
                    "robot_y": self.state.robot_y,  
                    "corn": int(self.state.corn),   
                }
                with open("maze_shared.json", "w") as f:
                    json.dump(data_to_save, f)
                print(f"[DEBUG] Файл maze_shared.json успешно обновлен.") 
            except Exception as e:
                print(f"[GUI Error] Не удалось записать файл: {e}")

            if self.state.lab[self.state.robot_y][self.state.robot_x] == 0: 
                    self.state.lab[self.state.robot_y][self.state.robot_x] = 1

    def main_loop_tick(self):
        """Метод, объединяющий логику и вызов рендера (вызывается на каждый кадр ImGui)"""
        # Сначала принимаем значения (Программа 2)
        self.check_network_and_update()
        
        # Затем вызываем подпрограммы отрисовки (Программа 1)
        MazeRenderer.render_full_frame(self.state, self.send_robot_cmd)


# =====================================================================
# ЗАПУСК ПРИЛОЖЕНИЯ
# =====================================================================
def main():
    # Создаем общее состояние
    shared_state = MonitorState()
    
    # Создаем контроллер, который будет управлять сетью и графикой
    controller = NetworkController(shared_state)
    
    params = hello_imgui.RunnerParams()
    params.app_window_params.window_title = "Rescue Maze Monitor 33x33 Thin Walls"
    
    try:
        params.app_window_params.window_geometry.full_screen_mode = hello_imgui.FullScreenMode.full_screen_desktop
    except AttributeError:
        params.app_window_params.window_geometry.size = (1400, 900)
    
    # ImGui вызывает метод controller.main_loop_tick 60 раз в секунду
    params.callbacks.show_gui = controller.main_loop_tick
    immapp.run(params)

if __name__ == "__main__":
    time.sleep(1.5) 
    main()