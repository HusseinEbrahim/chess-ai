import { useEffect, useRef, useState } from "react";
import { Chess } from "chess.js";
import { Chessboard } from "react-chessboard";
import { getEngineMove } from "./api";

const DIFFICULTIES = ["easy", "medium", "hard"];

function getStatus(game) {
  if (game.isCheckmate()) return `Checkmate! ${game.turn() === "w" ? "Black" : "White"} wins`;
  if (game.isStalemate()) return "Draw by stalemate";
  if (game.isThreefoldRepetition()) return "Draw by repetition";
  if (game.isInsufficientMaterial()) return "Draw by insufficient material";
  if (game.isDraw()) return "Draw";
  const side = game.turn() === "w" ? "White" : "Black";
  return game.inCheck() ? `${side} to move (in check!)` : `${side} to move`;
}

function formatEval(info) {
  if (!info) return null;
  const whiteScore = info.engineColor === "w" ? info.score : -info.score;
  if (Math.abs(whiteScore) > 90000) return whiteScore > 0 ? "White is mating" : "Black is mating";
  return `${whiteScore > 0 ? "+" : ""}${(whiteScore / 100).toFixed(2)}`;
}

function ToggleGroup({ options, value, onChange }) {
  return (
    <div className="inline-flex rounded-lg bg-slate-100 p-1">
      {options.map((option) => (
        <button
          key={option.value}
          onClick={() => onChange(option.value)}
          className={`rounded-md px-3 py-1.5 text-sm capitalize ${
            value === option.value ? "bg-white shadow text-slate-900 font-medium" : "text-slate-600 hover:text-slate-900"
          }`}
        >
          {option.label}
        </button>
      ))}
    </div>
  );
}

export default function App() {
  const gameRef = useRef(new Chess());
  const requestId = useRef(0);

  const [fen, setFen] = useState(gameRef.current.fen());
  const [history, setHistory] = useState([]);
  const [mode, setMode] = useState("ai");  // "ai" or "local"
  const [playerColor, setPlayerColor] = useState("w");
  const [difficulty, setDifficulty] = useState("medium");
  const [thinking, setThinking] = useState(false);
  const [engineInfo, setEngineInfo] = useState(null);
  const [error, setError] = useState("");
  const [retry, setRetry] = useState(0);

  const game = gameRef.current;
  const aiTurn = mode === "ai" && game.turn() !== playerColor && !game.isGameOver();

  function sync() {
    setFen(game.fen());
    setHistory(game.history());
  }

  // Whenever it's the engine's turn, ask it for a move
  useEffect(() => {
    if (!aiTurn) return;
    const id = ++requestId.current;
    setThinking(true);
    setError("");

    getEngineMove(fen, difficulty)
      .then((data) => {
        if (id !== requestId.current) return;  // game was reset or undone meanwhile
        game.move({ from: data.move.slice(0, 2), to: data.move.slice(2, 4), promotion: data.move[4] });
        setEngineInfo({ ...data, engineColor: playerColor === "w" ? "b" : "w" });
        sync();
      })
      .catch((err) => {
        if (id === requestId.current) setError(err.message);
      })
      .finally(() => {
        if (id === requestId.current) setThinking(false);
      });
  }, [fen, aiTurn, difficulty, retry]);

  function onPieceDrop({ sourceSquare, targetSquare }) {
    if (!targetSquare || thinking) return false;
    if (mode === "ai" && game.turn() !== playerColor) return false;
    try {
      game.move({ from: sourceSquare, to: targetSquare, promotion: "q" });
      sync();
      return true;
    } catch {
      return false;
    }
  }

  function startNewGame(nextMode = mode, nextColor = playerColor) {
    requestId.current++;  // ignore any engine reply still on its way
    game.reset();
    setMode(nextMode);
    setPlayerColor(nextColor);
    setThinking(false);
    setEngineInfo(null);
    setError("");
    sync();
  }

  function undo() {
    requestId.current++;
    setThinking(false);
    game.undo();
    // Against the engine, take back both the engine's move and yours
    if (mode === "ai" && game.turn() !== playerColor) game.undo();
    sync();
  }

  const movePairs = [];
  for (let i = 0; i < history.length; i += 2) movePairs.push([history[i], history[i + 1]]);

  const orientation = mode === "ai" && playerColor === "b" ? "black" : "white";

  return (
    <div className="min-h-screen bg-slate-100 p-6">
      <div className="max-w-5xl mx-auto">
        <h1 className="text-2xl font-bold text-slate-800 mb-4">Chess</h1>

        <div className="flex flex-col lg:flex-row gap-6">
          <div className="w-full max-w-[560px]">
            <Chessboard options={{ position: fen, onPieceDrop, boardOrientation: orientation }} />
          </div>

          <div className="flex-1 space-y-4">
            <div className="bg-white rounded-xl shadow p-5 space-y-4">
              <ToggleGroup
                options={[
                  { value: "ai", label: "vs Computer" },
                  { value: "local", label: "Two players" },
                ]}
                value={mode}
                onChange={(value) => startNewGame(value, playerColor)}
              />

              {mode === "ai" && (
                <div className="space-y-3">
                  <div className="flex items-center gap-3">
                    <span className="w-20 text-sm text-slate-500">Play as</span>
                    <ToggleGroup
                      options={[
                        { value: "w", label: "White" },
                        { value: "b", label: "Black" },
                      ]}
                      value={playerColor}
                      onChange={(value) => startNewGame("ai", value)}
                    />
                  </div>
                  <div className="flex items-center gap-3">
                    <span className="w-20 text-sm text-slate-500">Difficulty</span>
                    <ToggleGroup
                      options={DIFFICULTIES.map((d) => ({ value: d, label: d }))}
                      value={difficulty}
                      onChange={setDifficulty}
                    />
                  </div>
                </div>
              )}

              <div className="flex gap-2">
                <button
                  onClick={undo}
                  disabled={history.length === 0}
                  className="rounded-lg bg-slate-100 px-4 py-2 text-slate-700 hover:bg-slate-200 disabled:opacity-50"
                >
                  Undo
                </button>
                <button
                  onClick={() => startNewGame()}
                  className="rounded-lg bg-slate-900 px-4 py-2 text-white hover:bg-slate-800"
                >
                  New game
                </button>
              </div>
            </div>

            <div className="bg-white rounded-xl shadow p-5 space-y-3">
              <div className="text-lg font-semibold text-slate-800">
                {thinking ? "Engine is thinking..." : getStatus(game)}
              </div>

              {error && (
                <div className="rounded-lg bg-red-50 text-red-700 text-sm px-3 py-2 flex items-center justify-between">
                  <span>{error}</span>
                  <button onClick={() => setRetry((r) => r + 1)} className="font-medium underline">
                    Retry
                  </button>
                </div>
              )}

              {engineInfo && mode === "ai" && (
                <div className="text-sm text-slate-500">
                  Evaluation <span className="font-semibold text-slate-800">{formatEval(engineInfo)}</span>
                  {" · "}depth {engineInfo.depth}
                  {" · "}
                  {engineInfo.nodes.toLocaleString()} positions
                  {" · "}
                  {(engineInfo.timeMs / 1000).toFixed(2)}s
                </div>
              )}

              <div>
                <h2 className="text-sm font-medium text-slate-500 mb-2">Moves</h2>
                <div className="max-h-72 overflow-y-auto text-sm font-mono">
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
    </div>
  );
}