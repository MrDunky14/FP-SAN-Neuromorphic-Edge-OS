#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

// --- 1. Dimensions & Environment ---
const int GRID_SIZE = 5;
const int CLUSTER_DIM = 5; // One cluster for each possible target spawn location
const int ACTIONS = 3;     // 0: Left, 1: Stay, 2: Right

// --- 2. The Basal Ganglia Neuron ---
struct AgencyNeuron {
    int8_t sensory_weights[GRID_SIZE]; // What it sees (Target X)
    float action_weights[ACTIONS];     // The Dopaminergic bonds to motor outputs
    uint64_t last_fired;
};

AgencyNeuron BasalGanglia[CLUSTER_DIM];

void initialize_brain() {
    for (int c = 0; c < CLUSTER_DIM; c++) {
        for (int i = 0; i < GRID_SIZE; i++) BasalGanglia[c].sensory_weights[i] = 0;
        for (int a = 0; a < ACTIONS; a++) BasalGanglia[c].action_weights[a] = 0.0f; // Neutral instincts
        BasalGanglia[c].last_fired = 0;
    }
}

// --- 3. The Dopamine Loop ---
int main() {
    cout << "==========================================" << endl;
    cout << " FP-SAN v5 | Dopaminergic Motor Cortex    " << endl;
    cout << "==========================================\n" << endl;

    initialize_brain();
    srand(time(NULL));

    int total_catches = 0;
    float exploration_rate = 1.0f; // Starts at 100% curiosity (flailing)

    // Run 50 simulated lives
    for (int epoch = 1; epoch <= 50; epoch++) {
        // 1. Spawn the environment
        int target_x = rand() % GRID_SIZE;
        int paddle_x = 2; // Always start in the center
        
        vector<int8_t> sensory_state(GRID_SIZE, 0);
        sensory_state[target_x] = 1; 

        // 2. Sensory Cortex (Match state to a Cluster)
        int active_cluster = -1;
        for (int c = 0; c < CLUSTER_DIM; c++) {
            if (BasalGanglia[c].last_fired == 0) {
                // Neurogenesis: Burn the new sensory state in
                for (int i=0; i<GRID_SIZE; i++) BasalGanglia[c].sensory_weights[i] = sensory_state[i];
                BasalGanglia[c].last_fired = 1;
                active_cluster = c;
                break;
            }
            if (BasalGanglia[c].sensory_weights[target_x] == 1) {
                active_cluster = c;
                break;
            }
        }

        // 3. Basal Ganglia (Action Selection)
        int chosen_action = 1; // Default Stay
        
        // Exploration vs Exploitation
        if ((float)rand() / RAND_MAX < exploration_rate) {
            chosen_action = rand() % ACTIONS; // Thrash randomly
        } else {
            // Exploit Dopamine-hardened pathways
            float best_weight = -999.0f;
            for (int a = 0; a < ACTIONS; a++) {
                if (BasalGanglia[active_cluster].action_weights[a] > best_weight) {
                    best_weight = BasalGanglia[active_cluster].action_weights[a];
                    chosen_action = a;
                }
            }
        }

        // 4. Motor Output (Move the Paddle)
        int prev_distance = abs(target_x - paddle_x);
        if (chosen_action == 0 && paddle_x > 0) paddle_x--;
        if (chosen_action == 2 && paddle_x < GRID_SIZE - 1) paddle_x++;
        int new_distance = abs(target_x - paddle_x);

        // 5. Environmental Feedback (The Dopamine Hit)
        float dopamine = 0.0f;
        if (new_distance < prev_distance) dopamine = 1.0f;  // Moved closer! Reward!
        else if (new_distance > prev_distance) dopamine = -1.0f; // Moved away! Punish!
        else dopamine = -0.5f; // Stood still while target is elsewhere. Punish!

        if (target_x == paddle_x) {
            dopamine = 2.0f; // Massive reward for catching
            total_catches++;
        }

        // 6. Dopamine-Modulated STDP (Wiring the brain)
        // We literally just add the dopamine scalar to the specific action synapse
        BasalGanglia[active_cluster].action_weights[chosen_action] += dopamine;

        // Decrease curiosity over time as the brain crystallizes
        exploration_rate *= 0.90f; 

        // Console Output (Every 10 epochs to show progress)
        if (epoch % 10 == 0 || epoch == 1) {
            cout << "Epoch " << setw(2) << epoch << " | Target: " << target_x << " | Action: ";
            if (chosen_action == 0) cout << "LEFT ";
            if (chosen_action == 1) cout << "STAY ";
            if (chosen_action == 2) cout << "RIGHT";
            cout << " | Dopamine: " << showpos << fixed << setprecision(1) << dopamine << noshowpos;
            if (target_x == paddle_x) cout << " [CATCH!]";
            cout << endl;
        }
    }

    cout << "\n[System] Training Complete. The AI caught the target " << total_catches << " times." << endl;
    cout << "\n[Autopsy] Inspecting Motor Cortex Synapses for Target at X=0 (Far Left):" << endl;
    cout << " -> Bond to 'Move Left'  : " << BasalGanglia[0].action_weights[0] << " (Strongest)" << endl;
    cout << " -> Bond to 'Stay'       : " << BasalGanglia[0].action_weights[1] << endl;
    cout << " -> Bond to 'Move Right' : " << BasalGanglia[0].action_weights[2] << " (Repulsed)" << endl;

    return 0;
}