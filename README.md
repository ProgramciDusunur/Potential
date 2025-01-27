<h1 align="center">Potential</h1>

<h3 align="center">Decent UCI chess engine.</h3>




## Strength


| Version  |      [SP-CC UHO-Top15][spcc]       | [CCRL 40/15][ccrl-4015] | [CCRL Blitz][ccrl-blitz] | [CEGT 40/4][cegt-404] | [CEGT 40/20][cegt-4020] | [MCERL] |
|:--------:|:----------------------------------:|:-----------------------:|:------------------------:|:---------------------:|:-----------------------:|:-------:|
| [1.0.0]  |                 -                  |            -            |            -             |           -           |            -            |    -    |



## Evaluation

The evaluation function determines how good or bad a position is for each side. Key evaluation aspects include:  

- Material Balance  
- Mobility
- King Safety
- King Distance
- Tempo
- Check Tempo
- Winnable
- Can Passer Pawn Move
- King Distance
- Rook Open File
- Rook Semi Open File
- Pawn Structure  
  ### Key Factors in Pawn Structure:
  - Passed Pawns  
  - Isolated Pawns 🚧  
  - Doubled Pawns 🚧  
  - Backward Pawns 🚧  
  - Pawn Chains 🚧  

- Tapered Eval (Opening, Middlegame, Endgame)

---

## Search

The search function explores possible moves and evaluates their outcomes to find the best one. Core techniques include:  

- Negamax with Fail-Hard Framework
- Principle Variation Search
- Iterative Deepening 
- Transposition Table (TT)
- Aspiration Windows
- Late Move Pruning
- Mate Distance Pruning
- Late Move Reduction
- Null Move Pruning
- Futility Pruning
- Reverse Futility Pruning
- Quiescence SEE Pruning
- PVS SEE Pruning
- Improving Heuristic
- Razoring
- Cut Node
- Quiescence Search
- Move Ordering  
  ### Priorities in Move Ordering:
    When searching for the best move, the engine searches moves in the following order:
  
  1. Transposition Table Move
  2. Principle Variation Moves
  3. SEE Move Ordering (Capture Moves)
  4. MVV/LVA (Capture Moves)
  5. Quiet History Moves
---

## Time Control

- **Hard Limit**
- **Soft Limit**
- **Best Move TM**
- **Evaluation TM**

---

## Installation & Usage

### Running The Engine

#### Linux
```bash
$ git clone https://github.com/ProgramciDusunur/Potential.git
$ cd Potential/src
$ make
$ ./Potential
```
---

## **Credits**

This project has been shaped and inspired by the valuable support and thoughtful feedback of the following individuals. Your contributions and insights have been greatly appreciated—thank you! 🌟

- [**Ciekce**](https://github.com/Ciekce)
- [**rwbc**](https://github.com/rwbc)
- [**Dark Neutrino**](https://github.com/Haxk20)
- [**Zuppa**](https://github.com/PGG106)
---





[spcc]: https://www.sp-cc.de/
[ccrl-4015]: https://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[ccrl-blitz]: https://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[cegt-404]: http://www.cegt.net/40_4_Ratinglist/40_4_single/rangliste.html
[cegt-4020]: http://www.cegt.net/40_40%20Rating%20List/40_40%20All%20Versions/rangliste.html
[mcerl]: https://www.chessengeria.eu/mcerl


