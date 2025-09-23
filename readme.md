<!--
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-20 16:20:48
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 16:14:41
 * @FilePath: \ele_ds_server\readme.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->

# 电子桌搭服务器

## 程序使用库

- cjson 

- curl `sudo apt-get install libcurl4-openssl-dev`
  
  - 如果安装不了就手动安装`./resource/libcjson-dev_1.7.15-1_amd64.deb`和`libcjson1_1.7.15-1_amd64.deb`.

- glib `sudo apt-get install libglib2.0-dev`
  
  - 如果报错找不到`libconfig.h` 就咋`Makefile`添加 `-I/usr/lib/x86_64-linux-gnu/glib-2.0/include/`

- readline `sudo apt-get install libreadline-dev`

- sqlite3 `sudo apt install libsqlite3-dev sqlite3`

## 待实现功能

- [ ] 给离线设备发送消息, 要保存到数据库中, 然后设备上线时发送.

- [ ] 升级终端时是否需要每个终端单独一个线程实现对升级数据发送, 升级是否完成验证.

## 终端连接服务器说明

- 服务器运行`frpc`后开启代理且关闭认证, 终端使用tcp连接服务器, 暂时使用`24680`端口.

## 移植到ipq50xx上流程
  - [下载sdk](https://archive.openwrt.org/releases/19.07.10/targets/ipq40xx/generic/openwrt-sdk-19.07.10-ipq40xx-generic_gcc-7.5.0_musl_eabi.Linux-x86_64.tar.xz)
  - 修改`feeds.conf.default`内容为
    ```shell
    src-git base https://git.openwrt.org/openwrt/openwrt.git;openwrt-19.07
    src-git packages https://github.com/openwrt/packages.git;openwrt-19.07
    src-git luci https://github.com/openwrt/luci.git;openwrt-19.07
    src-git routing https://git.openwrt.org/feed/routing.git;openwrt-19.07
    src-git telephony https://git.openwrt.org/feed/telephony.git;openwrt-19.07
    src-git freifunk https://github.com/freifunk/openwrt-packages.git
    ```
  - 更新并暗装
    ```shell
    ./scripts/feeds update -a
    ./scripts/feeds install -a
    
    make package/mbedtls/compile V=s
    make package/readline/compile V=s
    make package/curl/compile V=s
    make package/glib/compile V=s
    # 如果curl有报错lmbed*库丢失可能需要在库内软连接以下库
    ln -sf libmbedx509.so.2.16.12 libmbedx509.so.0
    ln -sf libmbedcrypto.so.2.16.12 libmbedcrypto.so.3
    ln -sf libmbedtls.so.12 libmbedtls.so
    ```