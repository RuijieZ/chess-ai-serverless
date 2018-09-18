let WHITE = 'w';
let BLACK = 'b';
let HUMAN = 'HUMAN';
let AI = 'AI';

/*
Game class
*/
class Game {
    constructor(board, chess) {
        this.board = board;
        this.chess = chess;
        this.players = {
            'w': HUMAN,
            'b': AI
        }
        this.stepCount = 0;
    }

    newGame() {
        return true;
    }
}

/*
Helper functions
*/
function queryNextMove(async) {
    var chess = window.game.chess;
    var data = {
        'fen': chess.fen(),
    };

    $.ajax({
        type: 'POST',
        url:  '/next_move/'+window.game.stepCount.toString(),
        data: data,
        success: function(result) {
            var serverMove = {                      // reformat the move so that the frontend code can understand
                'from': result.substring(0,2),
                'to': result.substring(2,4)
            }
            if (result.length == 5) {
                serverMove['promotion'] = result.substring(4,5);    // promotion
            }
            window.game.chess.move(serverMove);     // make the move
            console.log('the move returned from server is: ' + result);
            window.game.board.position(window.game.chess.fen());
            updateStatus();
        },
        statusCode: {
          502: function(jqXHR) {
            console.log('Bad gateway, check if the server is down');
            var retryAfter = jqXHR.getResponseHeader('Retry-After');
            retryAfter = parseInt(retryAfter, 10);
            if (!retryAfter) retryAfter = 3;
            setTimeout(data, retryAfter * 1000);
          }
        },
        async: async
    });
}

var onDragStart = function(source, piece, position, orientation) {
    var chess = window.game.chess;
    if ((chess.turn() === BLACK && piece.search(/^b/) === -1) ||
        (chess.turn() === WHITE && piece.search(/^w/) === -1) ||
        chess.game_over())
    {
        return false;
    }
    return true;
};

function AISelfPaly() {
    if (window.AI_VS_AI_STOP) return;
    this.game.players = {
        'w': AI,
        'b': AI
    }

    var possibleMoves = this.game.chess.moves();
    if (this.game.chess.game_over() === true ||
        this.game.chess.in_draw() === true ||
        possibleMoves.length === 0) return;
    else {
        queryNextMove(false);
        window.setTimeout(AISelfPaly, 1000);
    }

}


var onDrop = function(source, target) {
  // see if the move is legal
  var move = window.game.chess.move({
    from: source,
    to: target,
    promotion: 'q' // NOTE: always promote to a queen for example simplicity
  });

  // illegal move
  if (move === null) return 'snapback';

  updateStatus();
};


var updateStatus = function() {
    var chess = window.game.chess;
    var status = '';
    var statusEl = $('#status');
    var fenEl = $('#fen');
    var pgnEl = $('#pgn');

    var moveColor = 'White';
    window.game.stepCount += 1;
    if (chess.turn() === BLACK) {
        moveColor = 'Black';
    }

    // checkmate?
    if (chess.in_checkmate() === true) {
        status = 'Game over, ' + moveColor + ' is in checkmate.';
    }

    // draw?
    else if (chess.in_draw() === true) {
        status = 'Game over, drawn position';
    }

    // game still on
    else {
        status = moveColor + ' to move';

        // check?
        if (chess.in_check() === true) {
            status += ', ' + moveColor + ' is in check';
        }
    }

    statusEl.html(status);
    fenEl.html(chess.fen());
    pgnEl.html(chess.pgn());
};

// update the board position after the piece snap
// for castling, en passant, pawn promotion
var onSnapEnd = function() {
    window.game.board.position(window.game.chess.fen());
    var chess = window.game.chess;
    if ((chess.turn() === BLACK && window.game.players['b'] == AI) || (chess.turn() === WHITE && window.game.players['w'] == AI))
        queryNextMove(true);
};

function AIvsHuman() {
    var cfg = {
        draggable: true,
        orientation: 'black',
        position: 'start',
        dropOffBoard: 'snapback', // this is the default
        onDragStart: onDragStart,
        onDrop: onDrop,
        onSnapEnd: onSnapEnd
    };
    var chess = new Chess();
    var board = ChessBoard('board', cfg);
    window.game = new Game(board, chess);
    window.game.players = {
        'w': AI,
        'b': HUMAN,
    }
    queryNextMove(true);
}

function HumanVsAI() {
    var cfg = {
        position: 'start',
        draggable: true,
        dropOffBoard: 'snapback', // this is the default
        onDragStart: onDragStart,
        onDrop: onDrop,
        onSnapEnd: onSnapEnd
    };
    var chess = new Chess();
    var board = ChessBoard('board', cfg);
    window.game = new Game(board, chess);
    window.game.players = {
        'w': HUMAN,
        'b': AI,
    }
}

var init = function() {
    var cfg = {
        draggable: true,
        dropOffBoard: 'snapback', // this is the default
        position: 'start',
        onDragStart: onDragStart,
        onDrop: onDrop,
        onSnapEnd: onSnapEnd
    };
    var chess = new Chess();
    var board = ChessBoard('board', cfg);
    window.game = new Game(board, chess);
    window.AI_VS_AI_STOP = true;

    $("#selfPlay").click(function() {
        window.AI_VS_AI_STOP = false;
        AISelfPaly();
    });

    $("#AIvsHUMAN").click(function() {
        window.AI_VS_AI_STOP = true;
        AIvsHuman();
    });

    $("#HumanVsAI").click(function() {
        window.AI_VS_AI_STOP = true;
        HumanVsAI();
    });
};
$(document).ready(init);