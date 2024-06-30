| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

<p align="center">
  <img src="https://raw.githubusercontent.com/isNineSun/img_repository/main/e057386c0df403c28fb037c34386544a.png" height=120>
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
- [如何使用](#如何使用)
  - [🚀快速开始](#快速开始)
  - [🤓进阶配置](#进阶配置)
    - [关于DDNS配置](#关于ddns配置)
    - [关于显示屏幕](#关于显示屏幕)
    - [完整配置](#完整配置)
  

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

# 如何使用
## 🚀快速开始
1. 下载espressif官方烧录工具：[flash_download_tool](https://www.espressif.com.cn/zh-hans/support/download/other-tools)    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/cb03ae14ec703d0c0ad7c6134b880564.png)    

2. 打开工具，选择对应的芯片型号，点击OK进入烧录界面    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/4eea5349f86e8d97c06a94ffb8a5ef1a.png)    

3. 下载Release界面的[Esp32固件](https://github.com/isNineSun/ESPHexo/releases/download/ESPHexo_V0.01/target.bin)，并使用烧录工具（ESP固件烧录工具）进行固件烧录，如果遇到点击Start之后等了一会而不开始下载的情况，可以给设备重启或重新上电。    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/ed2ab93b9c121e27800154b9f84c8417.png)    

4. 准备好配置文件，新建一个文本文件并命名为``bloxer_config.json``,按照以下模板进行编辑：
   ```json
    {
        "wifi_ssid":	"You WIFI SSID",
        "wifi_password":	"You WIFI Password",
        "ddns_domain":	"Your Domain",
        "ddns_token":	"Your token",
        "web_username":	"admin",
        "web_password":	"123456",
        "repo_owner": "You Repository Username",
        "repo_name": "You Repository Name"
    }
   ```
   - wifi_ssid： wifi的名称
   - wifi_password： WiFi密码
   - ddns_domain：DDNS域名，**对于快速启动来说，这一项可以不设置留空**
   - ddns_token：DDNS 服务token，**对于快速启动来说，这一项可以不设置留空**
   - web_username：WebUI配置界面的用户名，**对于快速启动来说，这一项可以不设置留空**
   - web_password：WebUI配置界面的密码，**对于快速启动来说，这一项可以不设置留空**
   - repo_owner：保存博客文件的仓库的拥有者的账户名（比如，部署在github上，那么就是你的github名），**对于快速启动来说，这一项可以不设置留空**
   - repo_name：保存博客文件的仓库名，**对于快速启动来说，这一项可以不设置留空**    
对于快速启动来说，其配置文件可以类似这样：    
```json
    {
        "wifi_ssid":	"123456",
        "wifi_password":	"1234567890",
        "ddns_domain":	"",
        "ddns_token":	"",
        "web_username":	"",
        "web_password":	"",
        "repo_owner": "",
        "repo_name": ""
    }
```
5. 将你本地已经存在的Hexo博客文件夹中的整个Public文件夹与``bloxer_config.json``一起放入SD卡，文件结构如图，并将SD卡插入设备，设备开机并等待设备初始化完成，显示屏应该会亮起（如果装了显示屏的话）。    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/af6507febec5c43988320a4935235951.png)    

6. 在浏览器地址栏输入显示屏上的IP地址，如：192.168.31.177，如果一切正常。就可以直接本地访问设备中的Blog网站了。    

## 🤓进阶配置
### 关于DDNS配置
首先，需要你的网络环境支持公网IP访问，IPV4(未进行测试)或者IPV6，然后在[Duck DNS](https://www.duckdns.org/domains)上免费申请一个域名，登录选项有很多，直接Github即可，也可以选择自己喜欢的，申请过后，将你的域名和账户的token配置记录下来，在ESPHexo的配置文件中即可。    
![](https://raw.githubusercontent.com/isNineSun/img_repository/main/5edce0.png)    

### 关于显示屏幕
显示屏幕是可选的，即使你的硬件上不支持显示屏，对程序运行也不会有影响，如果你想更干净地杜绝显示屏功能，可以关闭代码中的显示屏相关功能的编译宏控，其Makfile文件路径为``/根目录/CMakeLists.txt``：    

![](https://raw.githubusercontent.com/isNineSun/img_repository/main/1fb5ab5da8a03928e6a642b9e0dcf4ce.png)    

### 完整配置
1. 完整配置中包括对Blog文件的**自动拉取，DDNS与WebUI**，首先需要准备好配置文件：
   ```json
    {
        "wifi_ssid":	"You WIFI SSID",
        "wifi_password":	"You WIFI Password",
        "ddns_domain":	"Your Domain",
        "ddns_token":	"Your token",
        "web_username":	"admin",
        "web_password":	"123456",
        "repo_owner": "You Repository Username",
        "repo_name": "You Repository Name"
    }
   ```
将上文提到的DDNS的域名和账户的token，还有保存有你Hexo博客文件的Github仓库或者Gitee仓库的所有者，全部填写进去，关于web_username和web_password，则可以自己随便自定义，这个是你的WebUI配置界面的账户和密码。

2. 配置完成后，可以在SD卡中只放置这一个配置文件，其他所有文件都不需要（包括上文快速启动中提到的Blog的Public文件夹）。    
3. 将``bloxer_config.json``放入SD卡，并将SD卡插入设备，设备开机并等待设备初始化完成，显示屏应该会亮起（如果装了显示屏的话）。    
    ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/8bb7c72db3df0f3907b5075f1bc293b4.png)    

4. 在浏览器地址栏输入显示屏上的IP地址，如：192.168.31.177/Dashboard，可以直接进入设备的WebUI，输入用户名密码，进入配置界面。      
   **登录界面：**    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/e872c4abeb29cd58dacd1a02fe279db8.png)    
   **配置界面：**    
   ![](https://raw.githubusercontent.com/isNineSun/img_repository/main/c6d7ca031298882a3bb9f6e132679a6b.png)    

5.  在此可以再检查一下之前配置文件做的配置，确认无误后，点击``Update Bolg Files form Server``按钮，设备会自动开始下载博客仓库中的文件，显示屏会展示下载进度，**待下载完成后，在浏览器地址栏输入设备地址 192.168.31.177，即可打开博客网站**。 

6. 外网访问需要用到直接在Duck DNS上申请的域名，比如，你申请的域名是isbloxer，那么直接访问```http://isbloxer.duckdns.org/```，即可访问你的博客网站。