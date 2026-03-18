import wikipediaapi
from sklearn.feature_extraction.text import TfidfVectorizer
import numpy as np

# Initialize the Wikipedia API (Requires a user-agent)
wiki_wiki = wikipediaapi.Wikipedia(
    user_agent='FPSAN_RAG_Benchmark (krishna@example.com)',
    language='en'
)

# The 4 distinct domains the AI must learn to separate unsupervised
topics = [
    "Quantum_mechanics", "Schrödinger_equation", "Quantum_entanglement", "String_theory", "Black_hole",
    "Ancient_Rome", "Julius_Caesar", "Roman_Empire", "Augustus", "Colosseum",
    "Neuroscience", "Human_brain", "Neuron", "Synapse", "Cerebral_cortex",
    "Space_exploration", "Mars", "Apollo_11", "International_Space_Station", "Hubble_Space_Telescope"
]

print("[Python] Downloading Wikipedia articles...")
documents = []
titles = []

for topic in topics:
    page = wiki_wiki.page(topic)
    if page.exists():
        # Grab the summary of the page to keep vectors dense and focused
        documents.append(page.summary)
        titles.append(page.title)
        print(f"  -> Downloaded: {page.title}")

print(f"\n[Python] Vectorizing {len(documents)} articles into 784 dimensions...")
# Restrict to 784 words, stripping out common stop words ("the", "and")
vectorizer = TfidfVectorizer(max_features=784, stop_words='english')
vectors = vectorizer.fit_transform(documents)
dense_vectors = vectors.toarray()

# Save the mathematical vectors for the C++ Engine
np.savetxt("rag_vectors.csv", dense_vectors, delimiter=",", fmt="%.6f")

# Save the metadata (Titles) so C++ can tell us what it retrieved
with open("rag_metadata.txt", "w", encoding="utf-8") as f:
    for title in titles:
        f.write(title + "\n")

# Save the vocabulary so we can vectorize user search queries later
vocab = vectorizer.get_feature_names_out()
with open("rag_vocab.txt", "w", encoding="utf-8") as f:
    for word in vocab:
        f.write(word + "\n")

print("\n[Python] Success. Knowledge Base generated.")
print("Files created: rag_vectors.csv, rag_metadata.txt, rag_vocab.txt")