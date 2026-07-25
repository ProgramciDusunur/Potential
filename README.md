<div align="center" style="width: 300px; height: 300px; border-radius: 50%; overflow: hidden;">
  <img src="https://github.com/user-attachments/assets/55a93dd1-7479-477b-94b2-f9dcd234c1e9" width="300" height="300" style="display: block; object-fit: contain;">
</div>

<h4 align="center">World's strongest PSQT / Material-only UCI chess engine.</h4>

## Strength

| Version    | [CCRL 40/15][ccrl-4015] | [CCRL Blitz][ccrl-blitz] | [UBC][ubc] |
|:----------:|:-----------------------:|:------------------------:|:----------:|
| [Unlocked] |          ----           |           ----           |    ----    |
|  [3.0.0]   |          3083           |           3056           |    ----    |
|  [2.0.0]   |          2889           |           2923           |    2966    |
|  [1.0.0]   |          2641           |           ----           |    2719    |
---

## Evaluation

- Material
- Piece-Square Tables (PSQT / Selfgen)

---

## Search

The search function explores possible moves and evaluates their outcomes to find the best one. Core techniques include:

- Negamax with Fail-Soft Framework

- Principal Variation Search

- Iterative Deepening

- Aspiration Windows

- Transposition Table

- Static Exchange Evaluation

- Null Move Pruning<br>➔ Eval Margin  <br>➔ Depth-based Reduction<br>➔ Eval-based Reduction<br>➔ Verification Search<br>➔ Refutation Move History Bonus

- Late Move Reduction<br>➔ Cut Node LMR<br>➔ TT PV Fail Low LMR<br>➔ TT Capture LMR<br>➔ Good Eval LMR<br>➔ Improving LMR<br>➔ Quiet Non-PV LMR<br>➔ Futility LMR<br>➔ Quiet History LMR<br>➔ Pawn History LMR<br>➔ Capture History LMR<br>➔ TT PV LMR<br>➔ Gives Check LMR<br>➔ Dynamic Helper Thread Reduction Bias

- Late Move Pruning<br>➔ History-based Threshold<br>➔ Improving Threshold

- ProbCut<br>➔ Improving Margin<br>➔ SEE Threshold<br>➔ Noisy Futility Margin<br>➔ Capture History Margin<br>➔ Cut Node Scalar

- Reverse Futility Pruning<br>➔ Improving RFP<br>➔ Quadratic Depth Margin<br>➔ TT PV RFP Decision<br>➔ Corrplexity RFP

- Razoring

- Singular Extensions<br>➔ Double Extension<br>➔ Triple Extension<br>➔ Quadruple Extension<br>➔ Multi Low Depth Extension<br>➔ Multicut<br>➔ Negative Extensions<br>➔ Cut Node Extension<br>➔ Recapture Extension

- Low Depth Singular Extensions<br>➔ Correction Based Margin

- Futility Pruning<br>➔ History-based Margin<br>➔ Quadratic Depth Margin<br>➔ Offset Margin

- Move Ordering<br>➔ TT Move<br>➔ Static Exchange Evaluation (SEE)<br>➔ Quiet History<br>➔ Pawn History<br>➔ Continuation History<br>➔ Capture History

- SEE PVS Pruning

- Quiet History Pruning

- Bad Noisy Futility Pruning<br>➔ Quadratic Depth Margin

- Small Probcut

---

## Time Management

- Hard Limit
- Soft Limit
- Complexity TM
- Node Limits

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
- [**Shawn**](https://github.com/xu-shawn)
- [**Dan**](https://github.com/kelseyde)
- [**Swedishchef**](https://github.com/JonathanHallstrom)
---





[spcc]: https://www.sp-cc.de/
[ccrl-4015]: https://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[ccrl-blitz]: https://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[cegt-404]: http://www.cegt.net/40_4_Ratinglist/40_4_single/rangliste.html
[cegt-4020]: http://www.cegt.net/40_40%20Rating%20List/40_40%20All%20Versions/rangliste.html
[mcerl]: https://www.chessengeria.eu/mcerl
[ubc]: https://e4e6.com/


