# STM32F429I-DISC1 TouchGFX Game 2048

Phiên bản này dành cho board **STM32F429I-DISC1** dùng chip **STM32F429ZIT6**, có sẵn màn hình LCD và cảm ứng onboard. Project được triển khai theo đúng hướng dùng **STM32CubeIDE + TouchGFX**.

Không cần dùng các module rời như TFT ILI9341 SPI, XPT2046, joystick hay nút bấm ngoài. Người chơi điều khiển game bằng thao tác **vuốt trên màn hình cảm ứng onboard**.

## 1. Phần cứng cần có

### Bắt buộc

- Board **STM32F429I-DISC1**.
- Cáp USB Mini-B hoặc Micro-B tùy phiên bản board để nạp/debug qua ST-LINK onboard.
- Máy tính cài Windows.

### Không bắt buộc

- Buzzer ngoài.

Board STM32F429I-DISC1 thường **không có buzzer onboard**, nên nếu không gắn thêm buzzer thì project sẽ dùng phản hồi hình ảnh trên màn hình thay cho âm báo.

## 2. Phần mềm cần cài

Cài các phần mềm sau:

1. **STM32CubeIDE**
   - Dùng để build, nạp code và debug.
   - Tải từ trang STMicroelectronics.

2. **TouchGFX Designer**
   - Dùng để tạo giao diện màn hình và generate code TouchGFX.
   - Thường được cài cùng hệ sinh thái STM32Cube.

3. **ST-LINK driver**
   - Nếu STM32CubeIDE không nhận board, cần cài thêm ST-LINK USB driver.

4. **STM32CubeProgrammer** *(khuyến nghị)*
   - Dùng để kiểm tra board có kết nối ST-LINK được không.

## 3. Nội dung thư mục này

```text
STM32F429_TouchGFX_2048/
├── Core/
│   ├── Game2048Engine.hpp
│   └── Game2048Engine.cpp
├── TouchGFX_Screen/
│   ├── GameScreenView.hpp
│   └── GameScreenView.cpp
├── README.md
└── report_touchgfx.tex
```

Ý nghĩa:

- `Core/Game2048Engine.hpp`, `Core/Game2048Engine.cpp`  
  Chứa logic game 2048 thuần C++. Phần này xử lý board, merge, score, undo, win/game over.

- `TouchGFX_Screen/GameScreenView.hpp`, `TouchGFX_Screen/GameScreenView.cpp`  
  Mã mẫu để tích hợp vào screen TouchGFX. Phần này xử lý thao tác vuốt, vẽ board và cập nhật giao diện.

- `report_touchgfx.tex`  
  Báo cáo LaTeX cho phiên bản STM32F429I-DISC1 + TouchGFX.

## 4. Tạo project TouchGFX cho STM32F429I-DISC1

### Bước 1: Mở TouchGFX Designer

Mở **TouchGFX Designer**.

### Bước 2: Tạo project mới

Chọn:

```text
Create New Project
```

Sau đó chọn board/template:

```text
STM32F429I-DISCO
```

hoặc template tương đương cho **STM32F429I-DISC1**.

Nếu TouchGFX Designer chỉ hiện `STM32F429I-DISCO`, vẫn dùng được cho STM32F429I-DISC1 trong đa số trường hợp vì cùng dòng board Discovery F429.

### Bước 3: Tạo màn hình giao diện

Tạo một screen, ví dụ:

```text
Screen1
```

Kích thước màn hình thường là:

```text
240 x 320
```

### Bước 4: Thêm typography/text resource

Trong TouchGFX Designer, tạo một text resource dùng cho text động, ví dụ:

```text
T_SINGLEUSEID1
```

Nếu project của bạn dùng tên text khác, khi copy code cần đổi `T_SINGLEUSEID1` thành tên text resource tương ứng.

### Bước 5: Generate code

Bấm:

```text
Generate Code
```

Sau bước này TouchGFX sẽ tạo project STM32CubeIDE đầy đủ, gồm LCD, touch, LTDC, SDRAM, DMA2D và các file generated.

## 5. Mở project bằng STM32CubeIDE

Trong TouchGFX Designer, chọn:

```text
Open in STM32CubeIDE
```

Hoặc mở STM32CubeIDE rồi import project vừa được TouchGFX generate.

## 6. Copy engine game 2048 vào project CubeIDE

Trong project CubeIDE vừa tạo, tạo thư mục tùy chọn, ví dụ:

```text
TouchGFX/gui/include/gui/common/
TouchGFX/gui/src/common/
```

### Cách tạo thư mục trong STM32CubeIDE

Trong cửa sổ **Project Explorer**:

1. Chuột phải vào thư mục:

```text
TouchGFX/gui/include/gui/
```

2. Chọn:

```text
New > Folder
```

3. Đặt tên:

```text
common
```

4. Làm tương tự với thư mục:

```text
TouchGFX/gui/src/
```

và cũng tạo thư mục:

```text
common
```

Sau đó bạn sẽ có:

```text
TouchGFX/gui/include/gui/common/
TouchGFX/gui/src/common/
```

### Copy file bằng File Explorer

Từ thư mục repo này, copy:

```text
STM32F429_TouchGFX_2048/Core/Game2048Engine.hpp
```

vào project CubeIDE:

```text
TouchGFX/gui/include/gui/common/Game2048Engine.hpp
```

Copy:

```text
STM32F429_TouchGFX_2048/Core/Game2048Engine.cpp
```

vào:

```text
TouchGFX/gui/src/common/Game2048Engine.cpp
```

Sau khi copy bằng File Explorer, quay lại STM32CubeIDE, chuột phải vào project và chọn:

```text
Refresh
```

### Hoặc tạo file trực tiếp trong STM32CubeIDE

Nếu không muốn copy bằng File Explorer:

1. Chuột phải vào:

```text
TouchGFX/gui/include/gui/common/
```

2. Chọn:

```text
New > Header File
```

3. Đặt tên:

```text
Game2048Engine.hpp
```

4. Copy toàn bộ nội dung từ file mẫu `Core/Game2048Engine.hpp` vào.

Sau đó tạo source file:

```text
TouchGFX/gui/src/common/Game2048Engine.cpp
```

bằng:

```text
New > Source File
```

rồi copy nội dung từ `Core/Game2048Engine.cpp` vào.

Copy file:

```text
Core/Game2048Engine.hpp
```

vào:

```text
TouchGFX/gui/include/gui/common/
```

Copy file:

```text
Core/Game2048Engine.cpp
```

vào:

```text
TouchGFX/gui/src/common/
```

Sau đó kiểm tra include path. Nếu CubeIDE báo không tìm thấy `Game2048Engine.hpp`, thêm thư mục include vào:

```text
Project Properties
> C/C++ General
> Paths and Symbols
> Includes
```

Thêm đường dẫn:

```text
TouchGFX/gui/include/gui/common
```

### Kiểm tra file `.cpp` đã được build chưa

Trong STM32CubeIDE, sau khi copy `Game2048Engine.cpp`, file này phải xuất hiện trong Project Explorer dưới:

```text
TouchGFX/gui/src/common/Game2048Engine.cpp
```

Nếu build bị lỗi kiểu undefined reference tới các hàm của `game2048::Engine`, thường là file `.cpp` chưa được đưa vào build. Khi đó:

1. Chuột phải vào `Game2048Engine.cpp`.
2. Chọn:

```text
Resource Configurations > Exclude from Build
```

3. Đảm bảo file **không bị exclude** khỏi cấu hình build hiện tại.

## 7. Tích hợp code vào Screen1View

TouchGFX sẽ tự tạo các file tương tự:

```text
TouchGFX/gui/include/gui/screen1_screen/Screen1View.hpp
TouchGFX/gui/src/screen1_screen/Screen1View.cpp
```

Nếu bạn đặt tên screen khác, ví dụ `GameScreen`, thì file có thể là:

```text
TouchGFX/gui/include/gui/gamescreen_screen/GameScreenView.hpp
TouchGFX/gui/src/gamescreen_screen/GameScreenView.cpp
```

Khi đó hãy dùng đúng file View do TouchGFX tạo, không bắt buộc phải là `Screen1View`.

Mở file mẫu trong repo:

```text
TouchGFX_Screen/GameScreenView.hpp
TouchGFX_Screen/GameScreenView.cpp
```

Sau đó copy các phần sau vào `Screen1View.hpp` và `Screen1View.cpp` do TouchGFX tạo:

- Các `#include` cần thiết.
- Các biến thành viên:
  - `game2048::Engine game_`
  - `highScore_`
  - `theme_`
  - `dragStartX_`, `dragStartY_`
  - các `Box`, `TextArea`, buffer text.
- Các hàm:
  - `setupScreen()`
  - `handleClickEvent()`
  - `handleDragEvent()`
  - `handleTickEvent()`
  - `drawAll()`
  - `drawHeader()`
  - `drawBoard()`
  - `drawStatus()`
  - `applyMove()`
  - `onTap()`
  - `restartCurrentBoard()`
  - `changeBoard()`
  - `cycleTheme()`
  - `saveGame()`
  - `loadGame()`

### Cách copy vào file `.hpp`

Mở file TouchGFX generated:

```text
TouchGFX/gui/include/gui/screen1_screen/Screen1View.hpp
```

Sau đó:

1. Thêm include:

```cpp
#include "Game2048Engine.hpp"
```

2. Thêm các biến thành viên và khai báo hàm từ file mẫu:

```text
TouchGFX_Screen/GameScreenView.hpp
```

vào phần `private:` của class `Screen1View`.

Không copy dòng:

```cpp
class Screen1View : public Screen1ViewBase
```

nếu file generated đã có sẵn class này. Chỉ copy phần biến/hàm cần thêm vào class.

### Cách copy vào file `.cpp`

Mở file TouchGFX generated:

```text
TouchGFX/gui/src/screen1_screen/Screen1View.cpp
```

Sau đó copy nội dung các hàm từ:

```text
TouchGFX_Screen/GameScreenView.cpp
```

vào file generated.

Nếu file generated đã có sẵn:

```cpp
void Screen1View::setupScreen()
void Screen1View::tearDownScreen()
```

thì không tạo thêm hàm trùng tên. Hãy thay phần thân hàm bằng nội dung tương ứng trong file mẫu, hoặc ghép thêm logic game vào trong hàm có sẵn.

### Nếu screen không tên `Screen1`

Nếu screen của bạn tên khác, ví dụ `GameScreen`, hãy đổi trong code mẫu:

```cpp
Screen1View
Screen1ViewBase
screen1_screen
Screen1Presenter
```

thành tên tương ứng mà TouchGFX đã generate.

Ví dụ:

```cpp
GameScreenView
GameScreenViewBase
gamescreen_screen
GameScreenPresenter
```

Tên chính xác nằm trong các file generated của TouchGFX.

### Lưu ý quan trọng

Không nên copy đè toàn bộ file generated nếu chưa đổi tên class/include cho khớp. Cách an toàn nhất là:

1. Giữ file `Screen1View.hpp/.cpp` do TouchGFX tạo.
2. Copy từng phần biến, hàm, và logic từ file mẫu vào.
3. Build thử.
4. Sửa từng lỗi tên text hoặc include nếu có.

## 8. Cách xử lý swipe

TouchGFX gửi sự kiện touch vào `handleClickEvent()`.

Code lưu tọa độ khi người dùng nhấn:

```cpp
dragStartX_ = event.getX();
dragStartY_ = event.getY();
```

Khi người dùng thả tay, code tính:

```cpp
int16_t dx = event.getX() - dragStartX_;
int16_t dy = event.getY() - dragStartY_;
```

Sau đó suy ra hướng:

```cpp
dx > 0  -> vuốt phải
dx < 0  -> vuốt trái
dy > 0  -> vuốt xuống
dy < 0  -> vuốt lên
```

Nếu độ lệch nhỏ hơn ngưỡng, thao tác được xem là tap để xử lý nút menu/undo/restart.

## 9. Build project

Trong STM32CubeIDE:

1. Chọn project.
2. Bấm:

```text
Project > Build Project
```

Nếu build lỗi, kiểm tra các lỗi thường gặp:

- Không tìm thấy `Game2048Engine.hpp`: thiếu include path.
- Không tìm thấy `T_SINGLEUSEID1`: chưa tạo text resource trong TouchGFX hoặc tên text khác.
- Sai tên screen/class: code mẫu dùng `Screen1View`, project của bạn có thể dùng tên khác.
- Lỗi generated code: cần mở TouchGFX Designer và generate lại code.

## 10. Nạp vào board

1. Cắm board STM32F429I-DISC1 vào máy tính bằng cáp USB qua cổng ST-LINK.
2. Trong STM32CubeIDE, bấm:

```text
Run
```

hoặc:

```text
Debug
```

Nếu không nạp được:

- Kiểm tra cáp USB có truyền dữ liệu, không chỉ sạc.
- Kiểm tra ST-LINK driver.
- Mở STM32CubeProgrammer xem có kết nối được board không.
- Kiểm tra jumper nguồn trên board nếu có.

## 11. Cách chơi

- Vuốt trái/phải/lên/xuống trên màn hình để di chuyển board.
- Nếu hai ô cùng giá trị va vào nhau, chúng được gộp.
- Sau mỗi lượt hợp lệ, game sinh thêm một ô mới.
- Đạt ô `2048` thì thắng.
- Hết nước đi thì game over.
- Tap vùng nút trên màn để undo/restart/save/load nếu đã bố trí UI tương ứng.

## 12. Chức năng đã có trong engine

- Board 3x3, 4x4, 5x5, 6x6.
- Di chuyển 4 hướng.
- Merge đúng luật 2048.
- Sinh ô mới sau lượt hợp lệ.
- Tính điểm.
- Phát hiện thắng.
- Phát hiện hết nước đi.
- Undo nhiều bước.
- Snapshot để lưu/khôi phục ván chơi.

## 13. Lưu điểm cao và ván chơi

Trong file mẫu có hai hàm:

```cpp
void Screen1View::saveGame()
void Screen1View::loadGame()
```

Hiện hai hàm này để khung vì cách ghi Flash trong STM32CubeIDE phụ thuộc:

- linker script,
- vùng Flash còn trống,
- sector Flash được chọn,
- cấu hình bảo vệ Flash,
- cách project TouchGFX đang dùng bộ nhớ.

Khi làm demo cơ bản, có thể chỉ lưu điểm cao trong RAM. Nếu muốn lưu sau reset, cần viết thêm module Flash Storage bằng HAL Flash hoặc dùng backup SRAM nếu board/project đã cấu hình.

## 14. Buzzer

Board STM32F429I-DISC1 thường không có buzzer onboard.

Vì vậy phiên bản này:

- không yêu cầu buzzer ngoài,
- không dùng linh kiện bạn không có,
- dùng phản hồi hình ảnh thay cho âm báo.

Nếu sau này được phép gắn buzzer ngoài, có thể dùng một chân GPIO có timer/PWM và viết thêm module buzzer bằng HAL TIM PWM.

## 15. Checklist để ra project chạy được

1. Tạo project TouchGFX đúng board `STM32F429I-DISCO`.
2. Generate code từ TouchGFX Designer.
3. Mở project bằng STM32CubeIDE.
4. Copy `Game2048Engine.hpp/.cpp` vào project.
5. Copy logic View từ `GameScreenView.hpp/.cpp` vào `Screen1View`.
6. Tạo text resource/wildcard trong TouchGFX.
7. Build project.
8. Sửa lỗi include hoặc tên text nếu có.
9. Nạp vào board bằng ST-LINK.
10. Kiểm tra touch bằng cách vuốt trên màn.
11. Quay video demo game chạy thật.

## 16. Ghi chú báo cáo

Trong báo cáo nên ghi rõ:

- Board sử dụng: **STM32F429I-DISC1**, chip **STM32F429ZIT6**.
- Công cụ: **STM32CubeIDE**, **TouchGFX Designer**.
- Input chính: **touch onboard**, điều khiển bằng swipe.
- Không dùng module màn/touch rời.
- Không có buzzer onboard, nên dùng phản hồi hình ảnh thay âm báo nếu không bổ sung buzzer ngoài.
