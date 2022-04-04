# Pi Spigot
## a high performance pi spigot for finding far out digits of pi

this pi spigot uses high performance C++ and inline x86 asm to find far out digits of pi in base 16.

current performance:
```
50 hexits around the 1millionth hexit: 1.5s
```
Usage:
```
./pi_spigot [-f] n [range (default 50)
```
`n` is the starting position to find hexits from, range is how many hexits to find

the `-f` flag will make the program output all hexits from 0 to `n`



## TODO:
 - [x] figure out why it breaks after around 500mil
 - [x] implement basic [BBP formula](https://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula)
 - [ ] look into implementing [Bellard's formula](https://en.wikipedia.org/wiki/Bellard%27s_formula)
 - [x] implement custom 64bit fractional types
    - [x] experiment with *faster* 64bit division
 - [x] **implement *enhanced* arbitrary depth fractional types for more precision**
    - [x] addition
    - [x] multiplication
    - [x] division (based on multiplication with nr/gs)
    - [x] real division with hardware `div` instruction 
    - [x] (optional side quest) experiment with generalising to arbitrary bit debth
 - [ ] parallelize with multi-threading
   - [ ] (optional side quest) gpu accelerated version
 - [ ] speed up with `avx`
 - [ ] distribute over many systems with Golem's `yapapi` (see [POG](https://github.com/jedbrooke/pog))



