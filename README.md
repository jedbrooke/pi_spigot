# Pi On Golem
## a distributed program for computing digits of PI

### How does it work?
The plan is to accomplish something similar to [Pi-Hex](https://en.wikipedia.org/wiki/PiHex), using the distributed [Golem Network](https://golem.network).

## The goal:
still deciding on the specifics, but seeing as Pi-Hex was able to get to the 1 Quadrillionth bit all the way back in the year 2000, with how much faster computers have become in the past 20 years, we are aiming to go for something much greater.


## TODO:
 - [x] figure out why it breaks after around 500mil
 - [x] implement basic [BBP formula](https://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula)
 - [ ] look into implementing [Bellard's formula](https://en.wikipedia.org/wiki/Bellard%27s_formula)
 - [x] implement custom 64bit fractional types
    - [x] experiment with *faster* 64bit division
 - [x] **implement *enhanced* arbitrary depth fractional types for more precision**
    - [x] addition (see this [gist](https://gist.github.com/jedbrooke/5b3fbfe9a458e8d7d4272391b4d71597))
    - [x] multiplication
    - [x] division (based on multiplication with nr/gs)
    - [x] (optional side quest) experiment with generalising to arbitrary bit debth
 - [ ] parallelize with multi-threading
   - [ ] (optional side quest) gpu accelerated version
 - [ ] speed up with `avx`
 - [ ] distribute over many systems with Golem's `yapapi`



