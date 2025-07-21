# IR Remocon Analyzer (赤外線リモコン解析)

## 概要

PICマイコン(PIC16F18424)を使った赤外線リモコン解析ツールです。

下記の機能があります:
+ NECフォーマット・家製協フォーマットのコードの解析
+ Hi/Low時間の解析

元々はPCを赤外線リモコンでオンオフするガジェットを作ろうと思っていたのですが、折角ならちゃんと解析できるツールにしてしまおうと思ってこの形になりました。

## ビルド・デバッグ環境

下記のツールを使用します:
+ [Visual Studio Code](https://code.visualstudio.com/)
 \+ [MPLAB Extension Pack](https://marketplace.visualstudio.com/items?itemName=Microchip.mplab-extension-pack)
+ [MPLAB XC8 Compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc8)
+ [MPLAB Snap](https://www.microchip.com/en-us/development-tool/pg164100)
+ [TeraTerm](https://teratermproject.github.io/)
+ [MPLAB Integrated Programming Environment(IPE)](https://www.microchip.com/en-us/tools-resources/production/mplab-integrated-programming-environment)[^A]

※ MPLAB Code Configurator(MCC)は使用してません
※ Windows環境で開発

## 主要部品

||型番|メーカー|備考|
|---|---|---|---|
|PICマイコン|[PIC16F18424](https://akizukidenshi.com/catalog/g/g116267/)|Microchip||
|赤外線リモコン受信モジュール|[GP1UXC41QS](https://akizukidenshi.com/catalog/g/g106487/)|シャープ||
|FT234X 超小型USBシリアル変換モジュール|[AE-FT234X](https://akizukidenshi.com/catalog/g/g108461/)|秋月電子通商|電源供給兼用|
|圧電スピーカー|[PKM13EPYH4000-A0](https://akizukidenshi.com/catalog/g/g104118/)|村田製作所|オプション|

+ 主要部品はすべて[秋月電子通商](https://akizukidenshi.com)で入手可能です(2025/07/21現在)。型番のリンクは秋月電子通商の商品ページへのリンクです
+ その他、基板、ICソケット、コンデンサー、抵抗、ヘッダーピンが必要です

## ピン割り当て

|ピン番号|ピン名|ピン機能|接続先|
|---|---|---|---|
|1|VDD|VDD||
|2|RA5|NCO1OUT|圧電スピーカー|
|3|RA4|RX1|USBシリアル変換モジュール|
|4|MCLR#/RA3|MCLR#|MPLAB Snap|
|5|RC5|||
|6|RC4|||
|7|RC3|||
|8|RC2|TX1/CK1|USBシリアル変換モジュール|
|9|RC1|SMT1SIG|赤外線リモコン受信モジュール|
|10|RC0|||
|11|RA2|||
|12|RA1/ICSPCLK|ICSPCLK|MPLAB Snap|
|13|RA0/ICSPDAT|ICSPDAT|MPLAB Snap|
|14|VSS|VSS||

## 参考回路図

[schematic.pdf](files/schematic.pdf)

## 参考文献

[赤外線リモコンの通信フォーマット](https://elm-chan.org/docs/ir_format.html)
[RL78/G23 リモコン信号受信 (NEC フォーマット、SNOOZE モード使用)](https://www.renesas.com/ja/document/apn/rl78g23-remote-control-signal-reception-nec-format-snooze-mode)
[PIC16F18424](https://www.microchip.com/en-us/product/pic16f18424)
[PIC16(L)F18424/44 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/40002000B.pdf)
[MPLAB® XC8 C Compiler User’s Guide for PIC® MCU](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/UserGuides/MPLAB-XC8-C-Compiler-Users-Guide-for-PIC-50002737.pdf)

[^A]: MPLAB IDEのインストールでIPEだけをインストール可能
