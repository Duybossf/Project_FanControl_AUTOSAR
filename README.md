# Project_FanControl_AUTOSAR: Hệ Thống Điều Khiển Quạt Làm Mát Theo Nhiệt Độ

Dự án này là một hệ thống nhúng đơn giản ứng dụng kiến trúc phần mềm **AUTOSAR Classic**, sử dụng vi điều khiển STM32F103C8T6 để điều khiển tốc độ quạt làm mát dựa trên nhiệt độ môi trường.

---
## Mục lục
- [1. Mục tiêu](#1-mục-tiêu-dự-án)
- [2. Mô tả chức năng yêu cầu](#2-mô-tả-chức-năng)
- [3. Danh sách linh kiện sử dụng](#3-danh-sách-linh-kiện)
- [4. Kiến trúc phần mềm (AUTOSAR Classic)](#4-kiến-trúc-phần-mềm-autosar-classic)
  - [4.1 MCAL Drivers](#41-lớp-mcal-drivers)
  - [4.2 Module IoHwAb](#42-module-iohwab)
- [5. Cấu trúc cây thư mục](#5-cấu-trúc-cây-thư-mục)
- [6. Các hình ảnh mô phỏng của dự án](#6-các-hình-ảnh-mô-phỏng-của-dự-án)

---
## 1. Mục tiêu dự án

- Triển khai thực tế các module AUTOSAR cơ bản: **MCAL** (Microcontroller Abstraction Layer) và **IoHwAb** (I/O Hardware Abstraction).
- Làm quen cách cấu hình và sử dụng các MCAL Driver: `Port`, `Dio`, `Adc`, `Pwm`.
- Thiết kế và phát triển module IoHwAb để phân tách logic ứng dụng và phần cứng.
- Thực hành xây dựng hệ thống: Điều khiển tốc độ quạt DC theo giá trị cảm biến nhiệt độ.

---

## 2. Mô tả chức năng

Hệ thống sử dụng vi điều khiển **STM32F103C8T6** với các yêu cầu vận hành sau:
- **Đọc nhiệt độ:** Thu thập dữ liệu nhiệt độ môi trường từ cảm biến analog (LM35).
- **Điều khiển quạt DC (12V):** Thay đổi tốc độ quạt tự động dựa vào ngưỡng nhiệt độ đo được:
  - **Dưới 30°C:** Quạt tắt (Duty cycle 0%).
  - **Từ 30°C đến dưới 40°C:** Quạt chạy ở mức 50%.
  - **Từ 40°C trở lên:** Quạt chạy ở mức tối đa 100%.
- **Hiển thị trạng thái (LED):** Bật sáng khi quạt đang hoạt động, tắt khi quạt ngừng.

---

## 3. Danh sách linh kiện

| Linh kiện | Chức năng |
| :--- | :--- |
| **STM32F103C8T6 ("Blue Pill")** | Vi điều khiển trung tâm |
| **LM35DZ** | Cảm biến đo nhiệt độ (Sử dụng ADC) |
| **MODULE MOSFET** | Điều khiển tốc độ quạt bằng xung PWM |
| **Quạt DC 12V (80x80mm)** | Quạt làm mát động cơ |
| **LED buil-in** | Đèn báo trạng thái hoạt động của quạt (On/Off) |

---

## 4. Kiến trúc phần mềm (AUTOSAR Classic)

### 4.1. Lớp MCAL Drivers
Cần cấu hình và triển khai các drivers được thiết kế theo Standard Peripheral Library của STM32
- **Port Driver:**
  - `PA0`: Input Analog (Dành cho ADC).
  - `PA8`: PWM Output (Sử dụng Timer1 Channel 1).
  - `PC13`: GPIO Output (Điều khiển trạng thái LED).
- **ADC Driver:** Cấu hình ADC1 chế độ single-channel, sử dụng DMA để lấy mẫu liên tục.
- **PWM Driver:** Sử dụng Timer1 Channel 1, tần số PWM thiết lập ở 10 kHz.
- **Dio Driver:** Điều khiển trạng thái mức logic của chân GPIO (LED).

### 4.2. Module IoHwAb 
Dự án được thiết kế hoàn toàn sử dụng các API IoHwAb (ECU Abstraction) và **không chứa logic điều khiển nhiệt độ**, tách biệt hoàn toàn so với MCAL.

```c
/* Khởi tạo module IoHwAb */
Std_ReturnType IoHwAb_Init(void);

/* Đọc nhiệt độ hiện tại (trả về giá trị °C) */
Std_ReturnType IoHwAb_ReadTemperature(uint8 *temp);

/* Thiết lập tốc độ quạt (duty cycle theo %) */
Std_ReturnType IoHwAb_SetFanDuty(uint8 percent);

/* Điều khiển trạng thái LED báo (TRUE = Sáng, FALSE = Tắt) */
Std_ReturnType IoHwAb_SetLed(boolean state);
```

---

## 5. Cấu trúc cây thư mục

Dự án được sắp xếp theo cấu trúc phân lớp như sau:

```text
Project_FanControl_AUTOSAR
│
├── main.c                    /* Đóng vai trò là Application, gọi các API thông qua IoHwAb */
├── Makefile                  /* Build */
├── README.md                 /* Project documentation */
│
├── IoHwAb                    /* ECU Abstraction Layer */
│   ├── IoHwAb.c              /* Triển khai hàm gọi xuống MCAL */
│   └── IoHwAb.h              
│
├── MCAL                      /* Microcontroller Abstraction Layer */
│   ├── Port                  /* Implementation các Peripheral */
│   │   ├── Port.c
│   │   └── Port.h
│   ├── Dio                   /* Implementation các I/O */
│   │   ├── Dio.c
│   │   └── Dio.h
│   ├── Adc                   /* Implementation ngoại vi ADC */
│   │   ├── Adc.c
│   │   └── Adc.h
│   └── Pwm                   /* Implementation ngoại vi PWM */
│       ├── Pwm.c
│       └── Pwm.h
│
└── Config                    /* Configuration Files cho MCAL */
    ├── Port_Cfg.c
    ├── Dio_Cfg.c
    ├── Adc_Cfg.c
    └── Pwm_Cfg.c
```

---

## 6. Các hình ảnh mô phỏng của dự án
