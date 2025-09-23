# 目标文件名
TARGET = ele_ds_srever

# 源文件目录
SRCDIRS = . ./common ./weather ./server ./client ./command ./users

# 目标文件目录
OBJDIR = build

# 源代码文件
SRCS = $(wildcard $(addsuffix /*.c, $(SRCDIRS)))

# 对应的目标文件
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

WEATHER_API_KEY = "e7d95a70480a4d6c9140378d9d100d42"
RUN_TARGET ?= pc

# --------------------------
# 平台校验与配置
# --------------------------
# 允许的目标平台列表
ALLOWED_TARGETS := pc ipq50xx

# 检查目标平台是否合法（核心：用filter判断是否在允许列表中）
ifneq ($(filter $(RUN_TARGET), $(ALLOWED_TARGETS)),)
    # 平台合法：配置对应参数（pc或ipq50xx）
    ifeq ($(RUN_TARGET), pc)
        # PC平台配置
        CC = gcc
        CFLAGS = -Wall -Wextra -O2 \
                 -DWEATHER_API_KEY=\"$(WEATHER_API_KEY)\" \
                 -DPLATFORM_PC \
                 -std=gnu99 \
                 -I/usr/include/glib-2.0 \
                 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
                 -g -rdynamic
        LDFLAGS = -lcurl -lcjson -lpthread -lglib-2.0 -lreadline -lsqlite3 -lz -g
        INCLUDES = -I. -Icommon -Iweather
    else ifeq ($(RUN_TARGET), ipq50xx)
		SDK_DIR ?= /root/learn/openwrt/openwrt-sdk-19.07.10-ipq40xx-generic_gcc-7.5.0_musl_eabi.Linux-x86_64
		STAGING_DIR ?= $(SDK_DIR)/staging_dir/target-arm_cortex-a7+neon-vfpv4_musl_eabi
		TOOLCHAIN_DIR ?= $(SDK_DIR)/staging_dir/toolchain-arm_cortex-a7+neon-vfpv4_gcc-7.5.0_musl_eabi/bin

		CC = $(TOOLCHAIN_DIR)/arm-openwrt-linux-muslgnueabi-gcc

		CFLAGS = -Wall -Wextra -O2 \
				 -DWEATHER_API_KEY=\"$(WEATHER_API_KEY)\" \
				 -I$(STAGING_DIR)/usr/include \
				 -I$(STAGING_DIR)/usr/include/glib-2.0 \
				 -I$(STAGING_DIR)/usr/include/readline \
				 -I$(STAGING_DIR)/usr/include/curl \
				 -I. -Icommon -Iweather \
				 -DPLATFORM_IPQ50XX \
				 -std=gnu99 \
				 -g -rdynamic

		LDFLAGS = -L$(STAGING_DIR)/usr/lib \
				  -L$(STAGING_DIR)/root-ipq40xx/usr/lib \
				  -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
				  -lcurl -lcjson -lpthread -lglib-2.0 -lreadline -lsqlite3 -lz \
				  -lmbedtls -lmbedx509 -lmbedcrypto -g
    endif
else
    # 平台不合法：打印用法并退出
    $(error 无效的目标平台 "$(RUN_TARGET)"！请使用以下命令: \
      编译PC版本: make RUN_TARGET=pc \
   	  编译ipq50xx版本: make RUN_TARGET=ipq50xx)
endif

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
