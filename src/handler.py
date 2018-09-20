import json
from chess import Board
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
            "body": "missing move_count in move path"
        }
        print(response['body'])
        return response
    elif fen is None:
        response = {
            "statusCode": 422,
            "body": "missing fen in the post body"
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
                "body": move
            }

    b = Board(fen)
    if is_endgame(b):
        move = query_end_game_move(b.fen())
        if move != None:
            return move

    if b.turn:
        side = '0'  # white
    else:
        side = '1'

    return {
        "statusCode": 200,
        "body": 'none'
    }

    # Use this code if you don't use the http event with the LAMBDA-PROXY
    # integration
    """
    return {
        "message": "Go Serverless v1.0! Your function executed successfully!",
        "event": event
    }
    """

# def next_move(count):
#     count = int(count) // 2
#     fen = request.form['fen']
#     if count <= 5:
#         move = opening_book_next_move(fen, 'performance.bin')
#         if move is not None:
#             return move

#     # use the c version one
#     b = Board(fen)
#     if is_endgame(b):
#         move = query_end_game_move(b.fen())
#         if move != None:
#             return move

#     if b.turn:
#         side = '0'  # white
#     else:
#         side = '1'
#     result = s.run(['./ai/c_modules/main', fen ,side], stdout=s.PIPE)
#     move = result.stdout.decode('utf-8').split("\n")[-2]