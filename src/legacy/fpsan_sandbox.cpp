#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>

using namespace std;

// --- 1. Engine Dimensions ---
// 8 Sensors: Up, Down, Left, Right + The 4 Diagonals
const int INPUT_DIM = 8;         
const int HIDDEN_DIM = 8;        
const int CLUSTER_DIM = 10;  

struct Neuron {
    int8_t weight;              
    float stiffness;            
    uint64_t last_fired_time;   
};

// The Brain
Neuron Brain[CLUSTER_DIM][HIDDEN_DIM]; 

// --- 2. The Physical Environment ---
const int MAP_SIZE = 10;
char world_map[MAP_SIZE][MAP_SIZE] = {
    {'#','#','#','#','#','#','#','#','#','#'},
    {'#','.','.','.','#','.','.','.','.','#'},
    {'#','.','#','.','#','.','#','#','.','#'},
    {'#','.','#','.','.','.','#','.','.','#'},
    {'#','.','#','#','#','.','#','.','#','#'},
    {'#','.','.','.','#','.','.','.','.','#'},
    {'#','#','#','.','#','#','#','#','.','#'},
    {'#','.','.','.','.','.','.','#','.','#'},
    {'#','.','#','#','#','#','.','.','.','#'},
    {'#','#','#','#','#','#','#','#','#','#'}
};

int agent_x = 1;
int agent_y = 1;
bool right_sensor_mutilated = false;

// --- 3. The Unsupervised Brain ---
// This function checks if a sensory state matches known "Pain"
float check_resonance(const vector<int8_t>& sensory_state) {
    float highest_similarity = -9999.0f;
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Brain[c][0].last_fired_time == 0) continue; 
        int similarity_score = 0;
        int active_sensors = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (sensory_state[i] != 0 || Brain[c][i].weight != 0) { 
                if (Brain[c][i].weight == sensory_state[i]) similarity_score += 1;
                else similarity_score -= 1; 
                active_sensors++;
            }
        }
        if (active_sensors > 0) {
            float normalized_score = (float)similarity_score / active_sensors;
            if (normalized_score > highest_similarity) highest_similarity = normalized_score;
        }
    }
    return highest_similarity;
}

// Crystallizes a new Pain Memory
void crystallize_pain_memory(const vector<int8_t>& fatal_sensory_state) {
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Brain[c][0].last_fired_time == 0) {
            for (int i = 0; i < HIDDEN_DIM; i++) {
                if (fatal_sensory_state[i] != 0) {
                    Brain[c][i].weight = fatal_sensory_state[i];
                    Brain[c][i].stiffness = 1.0f;
                }
            }
            Brain[c][0].last_fired_time = 1;
            cout << "[Cortex] FATAL COLLISION. Neurogenesis Triggered. Pain Cluster " << c << " Formed." << endl;
            return;
        }
    }
}

// --- 4. The Senses ---
vector<int8_t> get_vision(int test_x, int test_y) {
    vector<int8_t> vision(INPUT_DIM, 0);
    
    // Cardinals
    vision[0] = (world_map[test_y - 1][test_x] == '#') ? -1 : 1; // Up
    vision[1] = (world_map[test_y + 1][test_x] == '#') ? -1 : 1; // Down
    vision[2] = (world_map[test_y][test_x - 1] == '#') ? -1 : 1; // Left
    vision[3] = (world_map[test_y][test_x + 1] == '#') ? -1 : 1; // Right
    
    // Diagonals
    vision[4] = (world_map[test_y - 1][test_x - 1] == '#') ? -1 : 1; // Top-Left
    vision[5] = (world_map[test_y - 1][test_x + 1] == '#') ? -1 : 1; // Top-Right
    vision[6] = (world_map[test_y + 1][test_x - 1] == '#') ? -1 : 1; // Bottom-Left
    vision[7] = (world_map[test_y + 1][test_x + 1] == '#') ? -1 : 1; // Bottom-Right

    // DYNAMIC HEALING TRIGGER: Blind the entire Right Hemisphere
    if (right_sensor_mutilated) {
        vision[3] = 0; // Right
        vision[5] = 0; // Top-Right
        vision[7] = 0; // Bottom-Right
    }
    
    return vision;
}

// --- 5. The OS Simulation Loop ---
void print_world() {
    cout << "\n======================" << endl;
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (x == agent_x && y == agent_y) cout << "A ";
            else cout << world_map[y][x] << " ";
        }
        cout << endl;
    }
    cout << "======================" << endl;
}

int main() {
    cout << "[System] FP-SAN v2.0 Sandbox Environment Booting..." << endl;
    srand(time(NULL));

    int lifecycles = 0;
    int collisions = 0;

    while (lifecycles < 80) {
        print_world();
        this_thread::sleep_for(chrono::milliseconds(200));

        // Trigger hardware damage at step 40
        if (lifecycles == 40) {
            cout << "\n[CRITICAL WARNING] RIGHT SENSOR HARDWARE FAILURE DETECTED." << endl;
            cout << "[CRITICAL WARNING] INITIATING DYNAMIC HEALING PROTOCOLS..." << endl;
            right_sensor_mutilated = true;
            this_thread::sleep_for(chrono::milliseconds(1500));
        }

        // Active Inference: Predict consequences of moving
        vector<pair<int, int>> perfectly_safe_moves;
        vector<pair<int, int>> least_terrifying_moves; // NEW: Track ties for adrenaline escapes
        float lowest_pain_seen = 9999.0f; 

        int dx[] = {0, 0, -1, 1}; // Up, Down, Left, Right
        int dy[] = {-1, 1, 0, 0};

        for (int i = 0; i < 4; i++) {
            int predicted_x = agent_x + dx[i];
            int predicted_y = agent_y + dy[i];
            
            vector<int8_t> predicted_vision = get_vision(predicted_x, predicted_y);
            float pain_resonance = check_resonance(predicted_vision);
            
            // Dynamically track the absolute least-painful moves (handling ties)
            if (pain_resonance < lowest_pain_seen) {
                lowest_pain_seen = pain_resonance;
                least_terrifying_moves.clear();
                least_terrifying_moves.push_back({predicted_x, predicted_y});
            } else if (pain_resonance == lowest_pain_seen) {
                // It's a tie! Add it to the list of possible escape routes
                least_terrifying_moves.push_back({predicted_x, predicted_y});
            }
            
            if (pain_resonance > 0.80f) {
                cout << "[Eye] Path looks dangerous (Pain Match: " << (int)(pain_resonance * 100) << "%)." << endl;
            } else {
                perfectly_safe_moves.push_back({predicted_x, predicted_y});
            }
        }

        // Execute Action: The Hybrid Decision
        int new_x, new_y;
        
        if (!perfectly_safe_moves.empty()) {
            // Option A: Standard Safe Exploration
            int choice = rand() % perfectly_safe_moves.size();
            new_x = perfectly_safe_moves[choice].first;
            new_y = perfectly_safe_moves[choice].second;
        } else {
            // Option B: The Courage Protocol (Adrenaline Thrashing)
            cout << "[System] AI Cornered. Adrenaline override. Thrashing to escape!" << endl;
            // Pick a RANDOM direction out of the least terrifying options
            int choice = rand() % least_terrifying_moves.size(); 
            new_x = least_terrifying_moves[choice].first;
            new_y = least_terrifying_moves[choice].second;
        }

        // Did we actually hit a wall?
        if (world_map[new_y][new_x] == '#') {
            collisions++;
            vector<int8_t> fatal_vision = get_vision(new_x, new_y);
            crystallize_pain_memory(fatal_vision);
            cout << "[Cortex] OUCH! Hit a wall. Adjusting..." << endl;
        } else {
            agent_x = new_x;
            agent_y = new_y;
            cout << "[System] Step " << lifecycles << " | Agent safely advanced." << endl;
        }

        lifecycles++;
    }

    cout << "\n[System] Simulation Ended. Total Collisions Experienced: " << collisions << endl;
    return 0;
}