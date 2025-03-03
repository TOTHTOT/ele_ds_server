# 目标文件名
TARGET = ele_ds_srever

# 源文件目录
SRCDIR = ./

# 目标文件目录
OBJDIR = build

# 源代码文件
SRCS = $(wildcard $(SRCDIR)/*.c)

# 对应的目标文件
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -O2 -DWEATHER_API_KEY=\"$(WEATHER_API_KEY)\"

# 头文件目录
INCLUDES = -Iinclude

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $^ -o $@

# 编译 .c 文件为 .o 文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: clean
