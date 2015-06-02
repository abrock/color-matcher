#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <cassert>
#include <cmath>
#include <exception>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * Read RGB-values from strings containing hexadecimal values like "ff0000" and store them in integer values.
 * This doesn't throw any exceptions, if the string doesn't make sense the behaviour is undefined.
 *
 * @param[in] rgb String containing the hexadecimal representation of the color.
 * @param[out] r Here the value for red is stored.
 * @param[out] g Here the value for green is stored.
 * @param[out] b Here the value for blue is stored.
 */
void string2rgb(const std::string& rgb, int& r, int& g, int& b) {
    r = b = g = 0;
    try {
        r = std::stoi(rgb.substr(0,2), 0, 16); 
        g = std::stoi(rgb.substr(2,2), 0, 16); 
        b = std::stoi(rgb.substr(4,2), 0, 16); 
    }
    catch(std::exception& e){}// Who cares?

}

/**
 * Convert L*a*b-values to DIN99-values.
 *
 * @param[in] L L-value
 * @param[in] L a-value
 * @param[in] L b-value
 * @param[out] DIN_L calculated L-value in the DIN99 system.
 * @param[out] DIN_a calculated a-value in the DIN99 system.
 * @param[out] DIN_b calculated b-value in the DIN99 system.
 */
void lab2DIN(const float L, const float a, const float b, float& DIN_L, float& DIN_a, float& DIN_b) {

    #define COS16 0.96126169593831886192
    #define SIN16 0.27563735581699918561

    DIN_L = 105.51 * std::log(1 + 0.0158 * L);
    const float DIN_e = a * COS16 + b * SIN16;
    const float DIN_f = 0.7*(-a * SIN16 + b * COS16);
    
    const float DIN_G = std::sqrt(DIN_e * DIN_e + DIN_f * DIN_f);
    const float k = std::log(1 + 0.045 * DIN_G) / 0.045;

    if (DIN_G == 0) {
        DIN_a = 0;
        DIN_b = 0;
    }
    else {
        DIN_a = k * DIN_e / DIN_G;
        DIN_b = k * DIN_f / DIN_G;
    }

}

/**
 * Convert a string containing a hexadecimal RGB-representation of some color to a L*a*b-representation of the same color.
 *
 * @param[in] rgb String (hopefully) containing a hexadecimal RGB-representation of a color.
 * @param[out] dest_L Calculated L-value in the L*a*b-system
 * @param[out] dest_a Calculated a-value in the L*a*b-system
 * @param[out] dest_b Calculated b-value in the L*a*b-system
 */
void rgb2Lab(const std::string& rgb, float& dest_L, float& dest_a, float& dest_b) {
    using namespace cv;
    
    // Create a matrix from the image.
    Mat M(1, 1, CV_32FC3);

    // Get the R, G and B from the string
    int r = 0, g = 0, b = 0;
    string2rgb(rgb, r, g, b);

    // Write the normalized values into the matrix
    M.at<Vec3f>(0,0)[0] = (float)r/255;
    M.at<Vec3f>(0,0)[1] = (float)g/255;
    M.at<Vec3f>(0,0)[2] = (float)b/255;

    // Color space conversion
    cvtColor(M, M, COLOR_RGB2Lab);

    // Reading the values from the matrix.
    dest_L = M.at<Vec3f>(0,0)[0];
    dest_a = M.at<Vec3f>(0,0)[1];
    dest_b = M.at<Vec3f>(0,0)[2];

}

/**
 * Convert a string containing a hexadecimal RGB-representation of some color to a DIN99-representation of the same color.
 *
 * @param[in] rgb String (hopefully) containing a hexadecimal RGB-representation of a color.
 * @param[out] DIN_L calculated L-value in the DIN99 system.
 * @param[out] DIN_a calculated a-value in the DIN99 system.
 * @param[out] DIN_b calculated b-value in the DIN99 system.
 */
void rgb2DIN(const std::string& rgb, float& DIN_L, float& DIN_a, float& DIN_b) {
    float L = 0, a = 0, b = 0;
    rgb2Lab(rgb, L, a, b);
    lab2DIN(L, a, b, DIN_L, DIN_a, DIN_b);
}

/**
 * Calculate the square of a float.
 *
 * @param [in] a The value to square.
 * @return a*a
 */
float sqr(const float a) {
    return a*a;
}

/**
 * Calculate the difference of two colors in the L*a*b-system.
 *
 * @param[in] x String containing a hexadecimal RGB-representation of color #1.
 * @param[in] y String containing a hexadecimal RGB-representation of color #2.
 * @return Color difference
 */
float labDiff(const std::string& x, const std::string& y) {

    float x_L = 0, x_a = 0, x_b = 0, y_L = 0, y_a = 0, y_b = 0;

    rgb2Lab(x, x_L, x_a, x_b);
    rgb2Lab(y, y_L, y_a, y_b);

    return std::sqrt(sqr(x_L-y_L) + sqr(x_a-y_a) + sqr(x_b-y_b));
}

/**
 * Calculate the difference of two colors in the DIN99-system.
 *
 * @param[in] x String containing a hexadecimal RGB-representation of color #1.
 * @param[in] y String containing a hexadecimal RGB-representation of color #2.
 * @return Color difference
 */
float DINDiff(const std::string& x, const std::string& y) {

    float x_L = 0, x_a = 0, x_b = 0, y_L = 0, y_a = 0, y_b = 0;

    rgb2DIN(x, x_L, x_a, x_b);
    rgb2DIN(y, y_L, y_a, y_b);

    return std::sqrt(sqr(x_L-y_L) + sqr(x_a-y_a) + sqr(x_b-y_b));
}

/**
 * This runs a set of simple tests, converting hexadecimal representations to integer values. If somebody breaks the implementation assertions fail.
 */
void test1() {
    int r = 0, g = 0, b = 0;
    std::string test("ff00ff");
    string2rgb(test, r, g, b);
    
    assert(255 == r);
    assert(0 == g);
    assert(255 == b);

    test = std::string("5061BB");
    string2rgb(test, r, g, b);

    assert(80 == r);
    assert(97 == g);
    assert(187 == b);
}

/**
 * Convert a hexadecimal representation of a RGB color to the L*a*b-system and assert the error is smaller than 2e-3.
 *
 * @param[in] data Hexadecimal representation of the RGB color.
 * @param[in] L known L-value corresponding to the data.
 * @param[in] a known a-value corresponding to the data.
 * @param[in] b known b-value corresponding to the data.
 */
void testRGB2Lab(const char* data, const float L, const float a, const float b) {
    std::string test(data);

    float M_L = 999, M_a = 999, M_b = 999;

    rgb2Lab(test, M_L, M_a, M_b);

    const double error = (M_L - L)*(M_L - L) + (M_a - a)*(M_a - a) + (M_b - b)*(M_b - b);
    assert(error < 2e-3);
}

/**
 * Test the rgb2Lab function using the testRGB2Lab function.
 */
void test2() {
    testRGB2Lab("ffffff", 100, 0, 0);
    testRGB2Lab("777777", 50, 0, 0);
    testRGB2Lab("000000", 0, 0, 0);
    testRGB2Lab("ff0000", 53.2328817, 80.109309529822, 67.2200683102643);
    testRGB2Lab("00ff00", 87.7370334735442, -86.1846364976253, 83.1811647477785);
    testRGB2Lab("0000ff", 32.3025866672495, 79.1966617893094, -107.863681044952);
    testRGB2Lab("ffff00", 97.1382469812973, -21.5559083348323, 94.4824854464446);
}

/**
 * Test the labDiff method by feeding it two hexadecimal RGB representations and comparing the calculated difference to the known difference.
 *
 * @param[in] x Hexadecimal RBG representation of color #1
 * @param[in] y Hexadecimal RBG representation of color #2
 * @param[in] diff Known difference between x and y in the L*a*b-system
 */
void testLabDiff(const char* x, const char* y, const float diff) {
    std::string test_x(x);
    std::string test_y(y);

    const float diff2 = labDiff(test_x, test_y);
    const float diff3 = labDiff(test_y, test_x);

    assert(std::abs(diff2-diff) < 1e-4);
    assert(std::abs(diff3-diff) < 1e-4);
}

/**
 * Test the labDiff method by using the testLabDiff method
 */
void test3() {
   testLabDiff("000000", "000000", 0);
   testLabDiff("000f00", "000f00", 0);
   testLabDiff("f00f00", "f00f00", 0);
   testLabDiff("f01f00", "f01f00", 0);
   testLabDiff("f01f0b", "f01f0b", 0);
}

/**
 * Run all tests
 */
void test() {
    test1();
    test2();
    test3();
}

/**
 * Given a hexadecimal representation of a color and a map of colors, find (and print) the best match for the given color in the map wrt. the L*a*b-difference.
 *
 * @param[in] input Hexadecimal RGB-repesentation of the color we are searching a mathc for.
 * @param[in] map Map of colors we search in.
 */
void findBestLabMatch(const std::string& input, const std::map<std::string, std::string>& map) {
    std::string bestNum = map.begin()->first;
    float bestVal = labDiff(map.begin()->second, input);
    std::string bestRGB = map.begin()->second;

    for (auto it = map.begin(); it != map.end(); ++it) {
        const float currentVal = labDiff(it->second, input);
        if (bestVal > currentVal) {
            bestVal = currentVal;
            bestNum = it->first;
            bestRGB = it->second;
        }
    }

    std::cout << "best LAB match: " << bestNum << " (rgb: " << bestRGB << "), difference: " << bestVal << " ";
    if (bestVal < 0.2) {
        std::cout << "(not visible)" << std::endl;
    }
    else if (bestVal < 1) {
        std::cout << "(very small)" << std::endl;
    }
    else if (bestVal < 3) {
        std::cout << "(small)" << std::endl;
    }
    else if (bestVal < 6) {
        std::cout << "(medium)" << std::endl;
    }
    else {
        std::cout << "(large)" << std::endl;
    }
}

/**
 * Given a hexadecimal representation of a color and a map of colors, find (and print) the best match for the given color in the map wrt. the DIN99-difference.
 *
 * @param[in] input Hexadecimal RGB-repesentation of the color we are searching a mathc for.
 * @param[in] map Map of colors we search in.
 */
void findBestDINMatch(const std::string& input, const std::map<std::string, std::string>& map) {
    std::string bestNum = map.begin()->first;
    float bestVal = DINDiff(map.begin()->second, input);
    std::string bestRGB = map.begin()->second;

    for (auto it = map.begin(); it != map.end(); ++it) {
        const float currentVal = DINDiff(it->second, input);
        if (bestVal > currentVal) {
            bestVal = currentVal;
            bestNum = it->first;
            bestRGB = it->second;
        }
    }

    std::cout << "best DIN match: " << bestNum << " (rgb: " << bestRGB << "), difference: " << bestVal << " ";
    if (bestVal < 0.2) {
        std::cout << "(not visible)" << std::endl;
    }
    else if (bestVal < 1) {
        std::cout << "(very small)" << std::endl;
    }
    else if (bestVal < 3) {
        std::cout << "(small)" << std::endl;
    }
    else if (bestVal < 6) {
        std::cout << "(medium)" << std::endl;
    }
    else {
        std::cout << "(large)" << std::endl;
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " datafile" << std::endl;
        return 0;
    }
    
    // Read the color table
    std::ifstream table(argv[1]);

    std::map<std::string, std::string> colormap;

    while (table) {
        std::string line;
        std::getline(table, line);
        if (line.length() < 8) {
            continue;
        }
        std::stringstream stream(line);
        std::string color;
        stream >> color;
        std::string rgb;
        stream >> rgb;
        colormap[color] = rgb;
    }


    test();
    
    std::string input;

    std::cout << "Tests passed, read " << colormap.size() << " values from file " << argv[1] << std::endl;

    while (std::cin) {
        std::getline(std::cin, input);
        if (input.size() > 5) {
            int r = 0, g = 0, b = 0;
            string2rgb(input, r, g, b); 
            std::cout << "Input was interpreted as (" << r << ", " << g << ", " << b << ")" << std::endl;          
            findBestLabMatch(input, colormap);
            findBestDINMatch(input, colormap);
        }
        else {
            std::cout << "too short, try again" << std::endl;
        }
    }


    return 0;
}

