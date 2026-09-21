#!/usr/bin/env bash

# 1. Tạo thư mục drivers
mkdir -p drivers

# 2. Di chuyển các file liên quan vào drivers/
# Kiểm tra nếu file tồn tại thì mới chuyển để tránh lỗi
for file in io.s io.h serial.c serial.h; do
    if [ -f "$file" ]; then
        mv "$file" drivers/
        echo "Đã chuyển $file -> drivers/"
    fi
done

echo "Hoàn tất tổ chức file vào drivers/!"
