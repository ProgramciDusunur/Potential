<h1 align="center">Potential</h1>

<h3 align="center">Decent UCI chess engine.</h3>

---

## Evaluation

The evaluation function determines how good or bad a position is for each side. Key evaluation aspects include:  

- Material Balance  
- Mobility
- King Safety
- Tempo
- Check Tempo (If we are in a check then our tempo bonus cancelled.)
- Winnable
- Can Passer Pawn Move
- King Distance
- Rook Open File
- Rook Semi Open File
- Pawn Structure  
  ### Key Factors in Pawn Structure:
  - Passed Pawns 🚧  
  - Isolated Pawns 🚧  
  - Doubled Pawns 🚧  
  - Backward Pawns 🚧  
  - Pawn Chains 🚧  

- Tapered Eval (Opening, Middlegame, Endgame)

---

## Search

The search function explores possible moves and evaluates their outcomes to find the best one. Core techniques include:  

- Negamax with Fail-Hard Framework
- Iterative Deepening 
- Transposition Table (TT)
- Aspiration Windows
- Late Move Pruning
- Mate Distance Pruning
- Late Move Reduction
- Null Move Pruning
- Futility Pruning (Quiet Moves)
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

### Running the Engine
```bash
$ git clone https://github.com/ProgramciDusunur/Potential.git
$ cd Potential/src
$ make
$ ./Potential
