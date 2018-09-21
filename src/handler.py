import json
from chess import Board
import subprocess as s
from pathlib import Path
from src.opening import opening_book_next_move
from src.endgame import query_end_game_move, is_endgame

def next_move(event, context):
    # get data from http post request
    fen = json.loads(event['body']).get('fen')
    move_count = event.get('pathParameters', {}).get('move_count')

    # some error handling
    if move_count is None:
        response = {
            "statusCode": 422,
            "body": "missing move_count in move path",
            "headers": {
                "Access-Control-Allow-Origin": "*"
            }
        }
        print(response['body'])
        return response
    elif fen is None:
        response = {
            "statusCode": 422,
            "body": "missing fen in the post body",
            "headers": {
                "Access-Control-Allow-Origin": "*"
            }
        }
        print(response['body'])
        return response

    # the main logic for getting next move
    move_count = int(move_count) // 2
    if move_count <= 9:
        move = opening_book_next_move(fen, 'performance.bin')
        if move is not None:
            return {
                "statusCode": 200,
                "body": json.dumps({'move': move}),
                "headers": {
                    "Access-Control-Allow-Origin": "*"
                }
            }

    b = Board(fen)
    if is_endgame(b):
        move = query_end_game_move(b.fen())
        if move != None:
            return {
                "statusCode": 200,
                "body": json.dumps({'move': move}),
                "headers": {
                    "Access-Control-Allow-Origin": "*"
                }
            }

    if b.turn:
        side = '0'  # white
    else:
        side = '1'

    # my_file = Path("./src/cpp/main")
    # if not my_file.is_file():   # our binary is not there, we should compile it first
    #     print("Compiling the executable")
    #     s.run(["make", "-C", "./src/cpp/"], stdout=s.PIPE)

    cmd_result = s.run(['./src/cpp/main', fen ,side], stdout=s.PIPE)
    move = cmd_result.stdout.decode('utf-8').split("\n")[-2]
    return {
        "statusCode": 200,
        "body": json.dumps({'move': move}),
        "headers": {
            "Access-Control-Allow-Origin": "*"
        }
    }
