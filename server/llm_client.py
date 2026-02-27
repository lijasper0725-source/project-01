from typing import Literal

Mode = Literal["mock", "openai"]

def generate_dialogue(prompt: str, mode: Mode = "mock") -> str:
    if mode == "mock":
        return "Mara: If you’re heading north, keep your purse close. Folks get desperate when heroes vanish."

    if mode == "openai":
        try:
            from openai import OpenAI
        except Exception as e:
            raise RuntimeError("OpenAI SDK not installed. Install with: pip install openai") from e

        client = OpenAI()
        resp = client.chat.completions.create(
            model="gpt-4.1-mini",
            messages=[
                {"role": "system", "content": "You are an NPC in a fantasy game. Stay in character."},
                {"role": "user", "content": prompt},
            ],
            temperature=0.8,
            max_tokens=80,
        )
        return resp.choices[0].message.content.strip()

    raise ValueError(f"Unknown mode: {mode}")
