<div align="center" style="width: 300px; height: 300px; border-radius: 50%; overflow: hidden;">
  <img src="https://github.com/user-attachments/assets/55a93dd1-7479-477b-94b2-f9dcd234c1e9" width="300" height="300" style="display: block; object-fit: contain;">
</div>

<h4 align="center">Strong UCI chess engine.</h4>

## Strength

| Version | [CCRL 40/15][ccrl-4015] | [CCRL Blitz][ccrl-blitz] | [UBC][ubc] |
|:-------:|:-----------------------:|:------------------------:|:----------:|
| [2.0.0] |            -            |            -             |     -      |
| [1.0.0] |           2629          |            -             |    2719    |
---

## Search

The search function explores possible moves and evaluates their outcomes to find the best one. Core techniques include:

- Negamax with Fail-Soft Framework
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
- Singular Extensions
- Razoring
- Cut Node
- Quiescence Search
- Move Ordering
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
$ git clone [https://github.com/ProgramciDusunur/Potential.git](https://github.com/ProgramciDusunur/Potential.git)
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
- [**Shawn**](https://github.com/xu-shawn)
- [**Dan**](https://github.com/kelseyde)
---





[spcc]: https://www.sp-cc.de/
[ccrl-4015]: https://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[ccrl-blitz]: https://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[cegt-404]: http://www.cegt.net/40_4_Ratinglist/40_4_single/rangliste.html
[cegt-4020]: http://www.cegt.net/40_40%20Rating%20List/40_40%20All%20Versions/rangliste.html
[mcerl]: https://www.chessengeria.eu/mcerl
[ubc]: https://e4e6.com/


