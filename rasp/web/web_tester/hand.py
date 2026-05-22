import socket
# from monitor_starter import stat_bfs
import subprocess
import time
import sys
import os
import json


# Создаем глобальную переменную внутри hand.py, чтобы она изначально существовала
stat_bfs = 0 

def load_data_from_monitor():
    global stat_bfs  # Разрешаем функции изменять глобальную переменную stat_bfs
    filename = "maze_shared.json"
    
    if os.path.exists(filename):
        try:
            with open(filename, "r") as f:
                data = json.load(f)
                
                # Проверяем, что в файле лежит словарь, а не просто чистый массив
                if isinstance(data, dict):
                    stat_bfs = data.get("stat_bfs", 0)  # Забираем значение переменной
                    return data.get("matrix")           # Возвращаем саму матрицу
                else:
                    return data  # Если там лежал только массив (старая версия файла)
        except Exception as e:
            print(f"Ошибка чтения файла моста: {e}")
            
    return [[0 for _ in range(33)] for _ in range(33)]

current_dir = os.path.dirname(os.path.abspath(__file__))
bfs_path = os.path.join(current_dir, "bfs.py")
TARGET_IP = "127.0.0.1"
TARGET_PORT = 5002
sender_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_msg(text):
    sender_sock.sendto(text.encode('utf-8'), (TARGET_IP, TARGET_PORT))

def get_global_walls(corn, wall_front, wall_right, wall_back, wall_left):
    """
    Пересчитывает локальные датчики робота в глобальные стены карты (up/down/left/right)
    в зависимости от текущего угла взгляда corn (0, 90, 180, 270).
    """
    global_walls = []
    
    if corn == 0 or corn == 360:
        if wall_front: global_walls.append('up')
        if wall_right: global_walls.append('right')
        if wall_back:  global_walls.append('down')
        if wall_left:  global_walls.append('left')
        
    elif corn == 180:
        if wall_front: global_walls.append('down')
        if wall_right: global_walls.append('left')
        if wall_back:  global_walls.append('up')
        if wall_left:  global_walls.append('right')
        
    elif corn == 270:
        if wall_front: global_walls.append('left')
        if wall_right: global_walls.append('up')
        if wall_back:  global_walls.append('right')
        if wall_left:  global_walls.append('down')
        
    elif corn == 90:
        if wall_front: global_walls.append('right')
        if wall_right: global_walls.append('down')
        if wall_back:  global_walls.append('left')
        if wall_left:  global_walls.append('up')
        
    return ",".join(global_walls)

def main():
    rx, ry = 8, 8  # Старт в центре сетки 16х16
    corn = 0       # Изначально смотрим вверх (0 градусов)
    
    test_photos = ['test_h.jpg', 'test_s.jpg', 'test_red.jpg']
    photo_idx = 0
    
    
    # Отправляем стартовую позицию на экран с углом corn
    send_msg(f"pos:{rx},{ry}:{corn}")
    
    # Словарик для красивого вывода направления в консоль
    dir_names = {0: "UP", 360: "UP", 90: "RIGHT", 180: "DOWN", 270: "LEFT"}

    while True:
        current_dir_name = dir_names.get(corn, f"{corn}°")
        print(f"\n[Робот на [{rx}, {ry}], смотрит: {current_dir_name} ({corn}°)]")
        
        cmd = input("walls (или спецкоманда): ").strip().lower()
        
        if cmd == 'end': 
            break
        
        elif cmd == 'move':
            print("\n>>> Write movement please")
            
            # Открываем сокет, чтобы слушать команды от монитора
            move_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            move_sock.bind(('127.0.0.1', 5003))
            move_sock.settimeout(0.5) # Тайм-аут, чтобы программа могла поймать Ctrl+C
            
            try:
                while True:
                    try:
                        data, _ = move_sock.recvfrom(1024)
                        rcv = data.decode('utf-8')
                        
                        if rcv.startswith("cmd:"):
                            action = rcv.split(":")[1]
                            
                            if action == 'w':
                                if corn == 0 or corn == 360: ry -= 1
                                elif corn == 180: ry += 1
                                elif corn == 270: rx -= 1
                                elif corn == 90: rx += 1
                            elif action == 's':
                                if corn == 0 or corn == 360: ry += 1
                                elif corn == 180: ry -= 1
                                elif corn == 270: rx += 1
                                elif corn == 90: rx -= 1
                                corn = (corn - 180) % 360
                            elif action == 'a':
                                print("Left")
                                corn = (corn - 90) % 360
                                if corn == 0 or corn == 360:  ry -= 1
                                elif corn == 180:             ry += 1
                                elif corn == 270:             rx -= 1
                                elif corn == 90:              rx += 1
                                send_msg(f"pos:{rx},{ry}:{corn}")
                            elif action == 'd':
                                print("Right")
                                corn = (corn + 90) % 360
                                if corn == 0 or corn == 360:  ry -= 1
                                elif corn == 180:             ry += 1
                                elif corn == 270:             rx -= 1
                                elif corn == 90:              rx += 1
                                send_msg(f"pos:{rx},{ry}:{corn}")
                            print(f"[ПУЛЬТ]: Команда {action.upper()} | Координаты: [{rx}, {ry}], Угол: {corn}°")
                            send_msg(f"pos:{rx},{ry}:{corn}") # Отправляем новую позицию на экран
                            
                    except socket.timeout:
                        pass # Ничего не пришло за 0.5 сек, просто ждем дальше
                        
            except KeyboardInterrupt:
                print("\n<<< ВЫХОД ИЗ РЕЖИМА ПУЛЬТА. Возврат к ручному вводу стен.")
                move_sock.close()
                continue

        elif cmd == 'vl' or cmd == 'vr':
            send_msg(f"{cmd}:")
            print(f"Команда {cmd.upper()} отправлена в монитор.")
            continue

        elif cmd == 'v':
            label = input("(F/nedoF/OM)").strip().upper() or "F"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            print(f"Данные автоматической метки отправлены.")
            continue

        if len(cmd) != 4 or not cmd.isdigit():
            print("What!?")
            continue
        
        n_walls = 0
        if cmd[0] == "1": n_walls += 1
        if cmd[1] == "1": n_walls += 1
        if cmd[2] == "1": n_walls += 1
        if cmd[3] == "1": n_walls += 1
        sf = cmd[0] == '1'
        sr = cmd[1] == '1'
        sb = cmd[2] == '1'
        sl = cmd[3] == '1'


        calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
        send_msg(f"wall:{rx},{ry}:{calculated_walls}")
        print(f"[UDP Arduino -> Rasp] Walls: {calculated_walls if calculated_walls else 'Свободно'}")

        if n_walls == 3 or stat_bfs == 1:
            subprocess.run(["python", bfs_path, str(corn)])
        
        else:
            if not sr:
                print("Right")
                corn = (corn + 90) % 360
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")
            
            elif not sf:
                print("Straight")
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")

            elif not sl:
                print("Left")
                corn = (corn - 90) % 360
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")

        if not (0 <= rx < 16 and 0 <= ry < 16):
            print("🛑 Авария: вылетел за карту! Сброс на базу [8,8]")
            rx, ry = 8, 8
            send_msg(f"pos:{rx},{ry}:{corn}")

if __name__ == "__main__":
    main()