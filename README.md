# LifeLua
LifeLua is a Lua interpreter for the PSVita which combines ease (OneLua) with advanceability (Lua Player Plus).

<img src="bg0.png"></img>

## Documentation
https://harommelrabbid.github.io/LifeLua
## Samples
For a showcase of LifeLua's features go to the repository's `sample` folder.
## Compiling
* [libsqlite](https://github.com/VitaSmith/libsqlite): run `cd libsqlite && make`, move the library (ends with` *.a`) in the folder where the libraries are stored in the vitasdk, and run `make install`, see https://github.com/VitaSmith/libsqlite?tab=readme-ov-file#compiling
* Install [vitasdk](https://github.com/vitasdk) if you haven't and build LifeLua using:

```
mkdir build && cd build && cmake .. && make
```

To make after your first build:

```
find . -mindepth 1 -delete && cmake .. && make
```

## To do
* ATRAC9 & 3 audio support & libmpv video support
* 3D support with shading, shadows & reflections
* More shape drawing functions, such as drawing arches and gradient lines
* Adhoc & socket support, and maybe PSN support as well
* SHA512 support
* Fix the thread library (it's kind of unstable, some functions as a thread may crash the app, depending on how heavy the function is)
* USB support (maybe)
## Credits
* HENkaku by Team Molecule
* TheFloW's VitaShell for SHA1 hashing
* vitasdk contributors
* [quirc-vita](https://github.com/cxziaho/quirc-vita) by [cxziaho](https://github.com/cxziaho)
* [QR-Code-generator](https://github.com/nayuki/QR-Code-generator) by [nayuki](https://github.com/nayuki)
* libvita2d & ftpvita by xerpi
* Inspiration from [Lua Player Plus Vita](https://github.com/Rinnegatamante/lpp-vita) by [Rinnegatamante](https://github.com/Rinnegatamante)
* [luautf8](https://github.com/starwing/luautf8) by [starwing](https://github.com/starwing)
<!--
* [Compound Assignment Operators](http://lua-users.org/files/wiki_insecure/power_patches/5.2/compound-5.2.2.patch) (Lua diff patch) by [SvenOlsen](http://lua-users.org/wiki/SvenOlsen)
* [Bitwise operators, integer division and !=](http://lua-users.org/files/wiki_insecure/power_patches/5.1/bitwise_operators_5.1.4_1.patch) by Thierry Grellier, darkmist(at)mail.ru & Joshua Simmons
* [Continue Statement](https://lua-users.org/files/wiki_insecure/power_patches/5.1/continue-5.1.3.patch) by Leszek Buczkowski, Wolfgang Oertl & [AskoKauppi](https://lua-users.org/wiki/AskoKauppi)
-->
