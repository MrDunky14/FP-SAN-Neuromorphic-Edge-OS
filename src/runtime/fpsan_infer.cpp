#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <fstream>
#include <sstream>
#include <random>

using namespace std;

// --- 1. Network Dimensions ---
const int INPUT_DIM = 784;         
const int HIDDEN_DIM = 784;        
const int CLUSTER_DIM = 100;       

// --- 2. The 4D Temporal Neuron ---
struct Neuron {
    int8_t weight;              
    float stiffness;            
    uint64_t birth_timestamp;   
    uint64_t last_fired_time;   
};

// --- 3. Full-Duplex Memory & Threading Globals ---
Neuron Brain_Bank_A[CLUSTER_DIM][HIDDEN_DIM]; 
Neuron Brain_Bank_B[CLUSTER_DIM][HIDDEN_DIM];

// Atomic pointer for the sub-nanosecond memory swap
atomic<Neuron(*)[HIDDEN_DIM]> Active_Brain{Brain_Bank_A}; 

// Thread-safe variables for passing data between Core 1 and Core 2
mutex sensor_mutex;
vector<int8_t> shared_sensor_buffer(INPUT_DIM, 0);
bool new_data_available = false;
bool os_running = true;

// Helper: Get Time
uint64_t get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// --- 4. The Math Module: Unsupervised Atomic Bonding ---
// Notice: It now takes a 'target_bank' so it can learn in the background without touching the active brain.
void process_sensory_input(const vector<int8_t>& input_data, Neuron (*target_bank)[HIDDEN_DIM]) {
    int best_cluster_id = -1;
    int first_empty_cluster_id = -1;
    float highest_similarity = -9999.0f;
    uint64_t current_time = get_current_time_ms();

    // Phase 1: The Measurement (Find the strongest bond)
    for (int c = 0; c < CLUSTER_DIM; c++) {
        
        // Track the first available blank cluster for Neurogenesis
        if (target_bank[c][0].last_fired_time == 0) {
            if (first_empty_cluster_id == -1) first_empty_cluster_id = c;
            continue; // Do NOT score empty clusters. They will unfairly score -100%.
        }

        int similarity_score = 0;
        int active_sensors = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0 || target_bank[c][i].weight != 0) { 
                if (target_bank[c][i].weight == input_data[i]) {
                    similarity_score += 1;
                } else {
                    similarity_score -= 1; // Mismatch Penalty
                }
                active_sensors++;
            }
        }

        if (active_sensors > 0) {
            float normalized_score = (float)similarity_score / active_sensors;
            if (normalized_score > highest_similarity) {
                highest_similarity = normalized_score;
                best_cluster_id = c;
            }
        }
    }

    // Phase 1.5: Adaptive Resonance (Neurogenesis Trigger)
    // If the best match is weak (e.g., less than 5% resonance), or the brain is totally empty,
    // we refuse to merge it into a black hole. We spawn a new cluster.
    float VIGILANCE_THRESHOLD = 0.05f; 

    if (highest_similarity < VIGILANCE_THRESHOLD && first_empty_cluster_id != -1) {
        best_cluster_id = first_empty_cluster_id;
        highest_similarity = 1.0f; // Perfect match for a clean slate
        // Optional: cout << "[Cortex] Anomaly detected. Spawning new cluster..." << endl;
    }

    // Phase 2: The Hebbian Update
    if (best_cluster_id != -1) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (input_data[i] != 0) {
                target_bank[best_cluster_id][i].weight = input_data[i];
                target_bank[best_cluster_id][i].stiffness += 0.05f;
                if (target_bank[best_cluster_id][i].stiffness > 1.0f) {
                    target_bank[best_cluster_id][i].stiffness = 1.0f;
                }
            }
        }
        target_bank[best_cluster_id][0].last_fired_time = current_time;
        
        // Only print actual resonance, not the 100% artificial override
        float display_res = (highest_similarity == 1.0f) ? 0.0f : highest_similarity * 100;
        cout << "[Cortex] Data bonded to Cluster " << best_cluster_id 
             << " | Resonance: " << display_res << "%" << endl;
    }
}

// --- 5. Algorithmic Decay (The Midnight Sweeper) ---
void trigger_algorithmic_decay(Neuron (*target_bank)[HIDDEN_DIM]) {
    uint64_t current_time = get_current_time_ms();
    int clusters_cleared = 0;
    uint64_t forgetting_threshold = 5000; // 5 seconds for testing

    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (target_bank[c][0].last_fired_time > 0) {
            if ((current_time - target_bank[c][0].last_fired_time) > forgetting_threshold) {
                for (int i = 0; i < HIDDEN_DIM; i++) {
                    target_bank[c][i].weight = 0;
                    target_bank[c][i].stiffness = 0.0f;
                    target_bank[c][i].birth_timestamp = 0;
                    target_bank[c][i].last_fired_time = 0;
                }
                clusters_cleared++;
            }
        }
    }
    
    if (clusters_cleared > 0) {
        cout << "\n[Sweeper] Algorithmic Decay Triggered. Pruned " << clusters_cleared << " dead clusters." << endl;
    }
}

// --- 6. Core 2: The Background Cortex (Learning Thread) ---
void background_learning_loop() {
    cout << "[Cortex] Background Learning Thread initialized on Core 2." << endl;
    
    while (os_running) {
        vector<int8_t> local_data;
        
        {
            lock_guard<mutex> lock(sensor_mutex);
            if (!new_data_available) {
                this_thread::yield(); 
                continue;
            }
            local_data = shared_sensor_buffer;
            new_data_available = false;
        }

        // Identify the Shadow Bank (the one NOT currently being used by Inference)
        Neuron(*Shadow_Brain)[HIDDEN_DIM] = (Active_Brain.load() == Brain_Bank_A) ? Brain_Bank_B : Brain_Bank_A;

        // Learn and Sweep on the Shadow Bank safely
        process_sensory_input(local_data, Shadow_Brain); 
        trigger_algorithmic_decay(Shadow_Brain);
        
        // The Atomic Swap: Instantly push the new learning to the active brain
        Active_Brain.store(Shadow_Brain);
    }
}

// --- 7. Core 1: The Eye (CSV Standard Benchmark Loader) ---
void stream_benchmark_dataset(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "[Eye] CRITICAL: Benchmark dataset not found at " << filepath << endl;
        cerr << "[Eye] The OS requires data to function. Halting." << endl;
        return;
    }

    string line;
    uint64_t articles_read = 0;
    cout << "[Eye] Streaming Benchmark Data..." << endl;

    while (getline(file, line) && os_running) {
        stringstream ss(line);
        string value;
        vector<int8_t> article_vector(INPUT_DIM, 0);
        int i = 0;

        while (getline(ss, value, ',') && i < INPUT_DIM) {
            float float_val = stof(value);
            article_vector[i] = (float_val > 0.0f) ? 1 : 0;
            i++;
        }

        {
            lock_guard<mutex> lock(sensor_mutex);
            shared_sensor_buffer = article_vector;
            new_data_available = true;
        }

        articles_read++;
        this_thread::sleep_for(chrono::milliseconds(50)); // Give Core 2 time to process
    }
    cout << "[Eye] Finished reading " << articles_read << " articles." << endl;
}

// --- 8. The OS Boot Sequence ---
int main() {
    cout << "-----------------------------------" << endl;
    cout << "🧠 FP-SAN v2.0 Neuromorphic OS" << endl;
    cout << "-----------------------------------" << endl;
    
    // Launch Core 2 (Learning)
    thread cortex_thread(background_learning_loop);
    
    // Launch Core 1 (Reading)
    // We expect a file named 'benchmark.csv' in the same folder
    stream_benchmark_dataset("benchmark.csv");

    // Shutdown Sequence
    os_running = false;
    cortex_thread.join();
    
    cout << "[System] OS Shutdown Complete." << endl;
    return 0;
}