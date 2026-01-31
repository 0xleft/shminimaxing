# shminimaxing

Built in collaboration with [@FBPinguin](https://github.com/FBPinguin)

## Run tests

```bash
./run_tests.sh
```

## Build

```bash
cmake . DJAVA_HOME=<your java home>
cmake --build . --target shminimaxing --config Release
```

## Usage

Call `getBestMove` from in java. Make sure you have the library loaded using `System.loadLibrary("shminimaxing")`.

```java
/**
 * The function assumes that the current state is where next expected move is a {@link PlacementMove} type.
 *
 * @param boardState     the 80 bit serialization of a board
 * @param selectionState the 16 bit selection state of the board
 * @param timeLeft       how much time is left to evaluate this position in milliseconds (this doesn't
 *                       guarantee that it will be done in that amount, as it could return quicker)
 * @return the upper 4 bits being the move and the lower 4 bits being the selection mvoe
 */
public native char getBestMove(char[] boardState, char selectionState, int selectedPiece, int timeLeft);
```

## Little details

We compute all of the symetries 384 (piece symetries) * 32 (board symetries) = 12288, therefore we reduce our search emencely.

Board representation is 80 bits (board) + 16 bits (selection state) + 8 bits for current move thats being chosen.

We experimented greatly with different K parameters for monte carlo however intrestingly sqrt of 2 as per wikipedia seemed to work the best.

During the tournament we noticed that our monte carlo was blundering in certain cases, this also happened to one other group, which means using monte carlo for quarto was not good idea, but we learned a lot and we used it during the tournament so fuck it we ball

## Performance

Both monte carlo and minimax searches are multi threaded up to 16 threads. Monte carlo shares one tree for all the threads, which makes it slower due to locking, however this allows for way deeper search of the tree.

Full game (meaning it will return the true best move) solution in ~3 seconds at 7 pieces on the board, and ~20 seconds for 6 pieces on the board.
