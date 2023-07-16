# CSteamRestarter
使用 ChatGPT 乱搞的 C 语言版本 SteamRestarter

## 下载
[Release](https://github.com/SummonHIM/CSteamRestarter/releases/latest)

## 编译
### Mingw
```
gcc -fdiagnostics-color=always -static csteamrestarter.c -o Output\CSteamRestarter.exe -lintl -liconv
```
### MSVC
你需要安装 GnuWin32 Gettext 来编译
```
cl.exe /Zi /EHsc /nologo /FeOutput\CSteamRestarter.exe /MT csteamrestarter.c -I Path\to\gettext\include -link Path\to\gettext\libintl.lib
```

## Bug
1. 使用 Mingw 编译出来 Gettext部分中文无法显示
2. 使用 MSVC 编译出来 fprintf 无法写入内容到配置文件