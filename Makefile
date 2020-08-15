build:
	@clang++ -std=c++11 mjs.cc -o mjs

run:
	@make build && ./mjs a.js