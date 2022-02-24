# Pi On Golem
## a distributed program for computing digits of PI

### How does it work?
The plan is to accomplish something similar to [Pi-Hex](https://en.wikipedia.org/wiki/PiHex), using the distributed [Golem Network](https://golem.network).

## The goal:
still deciding on the specifics, but seeing as Pi-Hex was able to get to the 1 Quadrillionth bit all the way back in the year 2000, with how much faster computers have become in the past 20 years, we are aiming to go for something much greater.


## TODO:
 - [x] implement basic [BBP formula](https://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula)
 - [ ] look into implementing [Bellard's formula](https://en.wikipedia.org/wiki/Bellard%27s_formula)
 - [x] implement custom 64bit fractional types
 - [ ] implement *enhanced* 256 bit fractional types for more precision
 - [ ] parallelize with multi-threading
 - [ ] distribute over many systems with Golem's `yapapi`



