# PC Remocon (赤外線リモコンでPCの電源ボタンを操作する)

## 概要

PICマイコン(PIC16F18424)を使って赤外線リモコンでPCの電源ボタンを操作できるようにします。
PCのマザーボードから出ているPWR SWとPWR LED信号を使用し、電源LEDの状態を見ながら電源ボタン操作をします。
これにより意図しない電源オンオフが起こらないようにしています。
操作時にはLEDが点灯しブザーが鳴ります。

## ビルド・デバッグ環境

下記のツールを使用します:
+ [Visual Studio Code](https://code.visualstudio.com/)
 \+ [MPLAB Extension Pack](https://marketplace.visualstudio.com/items?itemName=Microchip.mplab-extension-pack)
+ [MPLAB XC8 Compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc8)
+ [MPLAB Snap](https://www.microchip.com/en-us/development-tool/pg164100)
+ [MPLAB Integrated Programming Environment(IPE)](https://www.microchip.com/en-us/tools-resources/production/mplab-integrated-programming-environment)[^A]

※ MPLAB Code Configurator(MCC)は使用してません
※ Windows環境で開発

## 赤外線リモコン

赤外線リモコンには[Nature Remo Nano](https://shop.nature.global/products/nature-remo-nano)を使用します。

本ソースでは照明のプリセット「NEC LIGHT 201」を使用します。
各ボタンは対応は下記の通りです:
|ボタン|コード|PC Remoconでの機能|
|---|---|---|
|OFF|82 6d be 41|電源オフ|
|お気に入り|82 6d bd 42||
|常夜灯|82 6d bc 43|電源ボタン長押し(約12秒)|
|-|82 6d bb 44||
|+|82 6d ba 45||
|全灯|82 6d a6 59|電源オン|

※ ONボタンは最後に押したボタンによりコードが異なる (お気に入り or 全灯)

## 主要部品

||型番|メーカー|備考|
|---|---|---|---|
|PICマイコン|[PIC16F18424](https://akizukidenshi.com/catalog/g/g116267/)|Microchip||
|赤外線リモコン受信モジュール|[GP1UXC41QS](https://akizukidenshi.com/catalog/g/g106487/)|シャープ||
|2回路入フォトカプラー|[PS2501-2X](https://akizukidenshi.com/catalog/g/g130310/)|Isocom Components||
|圧電スピーカー|[PKM13EPYH4000-A0](https://akizukidenshi.com/catalog/g/g104118/)|村田製作所||

+ 主要部品はすべて[秋月電子通商](https://akizukidenshi.com)で入手可能です(2025/07/21現在)。型番のリンクは秋月電子通商の商品ページへのリンクです
+ その他、基板、LED、ICソケット、コンデンサー、抵抗、ヘッダーピンが必要です

## ピン割り当て

|ピン番号|ピン名|ピン機能|接続先||
|---|---|---|---|---|
|1|VDD|VDD|||
|2|RA5|NCO1OUT|圧電スピーカー||
|3|RA4||||
|4|MCLR#/RA3|MCLR#|MPLAB Snap||
|5|RC5||||
|6|RC4||||
|7|RC3|RC3|LED||
|8|RC2||||
|9|RC1|SMT1SIG|赤外線リモコン受信モジュール||
|10|RC0|RC0|フォトカプラ|入力: PWR LED|
|11|RA2|RA2|フォトカプラ|出力: PWR SW|
|12|RA1/ICSPCLK|ICSPCLK|MPLAB Snap||
|13|RA0/ICSPDAT|ICSPDAT|MPLAB Snap||
|14|VSS|VSS|||

## 参考回路図

+ [schematic.pdf](files/schematic.pdf)

## 参考文献

+ [赤外線リモコンの通信フォーマット](https://elm-chan.org/docs/ir_format.html)

+ [RL78/G23 リモコン信号受信 (NEC フォーマット、SNOOZE モード使用)](https://www.renesas.com/ja/document/apn/rl78g23-remote-control-signal-reception-nec-format-snooze-mode)

+ [PIC16F18424](https://www.microchip.com/en-us/product/pic16f18424)

+ [PIC16(L)F18424/44 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/40002000B.pdf)

+ [MPLAB® XC8 C Compiler User’s Guide for PIC® MCU](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/UserGuides/MPLAB-XC8-C-Compiler-Users-Guide-for-PIC-50002737.pdf)


[^A]: MPLAB IDEのインストールでIPEだけをインストール可能
