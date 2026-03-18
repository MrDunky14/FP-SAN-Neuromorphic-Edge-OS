#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// --- 1. Scale and Dimensions ---
// 784 pixels in an MNIST image (28x28)
const int INPUT_DIM = 784;         
const int HIDDEN_DIM = 784;        
const int CLUSTER_DIM = 500; // Increased capacity for handwriting variance      

// --- 2. The LIF Spiking Neuron ---
struct LIF_Neuron {
    int8_t weight;              
    float membrane_potential; // V(t)
    float leak_rate;          // Lambda
    float spike_threshold;    
    uint64_t last_spike_time;   
};

LIF_Neuron Cortex[CLUSTER_DIM][HIDDEN_DIM];

// --- Initialization ---
void initialize_brain() {
    for (int c = 0; c < CLUSTER_DIM; c++) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            Cortex[c][i].weight = 0;
            Cortex[c][i].membrane_potential = 0.0f;
            Cortex[c][i].leak_rate = 0.1f; // 10% voltage leak per cycle
            Cortex[c][i].spike_threshold = 0.75f; // Requires 75% resonance to spike
            Cortex[c][i].last_spike_time = 0;
        }
    }
}

// Helper: Get Time
uint64_t get_time_ms() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

// --- 3. The Spiking Inference Engine (STDP) ---
void process_spikes(const vector<int8_t>& incoming_spikes) {
    int winning_cluster = -1;
    int first_empty_cluster = -1;
    float highest_voltage = -9999.0f;
    uint64_t current_time = get_time_ms();

    // Phase 1: Integrate and Fire
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Cortex[c][0].last_spike_time == 0) {
            if (first_empty_cluster == -1) first_empty_cluster = c;
            continue; 
        }

        float incoming_current = 0.0f;
        int active_synapses = 0;

        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (incoming_spikes[i] != 0 || Cortex[c][i].weight != 0) {
                // Apply Leak: V(t) = V(t-1) * (1 - leak)
                Cortex[c][i].membrane_potential *= (1.0f - Cortex[c][i].leak_rate);

                // Accumulate Current: w_i * x_i
                if (Cortex[c][i].weight == incoming_spikes[i]) {
                    incoming_current += 1.0f;
                } else {
                    incoming_current -= 1.0f; // Mismatch penalty
                }
                active_synapses++;
            }
        }

        if (active_synapses > 0) {
            // Normalize current and add to membrane potential
            float normalized_current = incoming_current / active_synapses;
            Cortex[c][0].membrane_potential += normalized_current;

            // Track the most highly charged cluster
            if (Cortex[c][0].membrane_potential > highest_voltage) {
                highest_voltage = Cortex[c][0].membrane_potential;
                winning_cluster = c;
            }
        }
    }

    // Phase 2: Neurogenesis (Vigilance)
    // If no cluster reaches the spike threshold, spawn a new one
    if (highest_voltage < Cortex[0][0].spike_threshold && first_empty_cluster != -1) {
        winning_cluster = first_empty_cluster;
        Cortex[winning_cluster][0].membrane_potential = 1.0f; // Force spike
    }

    // Phase 3: Spike-Timing-Dependent Plasticity (Hebbian Update)
    if (winning_cluster != -1) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            if (incoming_spikes[i] != 0) {
                // Hebbian Bonding: The synapse physically wires to the incoming spike
                Cortex[winning_cluster][i].weight = incoming_spikes[i];
            }
        }
        Cortex[winning_cluster][0].last_spike_time = current_time;
        
        // Reset voltage after spiking (Refractory Period)
        Cortex[winning_cluster][0].membrane_potential = 0.0f; 

        cout << "[Cortex] Spike generated at Cluster " << winning_cluster 
             << " | Peak Voltage: " << highest_voltage << "mV" << endl;
    }
}

// --- 4. The Benchmark Streamer ---
void stream_mnist() {
    ifstream file("mnist_stream.csv");
    if (!file.is_open()) {
        cerr << "[System] CRITICAL: 'mnist_stream.csv' not found. Awaiting data." << endl;
        return;
    }

    string line;
    int digit_count = 0;
    cout << "[System] Commencing Continuous Spiking Benchmark..." << endl;

    while (getline(file, line)) {
        stringstream ss(line);
        string value;
        vector<int8_t> visual_cortex_spikes(INPUT_DIM, 0);
        int i = 0;

        // Convert greyscale pixels into binary spikes (1 for signal, 0 for silence)
        while (getline(ss, value, ',') && i < INPUT_DIM) {
            float pixel_intensity = stof(value);
            visual_cortex_spikes[i] = (pixel_intensity > 0.5f) ? 1 : 0; 
            i++;
        }

        process_spikes(visual_cortex_spikes);
        digit_count++;
    }
    cout << "\n[System] Benchmark Complete. " << digit_count << " samples processed sequentially." << endl;
}

int main() {
    cout << "=========================================" << endl;
    cout << "🧠 FP-SAN v3.0 | Spiking LIF Architecture" << endl;
    cout << "=========================================" << endl;
    
    initialize_brain();
    stream_mnist();

    return 0;
}