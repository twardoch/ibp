#include <iostream>

void ImageProcessor::applyFilters(/* parameters */) {
    for (const auto& filter : filters) {
        std::cout << "Applying filter: " << filter->getId() << std::endl;
        // Before/after applying each filter, you could log some image statistics
        // like average pixel values to verify changes
    }
} 