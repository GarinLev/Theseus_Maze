import subprocess
import time
import sys
import os
import serial

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

byte = ser.read(1)
s = byte.decode('ascii', errors='ignore')
while s != "s":
    byte = ser.read(1)
    s = byte.decode('ascii', errors='ignore')
    print(s)

def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    monitor_path = os.path.join(current_dir, "monitor_starter.py")
    camera_path = os.path.join(current_dir, "camera.py")
    robot_path = os.path.join(current_dir, "hand.py")
    python_cmd = sys.executable 

    monitor_process = subprocess.Popen([python_cmd, monitor_path])
    camera_process = subprocess.Popen([python_cmd, camera_path])
    time.sleep(1.5)

    subprocess.run([python_cmd, robot_path])
    
    monitor_process.terminate()
    camera_process.terminate()

if __name__ == "__main__":
    main()