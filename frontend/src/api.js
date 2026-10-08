const ENGINE_URL = import.meta.env.VITE_ENGINE_URL || "http://localhost:8080";

export async function getEngineMove(fen, difficulty) {
  const res = await fetch(`${ENGINE_URL}/move`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ fen, difficulty }),
  });
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.error || "Engine request failed");
  return data;
}