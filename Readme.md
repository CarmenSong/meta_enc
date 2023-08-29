## How to Build
```
 ./make.sh
```

g++, opensslが必要です。

## Usage
```
 ./main -k key.dat -r raw.dat
```

Optionは以下の通り。

　-k (鍵ファイル)  
　-r (raw block deviceのバイナリ)　これを指定していると、復号モードになり、復号した結果を出力します。  
　-d (decrypted deviceのバイナリ)　これを指定していると暗号モードになり、暗号化した結果を出力します。  
　-l (暗復号するサイズ)　デフォルト4096bytes(=1 block)　ブロック長の倍数しか受入れないと思います・・・。  
　-s (開始セクター)　デフォルト0。　デバイスの途中部分を切り出して復号するなら、これを指定する必要があります。  

## Note

-r -dでは、ファイルのopenを想定しているので、  
qemu起動して、デバイスをhexdumpできる状態になったら、  
```
  $ dd if=/dev/dm-0 of=decrypt_data.dat bs=4096 count=256
```
というコマンドとかで、/dev/dm-0の先頭から1MBをファイルとして、ダンプできます。

 
ddの引数で、skipとかを指定すると、任意の部分をdumpできるので、例えば、前の例の0x50c000をdumpしたければ、
```
  $ dd if=/dev/dm-0 of=decrypt_data.dat bs=4096 count=1 skip=1292  (1292=0x50c)
```
とかで、とれるんじゃないかな・・・と（詳しくはddのマニュアル調べて下さい）

