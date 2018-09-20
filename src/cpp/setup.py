#!/usr/bin/env python3

from distutils.core import setup, Extension

extra_compile_args=["-Ofast"]
sources = [
	"attack.c",
	"bitboards.c",
	"board.c",
	"data.c",
	"evaluation.c",
	"hashkeys.c",
	"init.c",
	"io.c",
	"main.c",
	"makemove.c",
	"misc.c",
	"movegen.c",
	"perft.c",
	"validate.c",
	"pvtable.c"
]
setup(
	name = "chess_in_c",
	version = "1.1",
	ext_modules = [Extension("chess_in_c", sources=sources, extra_compile_args=extra_compile_args)]
	);