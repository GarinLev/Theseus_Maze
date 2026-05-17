import subprocess
import time
import sys
import os

def main():
    print("[Launcher] Запуск симулятора лабиринта...")

    # Получаем правильный путь к текущей папке, где лежат скрипты
    current_dir = os.path.dirname(os.path.abspath(__file__))
    monitor_path = os.path.join(current_dir, "monitor_starter.py")
    robot_path = os.path.join(current_dir, "main_robot.py")

    # Имя команды python в зависимости от операционной системы
    python_cmd = sys.executable 

    # 1. ЗАПУСКАЕМ ГРАФИЧЕСКИЙ МОНИТОР IMGUI
    # Запускаем как фоновый процесс, чтобы он открыл свое окно
    print("[Launcher] Открываем окно ImGui...")
    monitor_process = subprocess.Popen([python_cmd, monitor_path])

    # Ждем 1.5 секунды, чтобы графика загрузилась и сокет занял порт 5002
    time.sleep(1.5)

    # 2. ЗАПУСКАЕМ КНШОЛЬНУЮ ЛОГИКУ РОБОТА
    # Запускаем через .run(), чтобы весь ввод-вывод (input) шел прямо в этот терминаль
    print("[Launcher] Запуск пульта управления роботом:\n")
    try:
        subprocess.run([python_cmd, robot_path])
    except KeyboardInterrupt:
        print("\n[Launcher] Выход по Ctrl+C.")
    finally:
        # Когда закрывается пульт управления, принудительно тушим и окно графики
        print("[Launcher] Закрываем графическое окно...")
        monitor_process.terminate()

if __name__ == "__main__":
    main()