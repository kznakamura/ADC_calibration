# Name
ADC_calibration

## Author
K. Z. Nakamura

## Overview
FEBのADCHとADCLのゲインの比を求めるための解析プログラム。
使い方の詳細はwikiにまとめてある。

## Install
`git clone git@akashi://home/git_repo/ADC_calibration`

## 実行手順
- mainのプログラムはexamples以下に保存されている
- プログラム名を確認して、以下のコマンドでmakeする
 - `make TARGET=filename(w/o .cc)`
 - bin以下に実行ファイルが生成される
 - 解析したいディレクトリに移動して、実行ファイルのシンボリックファイルを生成する
- 実行ファイルを全消去したい場合は以下のコマンドを実行する
 - ` make clean`