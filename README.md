# NoGo-Sample

根據交大電腦對局的最後的作業 Hollow NoGo 寫的範例。

## 甚麼是 Hollow NoGo？

Hollow NoGo 是一種 [NoGo](https://zh.wikipedia.org/zh-tw/%E4%B8%8D%E5%9C%8D%E6%A3%8B) 的變體，規則和 NoGo 相同，不同之處在於 Hollow NoGo 不是完整的正方形棋盤，棋盤上有些不規則的洞，雙方玩家要根據不同洞的位置選擇不同戰略，相比原本的 NoGo 具有更高的挑戰性。

## 如何使用？

推薦使用圍棋的圖形界面 [sabaki](https://sabaki.yichuanshen.de/) 加載編譯好的引擎，即可和引擎對戰，但需要注意 hollow 的部份不會顯示在棋盤。

## 實做特點

* 實做完整的 MCTS
* 支援多 CPU 核心搜索，能高效利用多個核心。
* 可重複使用樹，以提搜索高效率
* 較好的規則實做，經過對比，稍快於交大作業範例的 bitboard
* 完整的時間控制器，可較好的利用剩餘時間

## TODO

* Rave MCTS
