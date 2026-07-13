# GAME 2048 TRÊN KIT STM32F429I-DISCO

Project thực hiện trò chơi 2048 chạy trực tiếp trên kit phát triển STM32F429I-DISCO / STM32F429I-DISC1, sử dụng thư viện đồ họa TouchGFX và bộ nhớ Flash trong để lưu trữ dữ liệu ván chơi bền vững.

---

## Công cụ phát triển & Phiên bản (Tool Versions)

Để cài đặt, biên dịch và chạy dự án này, bạn cần cài đặt các công cụ với phiên bản khuyến nghị sau:

1.  **STM32CubeIDE (v2.2.0 trở lên):** Môi trường lập trình chính tích hợp GCC, dùng để biên dịch mã nguồn C++ và nạp chương trình lên kit.
2.  **TouchGFX Designer (v4.26.0 trở lên):** Công cụ thiết kế giao diện kéo thả trực quan và tự động sinh code đồ họa TouchGFX.
3.  **Trình biên dịch:** GNU C++14 (`arm-none-eabi-g++`).
4.  **FreeRTOS (CMSIS-RTOS V2):** Hệ điều hành thời gian thực cấu hình qua CubeMX dùng để điều phối luồng TouchGFX và hệ thống.

---

## Cấu hình phần cứng (Hardware Specifications)

*   **Bộ xử lý chính:** Vi điều khiển **STM32F429ZIT6** (lõi ARM Cortex-M4, chạy ở tần số clock **180 MHz**).
*   **Màn hình:** LCD TFT onboard 2.4 inch QVGA (độ phân giải 240x320), giao tiếp qua bộ điều khiển LTDC.
*   **Touch Controller:** Chip cảm ứng điện trở onboard **STMPE811**, giao tiếp qua bus I2C3.
*   **Bộ nhớ lưu trữ:** Sử dụng **Internal Flash (Sector 23)** bắt đầu từ địa chỉ `0x081E0000` đến `0x081FFFFF` (dung lượng 128KB) dùng làm phân vùng lưu Snapshot bàn cờ và High Score cho từng kích thước lưới.
*   **Phản hồi cảm biến ảo (Virtual Buzzer):** Hiển thị trực quan thông điệp trạng thái (`MOVE`, `MERGE`, `SAVED`, `GAME OVER`, `NO UNDO`) trực tiếp trên màn hình thay thế cho còi vật lý.

---

## Cấu trúc thư mục dự án (Repository Directory Structure)
```text
repository/
├── Core/                       # Cấu hình phần cứng sinh bởi STM32CubeMX (HAL drivers, GPIO, Clock)
├── Drivers/                    # Drivers của board STM32F429 và cảm ứng STMPE811
├── Middlewares/                # Kernel FreeRTOS
├── STM32CubeIDE/               # Chứa các file cấu hình project và output build (Debug, Release)
├── TouchGFX/                   # Toàn bộ mã nguồn giao diện & logic game
│   ├── MyApplication_2.touchgfx # File thiết kế giao diện TouchGFX Designer
│   ├── assets/                 # Quản lý Fonts, Texts và Images
│   └── gui/                    # Mã nguồn C++ chính của game (Model-View-Presenter)
│       ├── src/common/         # Logic Game Engine và Module Flash Storage
│       └── src/screen1_screen/ # Xử lý đồ họa vẽ bàn cờ, swipe gesture, onTap button
│
├── docs/                       # Tài liệu báo cáo
│   ├── BaoCao_CuoiKy_Nhom68.pdf # File báo cáo PDF đã biên dịch hoàn chỉnh
│   ├── report-game2048.tex     # File nguồn LaTeX của báo cáo
│   ├── logo.jpg                # Logo trường dùng cho LaTeX
│   ├── anh1.jpg                # Ảnh chụp Theme Classic (dùng cho báo cáo)
│   ├── anh2.jpg                # Ảnh chụp Theme Dark (dùng cho báo cáo)
│   └── anh3.jpg                # Ảnh chụp Theme Blue/Neon (dùng cho báo cáo)
├── .gitignore                  # File cấu hình bỏ qua các file build sinh ra tự động (Debug/, build/)
├── STM32F429I_DISCO_REV_D01.ioc # File cấu hình STM32CubeMX
└── README.md                   # Tài liệu hướng dẫn này
```

---

## Hướng dẫn cài đặt, Biên dịch & Chạy dự án (Build & Run)

### Bước 1: Clone dự án về máy tính
Sử dụng Git để tải mã nguồn dự án về máy của bạn:
```bash
git clone <URL_REPOS_CUA_BAN>
```

### Bước 2: (Tùy chọn) Tạo code giao diện bằng TouchGFX Designer
*Lưu ý: Toàn bộ thư mục `generated` và thư viện TouchGFX đã được nhóm đẩy sẵn lên GitHub, vì vậy thành viên khác **KHÔNG BẮT BUỘC** phải cài đặt TouchGFX Designer hoặc chạy Generate Code, vẫn có thể mở STM32CubeIDE build dự án bình thường.*
Nếu bạn cần chỉnh sửa, thiết kế lại giao diện game:
1. Mở file `TouchGFX/MyApplication_2.touchgfx` bằng **TouchGFX Designer v4.26.0 trở lên**.
2. Thực hiện sửa đổi giao diện và nhấn nút **Generate Code** (phím `F4`) ở góc dưới phải màn hình để tự động cập nhật mã nguồn giao diện.

### Bước 3: Mở project bằng STM32CubeIDE
1. Mở **STM32CubeIDE v2.2.0**.
2. Mở dự án bằng cách mở trực tiếp file `.project` tại thư mục `STM32CubeIDE` trong thư mục repo GitHub vừa clone về máy (`game-2048\STM32CubeIDE\.project`), hoặc chọn `File > Open Projects from File System...` trong STM32CubeIDE và chỉ tới thư mục `STM32CubeIDE`.
3. Nhấp chuột phải vào tên project `STM32F429I-DISCO` trong giao diện Project Explorer và chọn `Refresh`.

### Bước 4: Biên dịch và nạp code (Build & Run)
1. Chọn `Project > Clean` để dọn dẹp các tệp build cũ.
2. Nhấn tổ hợp phím `Ctrl + B` (hoặc chọn `Project > Build Project`) để biên dịch toàn bộ dự án.
3. Kết nối kit STM32F429I-DISCO vào máy tính qua cổng USB ST-LINK.
4. Nhấn nút **Run** (biểu tượng Play màu xanh) trên thanh công cụ của CubeIDE để nạp chương trình trực tiếp xuống kit và khởi chạy game trên phần cứng thực tế.

---

## Hướng dẫn chơi & Các chức năng chính

### 1. Cách chơi cơ bản
*   **Điều khiển:** Người chơi thực hiện thao tác vuốt (swipe gesture) theo 4 hướng: **Lên, Xuống, Trái, Phải** ở bất kỳ vùng trống nào trên màn hình cảm ứng để di chuyển và gộp các ô số.
*   **Luật gộp ô:** Hai ô có cùng giá trị khi va chạm sẽ gộp lại thành một ô có giá trị gấp đôi (ví dụ: `2 + 2 = 4`, `4 + 4 = 8`,...). Điểm số sẽ tăng thêm một lượng bằng giá trị của ô mới được gộp.
*   **Quy tắc sinh ô:** Ô số mới (giá trị 2 hoặc 4) chỉ xuất hiện ngẫu nhiên ở một ô trống sau một lượt di chuyển hợp lệ.

### 2. Các phím chức năng nhanh (ở cạnh dưới màn hình, tọa độ $y \ge 280$)
*   **UNDO:** Quay lại trạng thái bàn cờ ở lượt đi liền trước (hỗ trợ quay lại 1 lượt).
*   **NEW:** Bắt đầu lại ván chơi mới trên kích thước lưới hiện tại (không ảnh hưởng đến dữ liệu đã lưu thủ công qua Flash).
*   **SAVE:** Thực hiện lưu trạng thái bàn cờ hiện tại (kích thước lưới, vị trí các ô số, điểm số hiện tại) bền vững vào **Sector 23** của bộ nhớ Flash trong của STM32.
*   **LOAD:** Tải lại trạng thái game đã lưu gần nhất từ bộ nhớ Flash.

### 3. Menu cài đặt nâng cao
Nhấn nút **MENU** để mở bảng điều khiển:
*   **SIZE- / SIZE+:** Thay đổi kích thước bàn chơi (hỗ trợ 4 chế độ lưới: **3x3, 4x4, 5x5, và 6x6**). Điểm kỷ lục (Best Score) được lưu độc lập cho từng kích thước lưới.
*   **THEME:** Thay đổi giao diện game qua 3 theme màu sắc khác nhau:
    1.  **Classic:** Tông màu nguyên bản giống bản game 2048 gốc.
    2.  **Dark:** Chế độ nền tối tối giản, dịu mắt.
    3.  **Blue/Neon:** Màu neon rực rỡ, hiện đại.
*   **PLAY:** Đóng menu và tiếp tục chơi.

---

## Video ghi lại hoạt động thực tế

*   **Các chức năng chơi cơ bản & chuyển chế độ lưới:** [Link Video Youtube](https://youtube.com/shorts/GAm-MwbM3Wo?feature=share) 
*   **Các phím chức năng undo, save, load, new:** [Link Video Youtube](https://youtube.com/shorts/NjY-U0CGxY0) 
