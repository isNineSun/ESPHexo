| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

<p align="center">
  <img src="https://raw.githubusercontent.com/isNineSun/img_repository/main/aako8-4otcw-001.png" height=120>
</p>

# ESPHexo--Run Your Hexo Blog on ESP32
![GitHub repo size](https://img.shields.io/github/repo-size/isNineSun/ESPHexo)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/isNineSun/ESPHexo/total)
![GitHub issues](https://img.shields.io/github/issues/isNineSun/ESPHexo)
![GitHub License](https://img.shields.io/github/license/isNineSun/ESPHexo)

## 目录
- [ESPHexo--Run Your Hexo Blog on ESP32](#esphexo--run-your-hexo-blog-on-esp32)
  - [目录](#目录)
- [简介](#简介)
  - [特性](#特性)
  - [使用前的准备](#使用前的准备)
  - [推荐前置知识学习](#推荐前置知识学习)
- [🚀快速开始](#快速开始)
  

# 简介

这个项目可以让你在你的ESP32设备上运行Hexo博客，低功耗，便携，支持DDNS，支持自动拉取并更新Blog网站文件。    

## 特性
- 低功耗，使用esp32作为载体，功耗远远低于X86和绝大多数Linux平台。
- 部署简单，只需在配置文件中填写有限的几个的wifi、用户名等信息即可使用。
- 美观并且直观的Web配置界面，方便直接修改设备配置而无需经常手动修改SD卡中的配置文件。
- 支持公网DDNS，可以远程访问你的Blog网站。
- 信息监视屏，具有一个小的信息监视屏，直观的获取Blog的运行状态。

## 使用前的准备
在使用前，你或许需要做以下准备：    
1. ESP32设备，可以是开发板或者你自己设计的PCB，需要支持TF卡（4线SPI）。
2. 一张SD卡，推荐闪迪，可以自行搜索esp设备对tf卡的兼容性。
3. 一个SSD1306显示屏（可选的）
4. 一个已经可以正常运行的Hexo博客（可以是Github Page或Gitee或任何其他形式），需要拥有基本的如何部署Hexo博客的知识，本说明不包含Hexo博客的部署教程，请自行了解如何部署和生成Hexo网页。
5. 如果你需要外网访问你的设备，请确保你的网络环境支持ipv4或ipv6任何一种公网，ipv6现在很容易获取，如果你不确定自己是否拥有ipv6，可以从这个网站测试：[IPV6连接性测试](https://test-ipv6.com/)。

## 推荐前置知识学习
如果你对本篇文章在此之前的任何名词感到疑惑，或许可以从下面的链接中获取一些帮助：    
https://hexo.io/zh-cn/docs/    
https://zhuanlan.zhihu.com/p/391202765    
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html    

# 🚀快速开始
下载Release界面的Esp32固件，并使用烧录工具（ESP固件烧录工具）进行固件烧录