# 🔧 Настройка pigpio для FPV Interceptor

## 📋 Обзор

FPV Interceptor использует библиотеку **pigpio** вместо устаревшей wiringPi для работы с GPIO и SPI на Raspberry Pi 4.

## 🚀 Установка pigpio

### Автоматическая установка:
```bash
# Установка через apt
sudo apt update
sudo apt install -y pigpio libpigpio-dev

# Или через make
make install-deps
```

### Ручная установка:
```bash
# Скачивание и компиляция pigpio
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
```

## ⚙️ Настройка системы

### 1. Включение pigpio демона:
```bash
# Запуск pigpio демона
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

# Проверка статуса
sudo systemctl status pigpiod
```

### 2. Настройка автозапуска:
```bash
# Добавление в автозагрузку
sudo systemctl enable pigpiod

# Проверка
sudo systemctl is-enabled pigpiod
```

### 3. Настройка прав пользователя:
```bash
# Добавление пользователя в группу gpio
sudo usermod -a -G gpio $USER

# Перезагрузка для применения изменений
sudo reboot
```

## 🔧 Конфигурация

### Файл конфигурации pigpio:
```bash
# Создание конфигурации
sudo nano /etc/systemd/system/pigpiod.service.d/override.conf
```

Содержимое:
```ini
[Service]
ExecStart=
ExecStart=/usr/bin/pigpiod -l
```

### Настройка параметров:
```bash
# Запуск с параметрами
sudo pigpiod -l -p 8888

# Где:
# -l = локальный режим
# -p 8888 = порт для TCP соединений
```

## 🐍 Python интеграция

### Установка Python библиотеки:
```bash
# Установка pigpio для Python
pip3 install pigpio

# Или через apt
sudo apt install -y python3-pigpio
```

### Пример использования:
```python
import pigpio
import time

# Подключение к pigpio демону
pi = pigpio.pi()

if pi.connected:
    # Настройка пина как выход
    pi.set_mode(18, pigpio.OUTPUT)
    
    # Включение/выключение
    pi.write(18, 1)  # Включить
    time.sleep(1)
    pi.write(18, 0)  # Выключить
    
    # Закрытие соединения
    pi.stop()
else:
    print("Ошибка подключения к pigpio")
```

## 📡 SPI настройка

### Включение SPI:
```bash
# Включение SPI в config.txt
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt

# Перезагрузка
sudo reboot
```

### Проверка SPI:
```bash
# Проверка устройств SPI
ls /dev/spi*

# Должно показать:
# /dev/spidev0.0
# /dev/spidev0.1
```

### Пример SPI с pigpio:
```python
import pigpio

pi = pigpio.pi()

# Открытие SPI
spi_handle = pi.spi_open(0, 2000000)  # Канал 0, 2MHz

# Запись данных
pi.spi_write(spi_handle, [0x01, 0x02, 0x03])

# Чтение данных
count, data = pi.spi_read(spi_handle, 3)

# Закрытие SPI
pi.spi_close(spi_handle)
pi.stop()
```

## 🔍 Тестирование

### Тест GPIO:
```bash
# Простой тест GPIO
pigs w 18 1  # Включить пин 18
pigs w 18 0  # Выключить пин 18
```

### Тест SPI:
```bash
# Тест SPI через pigs
pigs spio 0 2000000 0  # Открыть SPI канал 0
pigs spiw 0 0x01 0x02  # Записать данные
pigs spic 0            # Закрыть SPI
```

### Python тест:
```python
import pigpio

pi = pigpio.pi()
if pi.connected:
    print("✅ pigpio подключен")
    pi.stop()
else:
    print("❌ Ошибка подключения к pigpio")
```

## 🛠️ Устранение неполадок

### Проблема: pigpio не запускается
```bash
# Проверка статуса
sudo systemctl status pigpiod

# Перезапуск
sudo systemctl restart pigpiod

# Проверка логов
journalctl -u pigpiod
```

### Проблема: Нет доступа к GPIO
```bash
# Проверка групп пользователя
groups $USER

# Добавление в группу gpio
sudo usermod -a -G gpio $USER

# Перезагрузка
sudo reboot
```

### Проблема: SPI недоступен
```bash
# Проверка SPI
ls /dev/spi*

# Включение SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

### Проблема: Python не может подключиться
```bash
# Проверка демона
sudo systemctl status pigpiod

# Запуск демона
sudo systemctl start pigpiod

# Проверка порта
netstat -tlnp | grep 8888
```

## 📊 Мониторинг

### Проверка статуса:
```bash
# Статус демона
sudo systemctl status pigpiod

# Использование ресурсов
ps aux | grep pigpiod

# Проверка соединений
netstat -tlnp | grep 8888
```

### Логи:
```bash
# Просмотр логов
journalctl -u pigpiod -f

# Логи системы
dmesg | grep -i spi
dmesg | grep -i gpio
```

## 🔒 Безопасность

### Ограничение доступа:
```bash
# Запуск только для локальных соединений
sudo pigpiod -l

# Ограничение портов
sudo pigpiod -l -p 8888
```

### Firewall:
```bash
# Блокировка внешнего доступа
sudo ufw deny 8888
sudo ufw allow from 127.0.0.1 to any port 8888
```

## 📈 Производительность

### Оптимизация:
```bash
# Увеличение приоритета
sudo nice -n -10 pigpiod

# Ограничение ресурсов
sudo systemctl edit pigpiod
```

### Мониторинг:
```bash
# Использование CPU
top -p $(pgrep pigpiod)

# Использование памяти
ps -o pid,ppid,cmd,%mem,%cpu -p $(pgrep pigpiod)
```

---

**🎯 pigpio настроен и готов к работе с FPV Interceptor!**

После настройки pigpio вы можете:
- Запустить тест оборудования: `python3 examples/test_hardware.py`
- Собрать программу: `make`
- Запустить FPV Interceptor: `./fpv_interceptor`

