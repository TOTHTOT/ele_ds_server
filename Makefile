# 目标文件名
TARGET = ele_ds_srever

# 源文件目录
SRCDIRS = . ./common ./weather ./server ./client

# 目标文件目录
OBJDIR = build

# 源代码文件
SRCS = $(wildcard $(addsuffix /*.c, $(SRCDIRS)))

# 对应的目标文件
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

# 编译器
CC = gcc

# 编译选项
WEATHER_API_KEY = "e7d95a70480a4d6c9140378d9d100d42"
CFLAGS = -Wall -Wextra -O2 -DWEATHER_API_KEY=\"$(WEATHER_API_KEY)\" -std=gnu99

# 链接选项
LDFLAGS = -lcurl -lcjson -lpthread

# 头文件目录
INCLUDES = -I. -Icommon -Iweather

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译 .c 文件为 .o 文件
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: clean
