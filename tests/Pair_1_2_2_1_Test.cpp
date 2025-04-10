#include <vector>
#include <utility> // For std::pair
#include <cstddef> // For size_t
#include <iostream> // For example usage

/**
 * @brief Computes all unique pairs of elements from a vector.
 *
 * Given a vector of IDs, this function generates all possible pairs (combinations)
 * of two distinct elements. The order within the pair does not matter, i.e.,
 * if {id1, id2} is included, {id2, id1} will not be. The pair itself will
 * store the elements in the order they appeared in the nested loop (first index < second index).
 *
 * @param ids A constant reference to a vector of size_t IDs.
 * @return A vector of std::pair<size_t, size_t> containing all unique pairs.
 * Returns an empty vector if the input has fewer than 2 elements.
 */
std::vector<std::pair<size_t, size_t>> findUniquePairs(const std::vector<size_t>& ids) {
    std::vector<std::pair<size_t, size_t>> pairs;
    size_t n = ids.size();

    // Need at least two elements to form a pair
    if (n < 2) {
        return pairs;
    }

    // Reserve space to avoid reallocations if performance is critical
    // The number of pairs is n * (n - 1) / 2
    // pairs.reserve(n * (n - 1) / 2); // Optional optimization

    // Iterate through all possible first elements
    for (size_t i = 0; i < n; ++i) {
        // Iterate through all possible second elements *after* the first one
        // This ensures that j > i, preventing duplicates like (a, b) and (b, a)
        // and also prevents pairing an element with itself (i != j).
        for (size_t j = i + 1; j < n; ++j) {
            pairs.push_back({ids[i], ids[j]});
        }
    }

    return pairs;
}

// --- Example Usage ---
int main() {
    std::vector<size_t> myIds = {1, 2, 3, 4};//, 3, 4};

    std::vector<std::pair<size_t, size_t>> resultPairs = findUniquePairs(myIds);

    std::cout << "Input IDs: ";
    for (size_t id : myIds) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    std::cout << "Unique Pairs:" << std::endl;
    for (const auto& p : resultPairs) {
        std::cout << "{" << p.first << ", " << p.second << "}" << std::endl;
    }

    // Example with fewer than 2 elements
    std::vector<size_t> singleId = {5};
    std::vector<std::pair<size_t, size_t>> resultSingle = findUniquePairs(singleId);
    std::cout << "\nInput IDs: 5" << std::endl;
    std::cout << "Unique Pairs (should be none): " << resultSingle.size() << std::endl;


    // Example with empty vector
    std::vector<size_t> emptyIds = {};
    std::vector<std::pair<size_t, size_t>> resultEmpty = findUniquePairs(emptyIds);
    std::cout << "\nInput IDs: (empty)" << std::endl;
    std::cout << "Unique Pairs (should be none): " << resultEmpty.size() << std::endl;


    return 0;
}