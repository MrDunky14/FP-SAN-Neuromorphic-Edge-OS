#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>

using namespace std;

// --- 1. Dimensions ---
const int INPUT_DIM = 5;      // A simple 1D visual field (5 pixels)
const int CLUSTER_DIM = 10;   // The Neocortex capacity

// --- 2. Temporal Dendrites (New in v5) ---
struct LateralSynapse {
    int target_cluster;
    float weight; // Strength of the temporal prediction
};

struct LIF_Neuron {
    // Spatial (What it sees)
    int8_t spatial_weights[INPUT_DIM]; 
    float membrane_potential; 
    
    // Temporal (What it predicts)
    float predictive_potential; 
    vector<LateralSynapse> lateral_synapses; 
    
    // Metadata
    uint64_t last_spike_time;   
};

LIF_Neuron Neocortex[CLUSTER_DIM];
int last_fired_cluster = -1; // Memory of t-1

void initialize_brain() {
    for (int c = 0; c < CLUSTER_DIM; c++) {
        for (int i = 0; i < INPUT_DIM; i++) Neocortex[c].spatial_weights[i] = 0;
        Neocortex[c].membrane_potential = 0.0f;
        Neocortex[c].predictive_potential = 0.0f;
        Neocortex[c].last_spike_time = 0;
    }
}

// --- 3. The Predictive Inference Engine ---
void process_temporal_spikes(const vector<int8_t>& incoming_spikes, bool training_mode) {
    int winning_cluster = -1;
    int first_empty_cluster = -1;
    float highest_voltage = -9999.0f;

    // Phase 2: Integrate Spatial Input
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Neocortex[c].last_spike_time == 0) {
            if (first_empty_cluster == -1) first_empty_cluster = c;
            continue; 
        }

        float incoming_current = 0.0f;
        int active_synapses = 0;

        for (int i = 0; i < INPUT_DIM; i++) {
            if (incoming_spikes[i] != 0 || Neocortex[c].spatial_weights[i] != 0) {
                if (Neocortex[c].spatial_weights[i] == incoming_spikes[i]) incoming_current += 1.0f;
                else incoming_current -= 1.0f;
                active_synapses++;
            }
        }

        if (active_synapses > 0) {
            // TOTAL VOLTAGE = What I see (Spatial) + What I expected (Predictive)
            float spatial_voltage = incoming_current / active_synapses;
            Neocortex[c].membrane_potential = spatial_voltage + (Neocortex[c].predictive_potential * 0.5f);

            if (Neocortex[c].membrane_potential > highest_voltage) {
                highest_voltage = Neocortex[c].membrane_potential;
                winning_cluster = c;
            }
        }
    }

    // Phase 3: Neurogenesis
    if (highest_voltage < 0.5f && first_empty_cluster != -1 && training_mode) {
        winning_cluster = first_empty_cluster;
        cout << "[Cortex] Anomaly detected. Neurogenesis forming Cluster " << winning_cluster << endl;
    }

    // Phase 4: Temporal STDP (Wiring time together)
    if (winning_cluster != -1) {
        if (training_mode) {
            // 1. Spatial Bonding (Learn the shape)
            for (int i = 0; i < INPUT_DIM; i++) {
                if (incoming_spikes[i] != 0) Neocortex[winning_cluster].spatial_weights[i] = incoming_spikes[i];
            }
            Neocortex[winning_cluster].last_spike_time = 1;

            // 2. Temporal Bonding (Wire t-1 to t)
            if (last_fired_cluster != -1 && last_fired_cluster != winning_cluster) {
                bool synapse_exists = false;
                for (auto& syn : Neocortex[last_fired_cluster].lateral_synapses) {
                    if (syn.target_cluster == winning_cluster) {
                        syn.weight += 0.2f; // Strengthen existing temporal bond
                        if (syn.weight > 1.0f) syn.weight = 1.0f;
                        synapse_exists = true; break;
                    }
                }
                if (!synapse_exists) {
                    Neocortex[last_fired_cluster].lateral_synapses.push_back({winning_cluster, 0.2f});
                    cout << "   -> [Synapse] Temporal Bond formed: Cluster " << last_fired_cluster << " predicts Cluster " << winning_cluster << endl;
                }
            }
        }
        
        cout << "[Spike] Cluster " << winning_cluster << " Fired." << endl;
        last_fired_cluster = winning_cluster;
    }

    // Phase 5: Fast leak, THEN forward-propagate prediction immediately
    for (int c = 0; c < CLUSTER_DIM; c++) {
        Neocortex[c].predictive_potential *= 0.1f; // Fast leak of old predictions
        Neocortex[c].membrane_potential = 0.0f;    // Reset spatial voltage
    }

    // THE FIX: Instantly pre-charge the expected future state
    if (last_fired_cluster != -1) {
        for (auto& synapse : Neocortex[last_fired_cluster].lateral_synapses) {
            Neocortex[synapse.target_cluster].predictive_potential += synapse.weight;
        }
    }
}
// --- 4. The Benchmark ---
int main() {
    cout << "==========================================" << endl;
    cout << " FP-SAN v5 | Neocortical Sequence Engine  " << endl;
    cout << "==========================================\n" << endl;
    
    initialize_brain();

    // A sequence representing an object moving left to right
    vector<vector<int8_t>> motion_sequence = {
        {1, 0, 0, 0, 0}, // t=0
        {0, 1, 0, 0, 0}, // t=1
        {0, 0, 1, 0, 0}, // t=2
        {0, 0, 0, 1, 0}, // t=3
        {0, 0, 0, 0, 1}  // t=4
    };

    cout << "--- PHASE 1: OBSERVING PHYSICS (Training) ---" << endl;
    // Let the AI watch the object move 3 times so it learns the temporal bonds
    for (int loop = 0; loop < 3; loop++) {
        cout << "\nObservation Cycle " << loop + 1 << ":" << endl;
        for (const auto& frame : motion_sequence) {
            process_temporal_spikes(frame, true);
        }
        last_fired_cluster = -1; // Reset memory between cycles
    }

    cout << "\n--- PHASE 2: THE PREDICTION TEST (Inference) ---" << endl;
    cout << "The AI is shown ONLY the first frame [1, 0, 0, 0, 0]." << endl;
    cout << "Checking internal Neocortical voltage..." << endl;
    
    // Feed it only the very first frame
    process_temporal_spikes(motion_sequence[0], false);

    cout << "\n[System] Internal Predictive State after Frame 1:" << endl;
    for (int c = 0; c < CLUSTER_DIM; c++) {
        if (Neocortex[c].predictive_potential > 0.1f) {
            cout << " -> Cluster " << c << " is Pre-Charged (Voltage: " 
                 << fixed << setprecision(2) << Neocortex[c].predictive_potential 
                 << "mV). The AI predicts this will happen next." << endl;
        }
    }

    return 0;
}