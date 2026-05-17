import socket

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
    
    if corn == 0 or corn == 360:  # Робот смотрит ВВЕРХ (Север)
        if wall_front: global_walls.append('up')
        if wall_right: global_walls.append('right')
        if wall_back:  global_walls.append('down')
        if wall_left:  global_walls.append('left')
        
    elif corn == 180:  # Робот смотрит ВНИЗ (Юг)
        if wall_front: global_walls.append('down')
        if wall_right: global_walls.append('left')
        if wall_back:  global_walls.append('up')
        if wall_left:  global_walls.append('right')
        
    elif corn == 270:  # Робот смотрит ВЛЕВО (Запад)
        if wall_front: global_walls.append('left')
        if wall_right: global_walls.append('up')
        if wall_back:  global_walls.append('right')
        if wall_left:  global_walls.append('down')
        
    elif corn == 90:  # Робот смотрит ВПРАВО (Восток)
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
    
    print("=" * 60)
    print("      РОБОТ: СИМУЛЯЦИЯ СТРОКИ ДАТЧИКОВ 4-BIT")
    print("=" * 60)
    print("Управление:")
    print("  w / s   — шаг вперед / назад")
    print("  a / d   — поворот влево / вправо на 90°")
    print("  vl / vr — ручная установка зелёной стены слева/справа")
    print("  v       — камера (выявить автоматическую метку ИИ)")
    print("\nФормат ввода датчиков: 4 цифры подряд [Спереди][Справа][Сзади][Слева]")
    print("Пример: 1101 -> Спереди: Да, Справа: Да, Сзади: Нет, Слева: Да\n")
    
    # Отправляем стартовую позицию на экран с углом corn
    send_msg(f"pos:{rx},{ry}:{corn}")
    
    # Словарик для красивого вывода направления в консоль
    dir_names = {0: "UP", 360: "UP", 90: "RIGHT", 180: "DOWN", 270: "LEFT"}

    while True:
        current_dir_name = dir_names.get(corn, f"{corn}°")
        print(f"\n[Робот на [{rx}, {ry}], смотрит: {current_dir_name} ({corn}°)]")
        cmd = input("Движение (w/a/s/d/vl/vr/v) или 'exit': ").strip().lower()
        
        if cmd == 'exit': 
            break
            
        # ==============================================================
        # 1. ОБРАБОТКА КОМАНД ПЕРЕМЕЩЕНИЯ И ПОВОРОТОВ
        # ==============================================================
        if cmd == 'w':
            if corn == 0 or corn == 360:  ry -= 1
            elif corn == 180:             ry += 1
            elif corn == 270:             rx -= 1
            elif corn == 90:              rx += 1
            print("Робот проехал вперед.")
            send_msg(f"pos:{rx},{ry}:{corn}")
            
        elif cmd == 's':
            if corn == 0 or corn == 360:  ry += 1
            elif corn == 180:             ry -= 1
            elif corn == 270:             rx += 1
            elif corn == 90:              rx -= 1
            print("Робот сдал назад.")
            send_msg(f"pos:{rx},{ry}:{corn}")
            
        elif cmd == 'a':
            corn = (corn - 90) % 360
            print(f"Поворот влево ↺. Теперь угол: {corn}° ({dir_names.get(corn)})")
            send_msg(f"pos:{rx},{ry}:{corn}")
            continue  # Ждем следующей команды
            
        elif cmd == 'd':
            corn = (corn + 90) % 360
            print(f"Поворот вправо ↻. Теперь угол: {corn}° ({dir_names.get(corn)})")
            send_msg(f"pos:{rx},{ry}:{corn}")
            continue  # Ждем следующей команды
            
        elif cmd == 'vl' or cmd == 'vr':
            # Отправляем команду ручной стены в монитор
            send_msg(f"{cmd}:")
            print(f"Команда {cmd.upper()} отправлена в монитор.")
            continue

        elif cmd == 'v':
            label = input("Метка с камеры (F/nedoF/OM): ").strip().upper() or "F"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            print(f"Данные автоматической метки отправлены.")
            continue
        else:
            print("Ошибка: Неверная команда!")
            continue

        # Проверка выхода за границы сетки 16х16
        if not (0 <= rx < 16 and 0 <= ry < 16):
            print("🛑 Авария: вылетел за карту! Сброс на базу [8,8]")
            rx, ry = 8, 8
            send_msg(f"pos:{rx},{ry}:{corn}")

        # ==============================================================
        # 2. ОПРОС ДАТЧИКОВ СТРОКОЙ ИЗ 4 СИМВОЛОВ
        # ==============================================================
        sensors_str = input("Стены (или Enter если пусто): ").strip()
        
        if not sensors_str or len(sensors_str) != 4 or not sensors_str.isdigit():
            print("Стен нет")
            sensors_str = "0000"
            
        # Разбираем строку посимвольно в булевы переменные (True/False)
        sf = sensors_str[0] == '1' # Спереди
        sr = sensors_str[1] == '1' # Справа
        sb = sensors_str[2] == '1' # Сзади
        sl = sensors_str[3] == '1' # Слева
        
        # Пересчитываем в глобальную систему координат по углу corn
        calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
        
        # Отправляем пачку оранжевых стен на экран монитора
        send_msg(f"wall:{rx},{ry}:{calculated_walls}")
        print(f"[UDP Отправлено] Глобальные стены клетки: {calculated_walls if calculated_walls else 'Свободно'}")

if __name__ == "__main__":
    main()