import socket

TARGET_IP = "127.0.0.1"
TARGET_PORT = 5002
sender_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_msg(text):
    sender_sock.sendto(text.encode('utf-8'), (TARGET_IP, TARGET_PORT))

def main():
    # Робот начинает в центре сетки 16х16 (индексы 8, 8)
    rx, ry = 8, 8
    r_dir = "up"
    
    test_photos = ['test_h.jpg', 'test_s.jpg', 'test_red.jpg']
    photo_idx = 0
    
    print("=" * 60)
    print("           КОНСОЛЬ УПРАВЛЕНИЯ СЕТЬЮ РОБОТА")
    print("=" * 60)
    print("Ходи кнопками: w, s, a, d.")
    print("Имитировать метку камеры: v")
    
    # Шлем стартовую позицию
    send_msg(f"pos:{rx},{ry}:{r_dir}")
    
    turn_left = {'up': 'left', 'left': 'down', 'down': 'right', 'right': 'up'}
    turn_right = {'up': 'right', 'right': 'down', 'down': 'left', 'left': 'up'}

    while True:
        print(f"\n[Робот на клетке: x={rx}, y={ry} | Направление: {r_dir.upper()}]")
        cmd = input("Твой ход (w/a/s/d/v) или 'exit': ").strip().lower()
        
        if cmd == 'exit': 
            break
            
        if cmd == 'w':
            if r_dir == 'up':    ry -= 1
            elif r_dir == 'down':  ry += 1
            elif r_dir == 'left':  rx -= 1
            elif r_dir == 'right': rx += 1
            print("Робот продвинулся на 1 клетку вперед.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 's':
            if r_dir == 'up':    ry += 1
            elif r_dir == 'down':  ry -= 1
            elif r_dir == 'left':  rx += 1
            elif r_dir == 'right': rx -= 1
            print("Робот сдал назад.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 'a':
            r_dir = turn_left[r_dir]
            print("Разворот колес влево.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 'd':
            r_dir = turn_right[r_dir]
            print("Разворот колес вправо.")
            send_msg(f"pos:{rx},{ry}:{r_dir}")
            
        elif cmd == 'v':
            label = input("Буква метки ИИ (H / S / U): ").strip().upper() or "H"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            print(f"Камера отправила снимок {filename} на экран.")
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            continue

        else:
            print("Неизвестная кнопка!")
            continue

        if not (0 <= rx < 16 and 0 <= ry < 16):
            print("🛑 Выезд за пределы поля! Телепортация на базу [8,8].")
            rx, ry = 8, 8
            send_msg(f"pos:{rx},{ry}:{r_dir}")

        # ОБНОВЛЕННЫЙ ВВОД СТЕН:
        # Просто перечисли через запятую направления, где робот видит стены.
        # Например: up,left или просто right. Если стен нет — жми Enter.
        wall_input = input("Где стены? (Перечисли через запятую: up,down,left,right или пусто): ").strip().lower()
        
        # Отправляем пачку стен одной понятной строчкой
        send_msg(f"wall:{rx},{ry}:{wall_input}")

if __name__ == "__main__":
    main()