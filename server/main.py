from fastapi import FastAPI
from pydantic import BaseModel
import os, json, time, threading
import requests
from typing import Dict, List, Any, Optional

app = FastAPI()

# ===== Zhipu / GLM =====
ZHIPU_API_KEY = os.getenv("ZHIPU_API_KEY", "ee52b0e5db624872b3e49c37683dfbb3.LSXvrEWgrJjOTXhb")
ZHIPU_URL = "https://open.bigmodel.cn/api/paas/v4/chat/completions"
MODEL = "GLM-4-Flash"

# ===== Memory config =====
MAX_TURNS = 12
SESSION_TTL_SEC = 60 * 30

_SESSIONS: Dict[str, Dict[str, Any]] = {}
_LOCK = threading.Lock()

def load_json_if_exists(path: str) -> Optional[dict]:
    try:
        if os.path.exists(path):
            with open(path, "r", encoding="utf-8") as f:
                return json.load(f)
    except Exception:
        return None
    return None

WORLD = load_json_if_exists("world.json") or load_json_if_exists("server/world.json")
NPCS = load_json_if_exists("npc.json") or load_json_if_exists("server/npc.json")

def get_npc_prompt(npc_id: str) -> str:
    base = (
        "You are an NPC in a video game. "
        "Stay in character. Speak naturally and briefly. "
        "Never mention AI or models."
    )

    world_text = ""
    if isinstance(WORLD, dict):
        world_text = WORLD.get("prompt", "") or WORLD.get("world", "") or ""

    npc_text = ""
    if isinstance(NPCS, dict):
        if npc_id in NPCS and isinstance(NPCS[npc_id], dict):
            npc_text = NPCS[npc_id].get("prompt", "") or NPCS[npc_id].get("persona", "")
        else:
            npcs_list = NPCS.get("npcs")
            if isinstance(npcs_list, list):
                for n in npcs_list:
                    if isinstance(n, dict) and (n.get("id") == npc_id or n.get("name") == npc_id):
                        npc_text = n.get("prompt", "") or n.get("persona", "")
                        break

    parts = [base]

    if world_text:
        parts.append(f"World setting: {world_text}")

    if npc_text:
        parts.append(f"NPC persona: {npc_text}")
    else:
        parts.append(f"You are NPC named {npc_id}. Stay consistent.")

    return "\n".join(parts)

def session_key(player_id: str, npc_id: str) -> str:
    return f"{player_id}::{npc_id}"

def trim_messages(messages: List[dict]) -> List[dict]:
    if not messages:
        return messages

    system = [messages[0]] if messages[0].get("role") == "system" else []
    rest = messages[1:] if system else messages

    keep = rest[-MAX_TURNS * 2 :]
    return system + keep

def cleanup_sessions():
    now = time.time()
    with _LOCK:
        dead = [k for k, v in _SESSIONS.items() if now - v.get("updated_at", 0) > SESSION_TTL_SEC]
        for k in dead:
            _SESSIONS.pop(k, None)

class ChatRequest(BaseModel):
    message: str
    npc_id: str = "Mara"
    player_id: str = "Player0"

class ResetRequest(BaseModel):
    npc_id: str = "Mara"
    player_id: str = "Player0"

@app.post("/reset")
def reset(req: ResetRequest):
    key = session_key(req.player_id, req.npc_id)
    with _LOCK:
        _SESSIONS.pop(key, None)
    return {"ok": True}

@app.post("/chat")
def chat(req: ChatRequest):
    cleanup_sessions()

    user_text = (req.message or "").strip()
    npc_id = (req.npc_id or "Mara").strip()
    player_id = (req.player_id or "Player0").strip()

    if not user_text:
        return {"reply": "Say something."}

    if not ZHIPU_API_KEY:
        return {"reply": "Server error: missing API key"}

    key = session_key(player_id, npc_id)

    with _LOCK:
        if key not in _SESSIONS:
            _SESSIONS[key] = {
                "messages": [{"role": "system", "content": get_npc_prompt(npc_id)}],
                "updated_at": time.time(),
            }

        _SESSIONS[key]["messages"].append({"role": "user", "content": user_text})
        _SESSIONS[key]["messages"] = trim_messages(_SESSIONS[key]["messages"])
        _SESSIONS[key]["updated_at"] = time.time()

        messages_to_send = list(_SESSIONS[key]["messages"])

    payload = {
        "model": MODEL,
        "messages": messages_to_send,
        "temperature": 0.7,
        "max_tokens": 512,
    }

    headers = {
        "Authorization": f"Bearer {ZHIPU_API_KEY}",
        "Content-Type": "application/json",
    }

    try:
        r = requests.post(ZHIPU_URL, headers=headers, json=payload, timeout=60)
        r.raise_for_status()
        data = r.json()
        reply = data["choices"][0]["message"]["content"].strip()

        with _LOCK:
            if key in _SESSIONS:
                _SESSIONS[key]["messages"].append({"role": "assistant", "content": reply})
                _SESSIONS[key]["messages"] = trim_messages(_SESSIONS[key]["messages"])
                _SESSIONS[key]["updated_at"] = time.time()

        return {"reply": reply}

    except Exception as e:
        return {"reply": f"Server error: {str(e)}"}