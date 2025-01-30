#include <iostream>

void ImageFilterList::load(const std::string& filename) {
    std::cout << "Loading filter list from: " << filename << std::endl;
    
    // After parsing each filter
    std::cout << "Loaded filter: " << filterId << std::endl;
} 