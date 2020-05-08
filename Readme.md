# half Visualizer
[English](https://translate.google.com/translate?sl=ja&tl=en&u=https://github.com/i-saint/halfVisualizer)

![](https://user-images.githubusercontent.com/1488611/81386573-2d33c480-9150-11ea-8ae2-bc52905f11fb.png)  
half や unorm8 のようなビット数を落とした float 型を、Visual Studio のデバッガ上で float で表示＆編集できるようにする拡張機能です。
Visual Studio 2019, 2017 で動作を確認しています。


## 使い方
[halfVisualizer.vsix](https://github.com/i-saint/halfVisualizer/releases/download/1.0.0/halfVisualizer.vsix) をインストール。[half.h](Sources/half.h) をお手元のプロジェクトにインクルード。
以後、half.h 内の型がデバッガ上で float 表示になります。  
デバッガ上で変数を編集する際、通常は入力値を float として扱い、変換を経た値を内部値として適用します (例: 1.00 -> 0x3c00)。ただし、"0xff" のような hex 表記の数値の場合は変換を経由せずそれを直接適用します。

お手元のプロジェクト内に half などの型が既に用意されている場合、本プロジェクトをカスタマイズして独自にパッケージを作り、それをインストールすることをおすすめいたします。  
ビルドには Visual Studio 拡張開発環境が必要です。Visual Studio Installer を起動し、"Modify" を選び、下の方にある "Visual Studio extension development" にチェックを入れて適用します。  
Sources/halfVisualizer.sln を開き、**Platform を Win32 に変更して** ビルドすることでパッケージの作成まで完了します。
デバッグ実行すると、このパッケージだけがインストールされた状態の Visual Studio が起動し、テストプロジェクトを開きます。

カスタマイズする際は以下の点に留意ください。
- [half.h](Sources/half.h)、[halfVisualizer.h](Source/halfVisualizer/halfVisualizer.h)、[halfVisualizer.cpp](Source/halfVisualizer/halfVisualizer.cpp) が halh <-> 文字列 の変換を担う部分です。
- [halfVisualizer.natvis](Source/halfVisualizer/halfVisualizer.natvis) が C++ 上の型との結びつけを担っています。
  Type Name="half" を Type Name="myproject::half" に変えると myproject ネームスペース内の half に表示を適用します。
- [halfVisualizer.vsdconfigxml](Source/halfVisualizer/halfVisualizer.vsdconfigxml) がデバッガ上の型の定義を担っています。
  型の追加を行いたい場合、このファイルに定義を追加します。
- [source.extension.vsixmanifest](Source/vsix/source.extension.vsixmanifest) がパッケージに関する詳細です。
  カスタマイズする際は Identity Id を適当に変えておくことをおすすめいたします。これが同じ拡張は同一のものとみなされ、インストールの際上書きされます。
- 拡張機能のデバッグには下準備が必要です。Visual Studio 2019 以降、IDE とデバッガが別プロセスになっており、デバッガの方にアタッチする必要があります。
  - [ChildProcessDebuggingPowerTool](https://marketplace.visualstudio.com/items?itemName=vsdbgplat.MicrosoftChildProcessDebuggingPowerTool) をインストール。
  - DEBUG -> Other Debug Targets -> Child Process Debugging Settings を選択。
  - Process Name に "msvsmon.exe" を追加。これがデバッガのプロセスになります。
  - All other processes の Action を "Do not debug" に変更。こうしておかないとデバッグ実行中の IDE がデバッグ実行できなくなってしまいます。

Visual Studio の拡張機能に関する詳細は https://github.com/microsoft/ConcordExtensibilitySamples のソース、および wiki が参考になるでしょう。

## ライセンス
- [MIT](LICENSE.txt)