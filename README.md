# Unreal AI NPC Dialogue System

An advanced real-time AI-driven NPC dialogue system integrating Unreal Engine 5 with a Python LLM backend.

##  Overview
This project demonstrates a decoupled architecture where a game world's state is dynamically communicated from UE5 (C++) to a Python FastAPI server, which leverages large language models (Zhipu AI/GLM-4) to generate context-aware NPC responses.

##  Key Features
- **C++/Python Bridge**: High-performance asynchronous communication using `FHttpModule` and `JSON` serialization.
- **Dynamic World Perception**: A custom "God Console" (Debug UI) allows players to trigger global events (e.g., Bandit Raids, Ghost King Arrival).
- **Contextual Intelligence**: NPCs perceive the current world state and adapt their personality/knowledge base accordingly.
- **Optimized UI**: Features a typewriter effect for immersive dialogue and a responsive interaction system.

##  Setup & Installation

### 1. Prerequisite (Crucial)
- Ensure the project directory path contains **ONLY English characters** (No special symbols or Chinese characters) to avoid Unreal Build Tool errors.
- Python 3.10+ installed.

### 2. Start the Backend Server (AI Brain)
- Navigate to the `server/` directory.
- **One-Click Startup**: Double-click **`run_server.bat`**. 
  - *This will automatically install dependencies from `requirements.txt` and start the server at `http://127.0.0.1:8000`.*

### 3. Build & Run Unreal Project
As intermediate files have been cleared for submission:
1. Right-click **`NPC_Dialogue_with_AI.uproject`** and select **"Generate Visual Studio project files"**.
2. Open **`NPC_Dialogue_with_AI.sln`**.
3. In Visual Studio, ensure the configuration is **Development Editor / Win64**, then click **Build Solution**.
4. Once compiled, open the project in Unreal Engine 5.7.
5. Load the map: `/Game/ThirdPerson/My_Saloon_Level`.
6. Press **Play**.

##  How to Use
- **Interaction**: Approach the NPC (Brewer) and press the interaction key.
- **God Console**: Press **[Tab]** to toggle the World Event Console. Toggle events and observe how the NPC's dialogue logic changes dynamically based on the "World State".

##  Project Structure
- `/Source`: Custom C++ components for HTTP communication and data management.
- `/server`: Python backend using FastAPI and Zhipu AI SDK.
- `/Content`: Optimized assets including the Saloon environment and UI Blueprints.