import torch
from ultralytics import YOLO

# ЭТА СТРОКА ОБЯЗАТЕЛЬНА В WINDOWS ДЛЯ МУЛЬТИПРОЦЕССИНГА!
if __name__ == '__main__':
    
    print("=== ЗАПУСК ОБУЧЕНИЯ ===")
    print("CUDA доступна:", torch.cuda.is_available())
    
    # Загружаем чистую модель
    model = YOLO('yolov8n.pt')
    
    # Запускаем тренировку
    model.train(
        data='data.yaml',
        epochs=60,
        imgsz=640,
        device=0, # твоя RTX 4060 Ti
        workers=4, # потоки процессора
        batch=16
    )