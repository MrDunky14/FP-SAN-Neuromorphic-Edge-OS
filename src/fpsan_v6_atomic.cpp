#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstdint>
using namespace std;

// --- 1. Dimensions ---
const int INPUT_DIM = 10;     // A 10-pixel 1D sensor
const int CLUSTER_DIM = 5;    // Small cluster pool for the atomic test

// --- 2. The Hybrid 1.58-Bit Neuron ---
struct LIF_Neuron {
    // The 1.58-bit Matrix (Ultra-low RAM)
    int8_t ternary_weights[INPUT_DIM]; 
    
    // The Continuous Scalar (Infinite Precision)
    float cluster_scalar;              

    // Voltage & Thresholds
    float membrane_potential; 
    float base_leak_rate;          
    float base_spike_threshold;    
    
    uint64_t last_spike_time;   
};

LIF_Neuron Cortex[CLUSTER_DIM];

// --- 3. The Endocrine System (Global Homeostasis) ---
float expected_ambient_energy = 5.0f; // Start expecting ~50% of pixels to be active

void initialize_brain() {
    for (int c = 0; c < CLUSTER_DIM; c++) {
        for (int i = 0; i < INPUT_DIM; i++) Cortex[c].ternary_weights[i] = 0;
        Cortex[c].cluster_scalar = 1.0f; // Default scalar
        Cortex[c].membrane_potential = 0.0f;
        Cortex[c].base_leak_rate = 0.1f;
        Cortex[c].base_spike_threshold = 0.8f; // Requires 80% match natively
        Cortex[c].last_spike_time = 0;
    }
}

// --- 4. The Atomic Inference Engine ---
void process_atomic_spikes(const vector<int8_t>& incoming_spikes, string frame_name) {
    int winning_cluster = -1;
    int first_empty_cluster = -1;
    float highest_voltage = -9999.0f;

    // --- HOMEOSTASIS CALCULATION ---
    float current_frame_energy = 0.0f;
    for (int i = 0; i < INPUT_DIM; i++) {
        if (incoming_spikes[i] != 0) current_frame_energy += 1.0f;
    }

    // Update the moving average of what the AI "expects" the world to look like
    expected_ambient_energy = (0.8f * expected_ambient_energy) + (0.2f * current_frame_energy);
    
    // Calculate the drop or spike in current
    float homeostasis_ratio = 1.0f;
    if (expected_ambient_energy > 0.1f) {
        homeostasis_ratio = current_frame_energy / expected_ambient_energy;
        homeostasis_ratio = pow(homeostasis_ratio, 1.5f);
    }

    // Bound the ratio to prevent infinite sensitivity or total blindness
    if (homeostasis_ratio < 0.3f) homeostasis_ratio = 0.3f; // Max 3x sensitivity for dark rooms
    if (homeostasis_ratio > 1.5f) homeostasis_ratio = 1.5f; // Max dampening for bright rooms

    cout << "\n[" << frame_name << "] Ambient Energy: " << current_frame_energy 
         << " | Homeostasis Sensitivity Ratio: " << fixed << setprecision(2) << homeostasis_ratio << endl;

    // --- INTEGRATE AND FIRE ---
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Cortex[c].last_spike_time == 0) {
            if (first_empty_cluster == -1) first_empty_cluster = c;
            continue; 
        }

        // Apply dynamic thresholds based on the Endocrine system
        float dynamic_threshold = Cortex[c].base_spike_threshold * homeostasis_ratio;
        float dynamic_leak = Cortex[c].base_leak_rate * homeostasis_ratio;

        float raw_ternary_sum = 0.0f;
        int active_synapses = 0;

        for (int i = 0; i < INPUT_DIM; i++) {
            if (incoming_spikes[i] != 0 || Cortex[c].ternary_weights[i] != 0) {
                Cortex[c].membrane_potential *= (1.0f - dynamic_leak);

                // The 1.58-bit Match (Fast Integer Math)
                if (Cortex[c].ternary_weights[i] == incoming_spikes[i] && incoming_spikes[i] != 0) {
                    raw_ternary_sum += 1.0f;
                } else if (incoming_spikes[i] != 0) {
                    raw_ternary_sum -= 1.0f;
                }
                active_synapses++;
            }
        }

        if (active_synapses > 0) {
            // THE HYBRID MATRIX MULTIPLIER
            float incoming_current = (raw_ternary_sum * Cortex[c].cluster_scalar) / active_synapses;
            Cortex[c].membrane_potential += incoming_current;

            if (Cortex[c].membrane_potential > highest_voltage) {
                highest_voltage = Cortex[c].membrane_potential;
                winning_cluster = c;
            }
        }
    }

    // Determine the active dynamic threshold to log it
    float active_threshold = (winning_cluster != -1) ? (Cortex[winning_cluster].base_spike_threshold * homeostasis_ratio) : (0.8f * homeostasis_ratio);

    // --- NEUROGENESIS ---
    if (highest_voltage < active_threshold && first_empty_cluster != -1) {
        winning_cluster = first_empty_cluster;
        cout << "[Cortex] Anomaly. Dynamic Threshold (" << active_threshold << "mV) not met. Spawning Cluster " << winning_cluster << endl;
    }

    // --- STDP LEARNING ---
    if (winning_cluster != -1) {
        for (int i = 0; i < INPUT_DIM; i++) {
            if (incoming_spikes[i] != 0) Cortex[winning_cluster].ternary_weights[i] = incoming_spikes[i];
        }
        Cortex[winning_cluster].last_spike_time = 1;
        
        // Simulating the Scalar Growth (Continuous Precision)
        Cortex[winning_cluster].cluster_scalar += 0.01f; 

        cout << " -> [Spike] Cluster " << winning_cluster << " fired. | Peak Voltage: " 
             << highest_voltage << "mV | Required Threshold: " << active_threshold << "mV" << endl;
        cout << " -> [Matrix] Updated Hybrid Scalar to: " << Cortex[winning_cluster].cluster_scalar << endl;

        Cortex[winning_cluster].membrane_potential = 0.0f; // Refractory reset
    }
}

int main() {
    cout << "===================================================" << endl;
    cout << " FP-SAN v6 | Atomic Core (Hybrid & Homeostasis) " << endl;
    cout << "===================================================\n" << endl;
    
    initialize_brain();

    // A standard, highly visible pattern (Energy = 6)
    vector<int8_t> clear_signal = {1, 1, 1, 0, 0, 0, 0, 1, 1, 1};
    
    // The EXACT same pattern, but severely degraded/dimmed (Energy = 2)
    // A standard PyTorch CNN would fail to recognize this due to low activation weights.
    vector<int8_t> glitch_signal = {1, 0, 0, 0, 0, 0, 0, 0, 0, 1};

    cout << "--- TEST 1: INGESTING CLEAR SIGNAL ---" << endl;
    process_atomic_spikes(clear_signal, "Clear Frame 1");
    process_atomic_spikes(clear_signal, "Clear Frame 2");

    cout << "\n--- TEST 2: THE GLITCH/DIMMING BENCHMARK ---" << endl;
    cout << "Sensors lose 66% of their data. Will the AI recognize the shape?" << endl;
    process_atomic_spikes(glitch_signal, "Glitch Frame 1");

    return 0;
}