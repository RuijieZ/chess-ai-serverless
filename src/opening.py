import chess
from chess import Board
import chess.polyglot
import random

def opening_book_next_move(fen, path):
	b = Board(fen)
	with chess.polyglot.open_reader(path) as reader:
		try:
			entry = reader.choice(b)
			return str(entry.move())
		except IndexError:
			return None