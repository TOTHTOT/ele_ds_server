<!--
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-03-20 16:20:48
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-28 15:41:38
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

## 终端连接服务器说明

 - 服务器运行`frpc`后开启代理且关闭认证, 终端使用tcp连接服务器, 暂时使用`24680`端口.
