# ThreadedThinker
ThreadedThinker is a simple chess engine written in C programming language. It is UCI compatible that means it also implements UCI Protocol which establish 
connection between third party gui application ( Arena ) and chess engine. 

# Demonstration
https://github.com/Max-Patil/ThreadedThinker/assets/151635173/c07f749a-f1ac-405f-98d1-396dcc2e152c

# Features 
It implemented core features with optimization techniques listed below
* Alpha-Beta Pruning: This technique helps in significantly reducing the number of nodes evaluated in the game tree, by pruning away branches that cannot
  possibly affect the final decision.
  
* Iterative Deepening: This approach involves repeatedly deepening the search tree and using the results of previous shallower searches to improve the
  efficiency of deeper ones.

* Quiescence Search: To avoid the horizon effect, this method extends the search in positions with potential significant changes (like captures or checks),
  ensuring that the engine evaluates only relatively 'quiet' positions.

* Move Ordering: By evaluating the most promising moves first, this technique increases the efficiency of the Alpha-Beta pruning, as better moves are likely
  to be considered earlier, leading to earlier pruning.

* Null Move Pruning is a chess engine technique that skips a player's turn to quickly determine if the position is already advantageous. If the position
  remains strong even without making a move, further analysis of that branch is skipped, speeding up the search process.

* Transposition Table: This is a memory optimization that stores previously evaluated positions. When the same position is reached again, the engine can use
  the stored evaluation instead of recalculating it.

* Static Evaluation Function: This function assesses a position without delving deeper into potential future moves. It usually includes material count,
  piece positions, control of the center, pawn structure, king safety, etc.

# Acknowledgement & References
* [ Bluefever Software's YouTube Series ]( https://www.youtube.com/watch?v=bGAfaepBco4&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg ) - This series really helped me to build chess engine from scratch and it also inspired others to build their own chess engine.
* [ Chess Programming Wiki ]( https://www.chessprogramming.org/Main_Page ) - It is extensive resource to become familiar with chess programming concepts in detail. 
* Chess Algorithm Book - The author has thoroughly explained algorithms and offered practical insight. 
* [ Chess Programming Archive  ]( https://web.archive.org/web/20070811182741/www.seanet.com/~brucemo/topics/topics.htm ) - This archive also helped me to understand and implement core concepts.


