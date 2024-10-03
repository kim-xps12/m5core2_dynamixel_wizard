# M5Core2 DYNAMUXEL Wizard

## 概要

[DYNAMIXEL TTL UART Board](https://github.com/kim-xps12/m5stack_board_dynamixel_ttl_rs3485)とM5Stack Core2を用いて[DYNAMIXEL Wizard](https://emanual.robotis.com/docs/en/software/dynamixel/dynamixel_wizard2/)の一部機能を再現します．現時点では以下の機能が実装されています．

- 接続されたサーボのIDとBaudrateをスキャン
- ID / Baudrateの変更
- 位置制御モード / 速度制御モードのサンプル実行

## セットアップ

1. M5Stack Core2へ本リポジトリのコードをビルドして書き込みます
1. [DYNAMIXEL TTL UART Board](https://github.com/kim-xps12/m5stack_board_dynamixel_ttl_rs3485)を用いてサーボを接続します
1. `initializing...`と数秒間表示された後に以下のようなメインメニューが表示されたら完了です
    ```
    DYNAMIXEL Wizard M5Core2!
    Button A: scan
    Button B: ID / Baud
    Button C: Sample Exec
    ```
**注意**
本コードはDYNAMIXEL TTL UART Board **v2.0** を想定しています．
v1.0をお使いの方は，[main.cpp#L9-L10]()のRX, TXピン番号を以下のように入れ替えてからビルドと書き込みを行ってください．（v2.0をお使いの場合は何も編集せずにそのままビルド→書き込みを行ってください）

```
// Pin assign for v1.0 board
const uint8_t PIN_RX_SERVO = 32;
const uint8_t PIN_TX_SERVO = 33;
```

## 機能と使用方法

### サーボのスキャン

メインメニューで`Button A`をタップすると，接続されたサーボのスキャンを行います．
結果の表示画面で再度`Button A`をタップするとメインメニューに戻ります．


### ID/Baudrateの変更

メインメニューで`Button B`をタップすると，接続されたサーボのIDまたはBaudrateの書き換えが行えます．

- Use Baud: サーボが**現在使用しているBaudrate**を指定してください
- Target ID: 書き換え対象のサーボに**現在設定されているID**を指定してください
- New ID: **変更後**に使用したいサーボIDを指定してください
- New Baud: **変更後**に使用したいBaudrateを指定してください

上記の項目の値を設定したら，IDの変更を行う場合には[Apply ID]を，Baudrateの変更を行う場合には[Apply Baud]をタップしてください．

設定の書き込み途中以外で再度`Button B`をタップするとメインメニューに戻ります．


### 位置制御/速度制御サンプルの実行

メインメニューで`Button C`をタップすると，位置制御 / 速度制御のサンプル実行が行えます．

- ID: 接続されたサーボのうち，動作させたいサーボのIDを指定してください
- Baud: 接続されたサーボが使用しているBaudrateを指定してください

上記の項目の値を設定したら，位置制御のサンプル実行を行う場合には[Position Mode]を，速度制御の場合には[Velocity Mode]をタップしてください．

- 位置制御のサンプル実行では，0, 90, 180, 270[deg]への位置制御が行われます
- 速度制御のサンプル実行では，60[rpm], -60[rpm]の速度制御がそれぞれ1秒間行われます

サンプル実行中以外で再度`Button C`をタップするとメインメニューに戻ります．

## 免責事項

MITライセンスでの公開です．本製品の利用によって生じたいかなる損害も責任を負いません．予めご了承ください．