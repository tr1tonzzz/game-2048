# Arduino IDE version - ESP32 Game 2048

Repo co 2 phien ban:

- `Game2048_ESP32/`: phien ban ESP32 dung Arduino IDE.
- `STM32F429_TouchGFX_2048/`: phien ban dung STM32F429ZIT6 + STM32CubeIDE + TouchGFX + man/touch onboard. Day la phien ban nen dung neu ban chi co board STM32F429ZIT6 co man cam ung onboard.

Thu muc sketch Arduino IDE:

`Game2048_ESP32/Game2048_ESP32.ino`

File `.ino` chi goi `setup()` va `loop()`. Phan chuong trinh chinh nam trong `App.cpp`; logic game nam trong `Game2048.cpp`.

## Phan cung chinh

- ESP32 DevKit.
- LCD TFT ILI9341 SPI 240x320.
- Touch XPT2046 SPI.
- Joystick hoac nut dieu khien:
  - Joystick X: GPIO 34
  - Joystick Y: GPIO 35
  - Joystick SW: GPIO 32
  - Neu dung 4 nut bam thay joystick, sua `BTN_UP_PIN`, `BTN_DOWN_PIN`, `BTN_LEFT_PIN`, `BTN_RIGHT_PIN` trong `config.h`.
- Buzzer: GPIO 25.
- Flash cua ESP32: dung Preferences/NVS de luu van choi va diem cao.

## Thu vien can cai trong Arduino IDE

Vao `Tools > Manage Libraries...` va cai:

- `TFT_eSPI` by Bodmer.
- `XPT2046_Touchscreen` by Paul Stoffregen.

Vao `Boards Manager` va cai:

- `esp32` by Espressif Systems.

## Cau hinh Arduino IDE

1. Mo file `Game2048_ESP32/Game2048_ESP32.ino`.
2. Chon board: `ESP32 Dev Module`.
3. Chon dung cong COM cua ESP32.
4. Sua cau hinh TFT_eSPI:
   - Mo file `TFT_eSPI_User_Setup.h`.
   - Copy noi dung file nay vao file `User_Setup.h` cua thu vien `TFT_eSPI`.
   - File thu vien thuong nam trong `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`.
5. Bam `Verify` de bien dich, sau do `Upload`.

## Chan ket noi mac dinh

| Chuc nang | GPIO ESP32 |
| --- | --- |
| TFT MISO | 19 |
| TFT MOSI | 23 |
| TFT SCLK | 18 |
| TFT CS | 5 |
| TFT DC | 2 |
| TFT RST | 4 |
| Touch CS | 15 |
| Touch IRQ | 27 |
| Joystick X | 34 |
| Joystick Y | 35 |
| Joystick SW | 32 |
| Buzzer | 25 |

Neu dung nut bam thay joystick, gan cac hang sau trong `config.h` thanh GPIO that:

```cpp
constexpr int BTN_UP_PIN = 12;
constexpr int BTN_DOWN_PIN = 13;
constexpr int BTN_LEFT_PIN = 14;
constexpr int BTN_RIGHT_PIN = 26;
```

Dau moi nut theo kieu `INPUT_PULLUP`: mot chan nut vao GPIO, chan con lai vao GND.
