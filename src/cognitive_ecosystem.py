import subprocess
import tkinter as tk
import random
import math

# --- 1. The Pre-Frontal Cortex (NLP Router) ---
# This filters out conversational fluff so the C++ engine only gets dense keywords
STOP_WORDS = {"who", "what", "where", "when", "why", "how", "is", "was", "a", "the", "he", "she", "it", "of", "do", "does"}

def clean_query(user_input):
    words = user_input.lower().replace("?", "").replace(".", "").split()
    keywords = [w for w in words if w not in STOP_WORDS]
    return " ".join(keywords)

# --- 2. The Hippocampus Link (C++ Subprocess) ---
def query_cpp_engine(keyword_string):
    print(f"\n[Router] Passing filtered concept to C++ Cortex: '{keyword_string}'")
    # In a full production build, we would pipe this into the running C++ binary.
    # For this simulation, we simulate the sub-millisecond retrieval.
    if "augustus" in keyword_string or "empire" in keyword_string:
        return "Augustus"
    elif "brain" in keyword_string or "nervous" in keyword_string:
        return "Neuroscience"
    else:
        return "MEMORY_NOT_FOUND"

# --- 3. The Visual Cortex (Particle Simulation GUI) ---
class SpikingVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("FP-SAN v4 | Live Neural Visualizer")
        self.canvas = tk.Canvas(root, width=800, height=500, bg="#0a0a12", highlightthickness=0)
        self.canvas.pack(pady=20)
        
        self.particles = []
        # Generate 300 dormant neural clusters (particles)
        for _ in range(300):
            x = random.randint(50, 750)
            y = random.randint(50, 450)
            vx = random.uniform(-0.5, 0.5)
            vy = random.uniform(-0.5, 0.5)
            # Tag 0-100 as Text (Left), 101-300 as Vision (Right)
            domain = "text" if x < 400 else "vision"
            node = self.canvas.create_oval(x, y, x+3, y+3, fill="#1e1e32", outline="")
            self.particles.append({"id": node, "x": x, "y": y, "vx": vx, "vy": vy, "domain": domain, "active": 0})

        self.animate()

    def animate(self):
        for p in self.particles:
            # Drift physics
            p["x"] += p["vx"]
            p["y"] += p["vy"]
            
            # Bounce off walls
            if p["x"] < 10 or p["x"] > 790: p["vx"] *= -1
            if p["y"] < 10 or p["y"] > 490: p["vy"] *= -1
            
            # Cool down active spikes
            if p["active"] > 0:
                p["active"] -= 1
                color = "#00ffff" if p["domain"] == "text" else "#ff00ff"
                if p["active"] < 10: color = "#1e1e32" # Fade back to dormant
            else:
                color = "#1e1e32"
                
            self.canvas.coords(p["id"], p["x"], p["y"], p["x"]+4, p["y"]+4)
            self.canvas.itemconfig(p["id"], fill=color)

        self.root.after(30, self.animate)

    def trigger_spike(self, domain_type):
        # Visually spike the neurons related to the specific domain
        for p in self.particles:
            if p["domain"] == domain_type and random.random() < 0.3:
                p["active"] = 30 # Light up for 30 frames

# --- 4. The Master Control Loop ---
def start_ecosystem():
    print("==================================================")
    print(" 🧠 FP-SAN COGNITIVE ECOSYSTEM INITIALIZED")
    print("==================================================")
    
    # Launch the GUI Visualizer in the background
    root = tk.Tk()
    root.configure(bg="#0a0a12")
    visualizer = SpikingVisualizer(root)
    
    def cli_loop():
        print("\n[System] Type a natural question. The Router will filter it and trigger the C++ Engine.")
        query = input("User: ")
        
        if query.lower() in ['exit', 'quit']:
            root.quit()
            return
            
        # 1. NLP Filtering
        cleaned_query = clean_query(query)
        
        # 2. Trigger the Visualizer (Text Domain)
        visualizer.trigger_spike("text")
        
        # 3. C++ Interrogation
        result = query_cpp_engine(cleaned_query)
        
        # 4. Agentic Response
        if result != "MEMORY_NOT_FOUND":
            print(f"FP-SAN: Based on my deep storage, the answer is -> {result}")
        else:
            print("FP-SAN: I do not have a memory of that concept.")
            
        root.after(1000, cli_loop) # Loop the CLI

    root.after(1000, cli_loop)
    root.mainloop()

if __name__ == "__main__":
    start_ecosystem()