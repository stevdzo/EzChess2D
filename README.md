# EzChess2D

Two-player chess on a single machine. C++ / freeglut / SOIL2, Visual Studio 2022.

## Running

```
EzChess2D.exe                                          normal game
EzChess2D.exe --perft                                  rules check (no window)
EzChess2D.exe --fen "4k3/P7/8/8/8/8/8/4K3 w - - 0 1"   start from a given position
```

The working directory must be `EzChess2D\` (that is where `resources/pieces.png` and
`freeglut.dll` live).

## Controls

| | |
|---|---|
| left click | select a piece / play a move |
| `R` | new game |
| `U` | undo move |
| `ESC` | quit |

## Rules

Complete, including castling, en passant, promotion, checkmate, stalemate, the fifty-move
rule, threefold repetition and insufficient material. Correctness is verified by a perft
test over 6 standard positions with `--perft`.

## Structure

`ChessCore.h/.cpp` is the rules core: it knows nothing about OpenGL, the mouse or pixels,
so it can be tested on its own. Everything else is presentation and input.
