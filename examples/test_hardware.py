#!/usr/bin/env python3
"""
Тест оборудования FPV Interceptor
Проверяет подключение RX5808, SPI, GPIO и видеоустройства
"""

import pigpio
import time
import subprocess
import sys
import os

# Конфигурация пинов
CS_PIN = 8
MOSI_PIN = 10
MISO_PIN = 9
SCK_PIN = 11
RSSI_PIN = 7

def test_gpio():
    """Тест GPIO пинов"""
    print("🔧 Тестирование GPIO...")
    
    try:
        pi = pigpio.pi()
        if not pi.connected:
            print("❌ Не удалось подключиться к pigpio")
            return False
        
        # Настройка пинов
        pi.set_mode(CS_PIN, pigpio.OUTPUT)
        pi.set_mode(MOSI_PIN, pigpio.OUTPUT)
        pi.set_mode(SCK_PIN, pigpio.OUTPUT)
        pi.set_mode(RSSI_PIN, pigpio.INPUT)
        
        # Тест выходных пинов
        pi.write(CS_PIN, 1)
        pi.write(MOSI_PIN, 0)
        pi.write(SCK_PIN, 0)
        time.sleep(0.1)
        
        pi.write(CS_PIN, 0)
        pi.write(MOSI_PIN, 1)
        pi.write(SCK_PIN, 1)
        time.sleep(0.1)
        
        # Тест входного пина RSSI
        rssi_value = pi.read(RSSI_PIN)
        
        print(f"✅ GPIO тест пройден")
        print(f"   RSSI pin {RSSI_PIN}: {rssi_value}")
        
        pi.stop()
        return True
        
    except Exception as e:
        print(f"❌ Ошибка GPIO: {e}")
        return False

def test_spi():
    """Тест SPI интерфейса"""
    print("📡 Тестирование SPI...")
    
    try:
        pi = pigpio.pi()
        if not pi.connected:
            print("❌ Не удалось подключиться к pigpio")
            return False
        
        # Открытие SPI
        spi_handle = pi.spi_open(0, 2000000)
        if spi_handle < 0:
            print("❌ Ошибка открытия SPI")
            pi.stop()
            return False
        
        # Настройка CS пина
        pi.set_mode(CS_PIN, pigpio.OUTPUT)
        
        # Тест записи
        pi.write(CS_PIN, 0)
        count, data = pi.spi_write(spi_handle, [0x01, 0x02, 0x03])
        pi.write(CS_PIN, 1)
        
        # Тест чтения
        pi.write(CS_PIN, 0)
        count, data = pi.spi_read(spi_handle, 3)
        pi.write(CS_PIN, 1)
        
        print(f"✅ SPI тест пройден")
        print(f"   Прочитанные данные: {data}")
        
        pi.spi_close(spi_handle)
        pi.stop()
        return True
        
    except Exception as e:
        print(f"❌ Ошибка SPI: {e}")
        return False

def test_video():
    """Тест видеоустройств"""
    print("📹 Тестирование видео...")
    
    try:
        # Поиск видеоустройств
        result = subprocess.run(['ls', '/dev/video*'], 
                              capture_output=True, text=True)
        
        if result.returncode == 0:
            devices = result.stdout.strip().split('\n')
            print(f"✅ Найдены видеоустройства: {devices}")
            return True
        else:
            print("❌ Видеоустройства не найдены")
            return False
            
    except Exception as e:
        print(f"❌ Ошибка видео: {e}")
        return False

def test_usb():
    """Тест USB устройств"""
    print("🔌 Тестирование USB...")
    
    try:
        result = subprocess.run(['lsusb'], capture_output=True, text=True)
        
        if result.returncode == 0:
            usb_devices = result.stdout
            print("✅ USB устройства:")
            print(usb_devices)
            
            # Поиск видеоустройств
            if 'video' in usb_devices.lower() or 'dvr' in usb_devices.lower():
                print("✅ Найдены USB видеоустройства")
                return True
            else:
                print("⚠️ USB видеоустройства не найдены")
                return False
        else:
            print("❌ Ошибка получения USB устройств")
            return False
            
    except Exception as e:
        print(f"❌ Ошибка USB: {e}")
        return False

def test_system():
    """Тест системных ресурсов"""
    print("💻 Тестирование системы...")
    
    try:
        # Проверка SPI
        if os.path.exists('/dev/spi0.0'):
            print("✅ SPI интерфейс доступен")
        else:
            print("❌ SPI интерфейс недоступен")
            return False
        
        # Проверка I2C
        if os.path.exists('/dev/i2c-1'):
            print("✅ I2C интерфейс доступен")
        else:
            print("⚠️ I2C интерфейс недоступен")
        
        # Проверка памяти
        with open('/proc/meminfo', 'r') as f:
            meminfo = f.read()
            for line in meminfo.split('\n'):
                if 'MemAvailable' in line:
                    mem_available = int(line.split()[1])
                    print(f"✅ Доступная память: {mem_available} KB")
                    break
        
        return True
        
    except Exception as e:
        print(f"❌ Ошибка системы: {e}")
        return False

def test_rx5808():
    """Тест модуля RX5808"""
    print("📡 Тестирование RX5808...")
    
    try:
        # Инициализация pigpio
        pi = pigpio.pi()
        if not pi.connected:
            print("❌ Не удалось подключиться к pigpio")
            return False
        
        # Настройка GPIO
        pi.set_mode(CS_PIN, pigpio.OUTPUT)
        pi.set_mode(MOSI_PIN, pigpio.OUTPUT)
        pi.set_mode(SCK_PIN, pigpio.OUTPUT)
        pi.set_mode(RSSI_PIN, pigpio.INPUT)
        
        # Инициализация SPI
        spi_handle = pi.spi_open(0, 2000000)
        if spi_handle < 0:
            print("❌ Ошибка открытия SPI")
            pi.stop()
            return False
        
        # Сброс модуля
        pi.write(CS_PIN, 0)
        time.sleep(0.1)
        pi.write(CS_PIN, 1)
        time.sleep(0.1)
        
        # Тест записи регистра
        pi.write(CS_PIN, 0)
        count, data = pi.spi_write(spi_handle, [0x8A, 0x00])  # Запись в регистр 0A
        pi.write(CS_PIN, 1)
        time.sleep(0.01)
        
        # Тест чтения регистра
        pi.write(CS_PIN, 0)
        count, data = pi.spi_read(spi_handle, 2)  # Чтение регистра 0A
        pi.write(CS_PIN, 1)
        
        print(f"✅ RX5808 тест пройден")
        print(f"   Прочитанные данные: {data}")
        
        # Тест RSSI
        rssi_values = []
        for i in range(10):
            rssi_values.append(pi.read(RSSI_PIN))
            time.sleep(0.1)
        
        avg_rssi = sum(rssi_values) / len(rssi_values)
        print(f"   Средний RSSI: {avg_rssi:.2f}")
        
        pi.spi_close(spi_handle)
        pi.stop()
        return True
        
    except Exception as e:
        print(f"❌ Ошибка RX5808: {e}")
        return False

def main():
    """Основная функция тестирования"""
    print("🚀 Запуск тестирования оборудования FPV Interceptor")
    print("=" * 60)
    
    tests = [
        ("GPIO", test_gpio),
        ("SPI", test_spi),
        ("Видео", test_video),
        ("USB", test_usb),
        ("Система", test_system),
        ("RX5808", test_rx5808)
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n🔍 Тест: {test_name}")
        print("-" * 40)
        
        try:
            if test_func():
                passed += 1
                print(f"✅ {test_name}: ПРОЙДЕН")
            else:
                print(f"❌ {test_name}: НЕ ПРОЙДЕН")
        except Exception as e:
            print(f"❌ {test_name}: ОШИБКА - {e}")
    
    print("\n" + "=" * 60)
    print(f"📊 Результаты тестирования: {passed}/{total}")
    
    if passed == total:
        print("🎉 Все тесты пройдены! Оборудование готово к работе.")
        return 0
    else:
        print("⚠️ Некоторые тесты не пройдены. Проверьте подключения.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
