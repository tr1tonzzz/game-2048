# GAME 2048 TRÊN KIT STM32F429I-DISCO

Dự án cuối kỳ môn học **Hệ Nhúng** (Mã lớp: **166152**) - Trường Công nghệ Thông tin & Truyền thông - Đại học Bách khoa Hà Nội.

Project thực hiện trò chơi 2048 chạy trực tiếp trên kit phát triển STM32F429I-DISCO / STM32F429I-DISC1, sử dụng thư viện đồ họa TouchGFX và bộ nhớ Flash trong để lưu trữ dữ liệu ván chơi bền vững.

---

## 👥 Thành viên nhóm thực hiện & Đóng góp

| MSSV | Họ và tên | Nhiệm vụ chính | Đóng góp |
| :--- | :--- | :--- | :---: |
| **20235791** | **Phạm Hải Nam** | Thiết kế giao diện TouchGFX không dùng ảnh, hệ thống đổi 3 theme màu, phát triển thuật toán hiệu ứng chuyển động (animation zoom/pulse), viết báo cáo. | **25%** |
| **20235719** | **Bùi Huy Hoàng** | Thiết kế và kiểm thử thư viện logic game 2048, xử lý gộp ô, tính điểm, undo nhiều bước trong engine. | **25%** |
| **20235877** | **Lê Anh Vũ** | Phát triển bộ xử lý cử chỉ vuốt (swipe gesture), mở rộng vùng nhận cảm ứng nút bấm ở viền dưới màn hình và hệ thống tín hiệu phản hồi ảo. | **25%** |
| **20235806** | **Nguyễn Chấn Phong** | Phát triển bộ giao tiếp ghi Flash trong Sector 23, xử lý tắt/bật ngắt an toàn và reset ART Accelerator cache để tránh lỗi HardFault. | **25%** |

*Giảng viên hướng dẫn:* **Thầy Đỗ Công Thuần**

---

## 🛠️ Công cụ phát triển & Phiên bản (Tool Versions)

Để cài đặt, biên dịch và chạy dự án này, bạn cần cài đặt các công cụ với phiên bản khuyến nghị sau:

1.  **STM32CubeIDE (v1.13.0 trở lên):** Môi trường lập trình chính tích hợp GCC, dùng để biên dịch mã nguồn C++ và nạp chương trình lên kit.
2.  **TouchGFX Designer (v4.22.0 trở lên):** Công cụ thiết kế giao diện kéo thả trực quan và tự động sinh code đồ họa TouchGFX.
3.  **Trình biên dịch:** GNU C++14 (`arm-none-eabi-g++`).
4.  **FreeRTOS (CMSIS-RTOS V2):** Hệ điều hành thời gian thực cấu hình qua CubeMX dùng để điều phối luồng TouchGFX và hệ thống.

---

## 📐 Cấu hình phần cứng (Hardware Specifications)

*   **Bộ xử lý chính:** Vi điều khiển **STM32F429ZIT6** (lõi ARM Cortex-M4, chạy ở tần số clock **180 MHz**).
*   **Màn hình:** LCD TFT onboard 2.4 inch QVGA (độ phân giải 240x320), giao tiếp qua bộ điều khiển LTDC.
*   **Touch Controller:** Chip cảm ứng điện trở onboard **STMPE811**, giao tiếp qua bus I2C3.
*   **Bộ nhớ lưu trữ:** Sử dụng **Internal Flash (Sector 23)** bắt đầu từ địa chỉ `0x081E0000` đến `0x081FFFFF` (dung lượng 128KB) dùng làm phân vùng lưu Snapshot bàn cờ và High Score cho từng kích thước lưới.
*   **Phản hồi cảm biến ảo (Virtual Buzzer):** Hiển thị trực quan thông điệp trạng thái (`MOVE`, `MERGE`, `SAVED`, `GAME OVER`, `NO UNDO`) trực tiếp trên màn hình thay thế cho còi vật lý.

---

## 📁 Cấu trúc thư mục dự án (Repository Directory Structure)

Dự án được tổ chức theo **Phương án 1** (các thư mục mã nguồn đặt trực tiếp tại thư mục gốc của repository giúp import thuận tiện nhất):

```text
ten-repository/
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

## 🚀 Hướng dẫn cài đặt, Biên dịch & Chạy dự án (Build & Run)

### Bước 1: Clone dự án về máy tính
Sử dụng Git để tải mã nguồn dự án về máy của bạn:
```bash
git clone <URL_REPOS_CUA_BAN>
```

### Bước 2: Tạo code giao diện bằng TouchGFX Designer
1. Tải và cài đặt **TouchGFX Designer v4.22.0**.
2. Điều hướng vào thư mục dự án và kích đúp mở file `TouchGFX/MyApplication_2.touchgfx`.
3. Nhấn nút **Generate Code** (ở góc dưới cùng bên phải màn hình TouchGFX Designer) để công cụ tự động tạo mã nguồn giao diện tương thích.

### Bước 3: Import project vào STM32CubeIDE
1. Tải và mở **STM32CubeIDE v1.13.0**.
2. Chọn `File > Import > Existing Projects into Workspace`.
3. Nhấp vào `Browse...` và chọn thư mục gốc của dự án vừa clone.
4. Tích chọn project `STM32F429I-DISCO` và nhấn `Finish`.
5. Sau khi import xong, nhấn chuột phải vào thư mục project trong giao diện Explorer chọn `Refresh`.

### Bước 4: Biên dịch và nạp code
1. Chọn `Project > Clean` để dọn dẹp các tệp build cũ.
2. Nhấn tổ hợp phím `Ctrl + B` (hoặc vào `Project > Build Project`) để bắt đầu biên dịch toàn bộ dự án.
3. Kết nối kit STM32F429I-DISCO vào máy tính qua cổng USB ST-LINK.
4. Nhấn nút **Run** (biểu tượng Play màu xanh) hoặc **Debug** (biểu tượng con bọ) trên thanh công cụ của CubeIDE để nạp code trực tiếp xuống kit và khởi chạy game.

---

## 🎮 Hướng dẫn chơi & Các chức năng chính

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

## 📺 Video ghi lại hoạt động thực tế

*   **Các chức năng chơi cơ bản & chuyển chế độ lưới:** [Link Video Youtube/Drive](https://example.com) *(Người dùng tự cập nhật đường dẫn)*
*   **Các phím chức năng undo, save, load, new:** [Link Video Youtube/Drive](https://example.com) *(Người dùng tự cập nhật đường dẫn)*
