import socket

TARGET_IP = "127.0.0.1"
TARGET_PORT = 5002
sender_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_msg(text):
    sender_sock.sendto(text.encode('utf-8'), (TARGET_IP, TARGET_PORT))

def get_global_walls(robot_dir, wall_front, wall_right, wall_back, wall_left):
    """
    Пересчитывает локальные датчики робота в глобальные стены карты (up/down/left/right)
    в зависимости от того, куда сейчас развернут робот.
    """
    global_walls = []
    
    if robot_dir == 'up':
        if wall_front: global_walls.append('up')
        if wall_right: global_walls.append('right')
        if wall_back:  global_walls.append('down')
        if wall_left:  global_walls.append('left')
        
    elif robot_dir == 'down':
        if wall_front: global_walls.append('down')
        if wall_right: global_walls.append('left')
        if wall_back:  global_walls.append('up')
        if wall_left:  global_walls.append('right')
        
    elif robot_dir == 'left':
        if wall_front: global_walls.append('left')
        if wall_right: global_walls.append('up')
        if wall_back:  global_walls.append('right')
        if wall_left:  global_walls.append('down')
        
    elif robot_dir == 'right':
        if wall_front: global_walls.append('right')
        if wall_right: global_walls.append('down')
        if wall_back:  global_walls.append('left')
        if wall_left:  global_walls.append('up')
        
    return ",".join(global_walls)

def main():
    rx, ry = 8, 8  # Старт в центре сетки 16х16
    r_dir = "up"   # Изначально смотрим вверх (Север)
    
    test_photos = ['test_h.jpg', 'test_s.jpg', 'test_red.jpg']
    photo_idx = 0
    
    print("=" * 60)
    print("      РОБОТ: СИМУЛЯЦИЯ СТРОКИ ДАТЧИКОВ 4-BIT")
    print("=" * 60)
    print("Управление:")
    print("  w / s — шаг вперед / назад")
    print("  a / d — поворот влево / вправо на 90°")
    print("  v     — камера (выявить метку ИИ)")
    print("\nФормат ввода датчиков: 4 цифры подряд [Спереди][Справа][Сзади][Слева]")
    print("Пример: 1101 -> Спереди: Да, Справа: Да, Сзади: Нет, Слева: Да\n")
    
    # Отправляем стартовую позицию на экран
    send_msg(f"pos:{rx},{ry}:{r_dir}")
    
    turn_left_rules = {'up': 'left', 'left': 'down', 'down': 'right', 'right': 'up'}
    turn_right_rules = {'up': 'right', 'right': 'down', 'down': 'left', 'left': 'up'}

    while True:
        print(f"\n[Робот на [{rx}, {ry}], смотрит: {r_dir.upper()}]")
        cmd = input("Движение (w/a/s/d/v) или 'exit': ").strip().lower()
        
        if cmd == 'exit': 
            break
            
        # ==============================================================
        # 1. ОБРАБОТКА КОМАНД ПЕРЕМЕЩЕНИЯ (WASD)
        # ==============================================================
        if cmd == 'w':
            if r_dir == 'up':    ry -= 1
            elif r_dir == 'down':  ry += 1
            elif r_dir == 'left':  rx -= 1
            elif r_dir == 'right': rx += 1
            print("Робот проехал вперед.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 's':
            if r_dir == 'up':    ry += 1
            elif r_dir == 'down':  ry -= 1
            elif r_dir == 'left':  rx += 1
            elif r_dir == 'right': rx -= 1
            print("Робот сдал назад.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 'a':
            r_dir = turn_left_rules[r_dir]
            print(f"Поворот влево ↺. Теперь смотрим: {r_dir.upper()}")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            continue  # При чистом повороте датчики не опрашиваем, ждем шага
            
        elif cmd == 'd':
            r_dir = turn_right_rules[r_dir]
            print(f"Поворот вправо ↻. Теперь смотрим: {r_dir.upper()}")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            continue  # При чистом повороте датчики не опрашиваем, ждем шага
            
        elif cmd == 'vr' or cmd == "vl":
            label = input("Метка с камеры (F/nedoF/OM): ").strip().upper() or "F"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            print(f"Данные камеры отправлены.")
            continue
        else:
            print("Ошибка: Неверная команда!")
            continue

        # Проверка выхода за границы сетки 16х16
        if not (0 <= rx < 16 and 0 <= ry < 16):
            print("🛑 Авария: вылетел за карту! Сброс на базу [8,8]")
            rx, ry = 8, 8
            send_msg(f"pos:{rx},{ry}:{r_dir}")

        # ==============================================================
        # 2. ОПРОС ДАТЧИКОВ СТРОКОЙ ИЗ 4 СИМВОЛОВ
        # ==============================================================
        sensors_str = input("Введи показания датчиков (например, 1101): ").strip()
        
        # Если ввели кривую строку или просто нажали Enter, забиваем нулями (нет стен)
        if len(sensors_str) != 4 or not sensors_str.isdigit():
            print("Предупреждение: неверный формат строки датчиков. Считаем, что стен нет.")
            sensors_str = "0000"
            
        # Разбираем строку посимвольно в булевы переменные (True/False)
        sf = sensors_str[0] == '1' # Спереди
        sr = sensors_str[1] == '1' # Справа
        sb = sensors_str[2] == '1' # Сзади
        sl = sensors_str[3] == '1' # Слева
        
        # Пересчитываем в глобальную систему координат лабиринта
        calculated_walls = get_global_walls(r_dir, sf, sr, sb, sl)
        
        # Отправляем пачку оранжевых стен на экран монитора
        send_msg(f"wall:{rx},{ry}:{calculated_walls}")
        print(f"[UDP Отправлено] Глобальные стены клетки: {calculated_walls if calculated_walls else 'Свободно'}")

if __name__ == "__main__":
    main()