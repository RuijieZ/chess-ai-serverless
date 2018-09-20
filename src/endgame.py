import requests
import json
from chess import Board

base_url = 'http://tablebase.lichess.ovh/standard?fen={}'

def query_end_game_move(fen):
	try:
		url = base_url.format(fen)
		r = requests.get(url)
		res = json.loads(r.text)
		move = res['moves'][0]['uci']
		print('Move returned is {}'.format(move))
		return move
	except Exception as e:
		print(e)
		print('exception raised for query end game database')
		print('return None as result')
		return None


def is_endgame(board):
	return len(board.piece_map()) <= 7



if __name__ == '__main__':
	query_end_game_move('5k2/8/p7/6R1/8/1r3pK1/7P/8 w - - 1 69')
	query_end_game_move('6k1/p4p1p/3Qp3/4P1P1/4PK2/6P1/4q2P/7r b - - 11 41')
	print(is_endgame(Board()))
	print(is_endgame(Board("5k2/8/p7/6R1/8/1r3pK1/7P/8 w - - 1 69")))