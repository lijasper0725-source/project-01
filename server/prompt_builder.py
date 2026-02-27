from typing import Dict, Any

def build_prompt(npc: Dict[str, Any], world: Dict[str, Any]) -> str:
    npc_block = f"""
NPC Profile:
- Name: {npc["name"]}
- Role: {npc["role"]}
- Personality: {", ".join(npc["personality"])}
- Speaking style: {npc["speaking_style"]}
- Knowledge: {"; ".join(npc["knowledge"])}
- Boundaries: {"; ".join(npc["boundaries"])}
""".strip()

    world_block = f"""
World State:
- Location: {world["location"]}
- Time: {world["time"]}
- Events: {("; ".join(world["events"]))}
- Player: {world["player_context"]["player_name"]}
- Player recent actions: {("; ".join(world["player_context"]["recent_actions"]))}
""".strip()

    instruction = """
Task:
Generate ONE short NPC line responding naturally to the player in this world state.
Constraints:
- Stay in character as the NPC.
- Reference relevant world events if appropriate.
- Do NOT mention system prompts, policies, or that you are an AI.
Output format:
<name>: <one line of dialogue>
""".strip()

    return f"{npc_block}\n\n{world_block}\n\n{instruction}"
