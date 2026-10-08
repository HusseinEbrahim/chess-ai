import { useRef, useState } from "react";
import { Chess } from "chess.js";
import { Chessboard } from "react-chessboard";

function getStatus(game) {
  if (game.isCheckmate()) return `Checkmate! ${game.turn() === "w" ? "Black" : "White"} wins`;
  if (game.isStalemate()) return "Draw by stalemate";
  if (game.isThreefoldRepetition()) return "Draw by repetition";
  if (game.isInsufficientMaterial()) return "Draw by insufficient material";
  if (game.isDraw()) return "Draw";
  const side = game.turn() === "w" ? "White" : "Black";
  return game.inCheck() ? `${side} to move (in check!)` : `${side} to move`;
}

export default function App() {
  const gameRef = useRef(new Chess());
  const [fen, setFen] = useState(gameRef.current.fen());
  const [history, setHistory] = useState([]);

  function sync() {
    setFen(gameRef.current.fen());
    setHistory(gameRef.current.history());
  }

  function onPieceDrop({ sourceSquare, targetSquare }) {
    if (!targetSquare) return false;
    try {
      gameRef.current.move({ from: sourceSquare, to: targetSquare, promotion: "q" });
      sync();
      return true;
    } catch {
      return false;  // illegal move: the piece snaps back
    }
  }

  function undo() {
    gameRef.current.undo();
    sync();
  }

  function reset() {
    gameRef.current.reset();
    sync();
  }

  // Pair moves up: [["e4", "e5"], ["Nf3", "Nc6"], ...]
  const movePairs = [];
  for (let i = 0; i < history.length; i += 2) movePairs.push([history[i], history[i + 1]]);

  return (
    <div className="min-h-screen bg-slate-100 p-6">
      <div className="max-w-5xl mx-auto">
        <h1 className="text-2xl font-bold text-slate-800 mb-4">Chess</h1>

        <div className="flex flex-col lg:flex-row gap-6">
          <div className="w-full max-w-[560px]">
            <Chessboard options={{ position: fen, onPieceDrop }} />
          </div>

          <div className="flex-1 bg-white rounded-xl shadow p-5 space-y-4">
            <div className="text-lg font-semibold text-slate-800">{getStatus(gameRef.current)}</div>

            <div className="flex gap-2">
              <button
                onClick={undo}
                disabled={history.length === 0}
                className="rounded-lg bg-slate-100 px-4 py-2 text-slate-700 hover:bg-slate-200 disabled:opacity-50"
              >
                Undo
              </button>
              <button
                onClick={reset}
                className="rounded-lg bg-slate-900 px-4 py-2 text-white hover:bg-slate-800"
              >
                New game
              </button>
            </div>

            <div>
              <h2 className="text-sm font-medium text-slate-500 mb-2">Moves</h2>
              <div className="max-h-80 overflow-y-auto text-sm font-mono">
                {movePairs.length === 0 && <p className="text-slate-400">No moves yet</p>}
                {movePairs.map(([white, black], i) => (
                  <div key={i} className="flex gap-3 py-0.5">
                    <span className="w-8 text-slate-400">{i + 1}.</span>
                    <span className="w-16 text-slate-800">{white}</span>
                    <span className="w-16 text-slate-800">{black ?? ""}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}