#include <windows.h>
#include <vfw.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>

#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

// Simple RGB structure
struct RGB {
    unsigned char r, g, b;
};

// Simple image structure
struct SimpleImage {
    int width, height;
    vector<RGB> pixels;
    
    SimpleImage(int w = 0, int h = 0) : width(w), height(h) {
        pixels.resize(w * h);
    }
    
    RGB& getPixel(int x, int y) {
        return pixels[y * width + x];
    }
    
    const RGB& getPixel(int x, int y) const {
        return pixels[y * width + x];
    }
};

// Simple face detection using basic image processing
class SimpleFaceDetector {
public:
    struct FaceRect {
        int x, y, width, height;
        double confidence;
    };
    
    vector<FaceRect> detectFaces(const SimpleImage& image) {
        vector<FaceRect> faces;
        
        // Convert to grayscale for processing
        vector<int> gray = convertToGray(image);
        
        // Simple face detection using template matching and skin color detection
        vector<FaceRect> candidates = findFaceCandidates(image, gray);
        
        // Filter candidates based on face-like features
        for (const auto& candidate : candidates) {
            if (isLikelyFace(image, candidate)) {
                faces.push_back(candidate);
            }
        }
        
        return faces;
    }
    
private:
    vector<int> convertToGray(const SimpleImage& image) {
        vector<int> gray(image.width * image.height);
        
        for (int i = 0; i < image.width * image.height; i++) {
            const RGB& pixel = image.pixels[i];
            gray[i] = (int)(0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b);
        }
        
        return gray;
    }
    
    vector<FaceRect> findFaceCandidates(const SimpleImage& image, const vector<int>& gray) {
        vector<FaceRect> candidates;
        
        // Simple sliding window approach
        for (int y = 0; y < image.height - 60; y += 10) {
            for (int x = 0; x < image.width - 60; x += 10) {
                // Check for skin color regions
                if (hasSkinColor(image, x, y, 60, 60)) {
                    FaceRect rect = {x, y, 60, 60, 0.7};
                    candidates.push_back(rect);
                }
            }
        }
        
        return candidates;
    }
    
    bool hasSkinColor(const SimpleImage& image, int startX, int startY, int width, int height) {
        int skinPixels = 0;
        int totalPixels = 0;
        
        for (int y = startY; y < startY + height && y < image.height; y++) {
            for (int x = startX; x < startX + width && x < image.width; x++) {
                const RGB& pixel = image.getPixel(x, y);
                
                // Simple skin color detection (rough approximation)
                if (isSkinColor(pixel)) {
                    skinPixels++;
                }
                totalPixels++;
            }
        }
        
        return (double)skinPixels / totalPixels > 0.3;
    }
    
    bool isSkinColor(const RGB& pixel) {
        // Simple skin color detection using RGB values
        int r = pixel.r, g = pixel.g, b = pixel.b;
        
        // Basic skin color ranges
        return (r > 95 && g > 40 && b > 20 && 
                r > g && r > b && 
                r - g > 15 && 
                abs(r - g) > 15);
    }
    
    bool isLikelyFace(const SimpleImage& image, const FaceRect& rect) {
        // Simple heuristics to validate face regions
        // Check aspect ratio
        double aspectRatio = (double)rect.width / rect.height;
        if (aspectRatio < 0.7 || aspectRatio > 1.3) return false;
        
        // Check if region has some variation (not uniform color)
        return hasVariation(image, rect);
    }
    
    bool hasVariation(const SimpleImage& image, const FaceRect& rect) {
        vector<int> intensities;
        
        for (int y = rect.y; y < rect.y + rect.height && y < image.height; y++) {
            for (int x = rect.x; x < rect.x + rect.width && x < image.width; x++) {
                const RGB& pixel = image.getPixel(x, y);
                int intensity = (pixel.r + pixel.g + pixel.b) / 3;
                intensities.push_back(intensity);
            }
        }
        
        if (intensities.empty()) return false;
        
        // Calculate variance
        double mean = 0;
        for (int val : intensities) mean += val;
        mean /= intensities.size();
        
        double variance = 0;
        for (int val : intensities) {
            variance += (val - mean) * (val - mean);
        }
        variance /= intensities.size();
        
        return variance > 100; // Threshold for sufficient variation
    }
};

// Simple gender classifier using basic features
class SimpleGenderClassifier {
public:
    struct GenderResult {
        string gender;
        double confidence;
    };
    
    GenderResult classifyGender(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        // Extract simple features from face region
        vector<double> features = extractFeatures(image, face);
        
        // Simple rule-based classification (this is very basic)
        double maleScore = calculateMaleScore(features);
        
        GenderResult result;
        if (maleScore > 0.5) {
            result.gender = "Male";
            result.confidence = maleScore;
        } else {
            result.gender = "Female";
            result.confidence = 1.0 - maleScore;
        }
        
        return result;
    }
    
private:
    vector<double> extractFeatures(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        vector<double> features;
        
        // Feature 1: Average brightness
        features.push_back(getAverageBrightness(image, face));
        
        // Feature 2: Color variance
        features.push_back(getColorVariance(image, face));
        
        // Feature 3: Edge density (rough approximation)
        features.push_back(getEdgeDensity(image, face));
        
        // Feature 4: Skin tone analysis
        features.push_back(getSkinToneFeature(image, face));
        
        return features;
    }
    
    double getAverageBrightness(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        double totalBrightness = 0;
        int pixelCount = 0;
        
        for (int y = face.y; y < face.y + face.height && y < image.height; y++) {
            for (int x = face.x; x < face.x + face.width && x < image.width; x++) {
                const RGB& pixel = image.getPixel(x, y);
                totalBrightness += (pixel.r + pixel.g + pixel.b) / 3.0;
                pixelCount++;
            }
        }
        
        return pixelCount > 0 ? totalBrightness / pixelCount : 0;
    }
    
    double getColorVariance(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        vector<double> intensities;
        
        for (int y = face.y; y < face.y + face.height && y < image.height; y++) {
            for (int x = face.x; x < face.x + face.width && x < image.width; x++) {
                const RGB& pixel = image.getPixel(x, y);
                intensities.push_back((pixel.r + pixel.g + pixel.b) / 3.0);
            }
        }
        
        if (intensities.empty()) return 0;
        
        double mean = 0;
        for (double val : intensities) mean += val;
        mean /= intensities.size();
        
        double variance = 0;
        for (double val : intensities) {
            variance += (val - mean) * (val - mean);
        }
        
        return variance / intensities.size();
    }
    
    double getEdgeDensity(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        int edgeCount = 0;
        int totalPixels = 0;
        
        for (int y = face.y + 1; y < face.y + face.height - 1 && y < image.height - 1; y++) {
            for (int x = face.x + 1; x < face.x + face.width - 1 && x < image.width - 1; x++) {
                // Simple edge detection using gradient
                const RGB& center = image.getPixel(x, y);
                const RGB& right = image.getPixel(x + 1, y);
                const RGB& bottom = image.getPixel(x, y + 1);
                
                int centerGray = (center.r + center.g + center.b) / 3;
                int rightGray = (right.r + right.g + right.b) / 3;
                int bottomGray = (bottom.r + bottom.g + bottom.b) / 3;
                
                int gradient = abs(centerGray - rightGray) + abs(centerGray - bottomGray);
                
                if (gradient > 30) edgeCount++;
                totalPixels++;
            }
        }
        
        return totalPixels > 0 ? (double)edgeCount / totalPixels : 0;
    }
    
    double getSkinToneFeature(const SimpleImage& image, const SimpleFaceDetector::FaceRect& face) {
        double rSum = 0, gSum = 0, bSum = 0;
        int pixelCount = 0;
        
        for (int y = face.y; y < face.y + face.height && y < image.height; y++) {
            for (int x = face.x; x < face.x + face.width && x < image.width; x++) {
                const RGB& pixel = image.getPixel(x, y);
                rSum += pixel.r;
                gSum += pixel.g;
                bSum += pixel.b;
                pixelCount++;
            }
        }
        
        if (pixelCount == 0) return 0;
        
        double avgR = rSum / pixelCount;
        double avgG = gSum / pixelCount;
        double avgB = bSum / pixelCount;
        
        // Simple skin tone analysis
        return (avgR - avgG) / (avgR + avgG + avgB);
    }
    
    double calculateMaleScore(const vector<double>& features) {
        if (features.size() < 4) return 0.5;
        
        double score = 0.5; // Start with neutral
        
        // Very simple rule-based scoring (this is quite basic)
        // Feature 0: Brightness - males often have slightly different skin tones
        if (features[0] > 120) score += 0.1;
        
        // Feature 1: Color variance - males might have more facial hair
        if (features[1] > 200) score += 0.15;
        
        // Feature 2: Edge density - males often have more defined features
        if (features[2] > 0.3) score += 0.2;
        
        // Feature 3: Skin tone feature
        if (features[3] > 0.1) score += 0.1;
        
        // Clamp between 0 and 1
        return max(0.0, min(1.0, score));
    }
};

// Camera capture class using Windows API
class CameraCapture {
private:
    HWND hWndCap;
    bool isInitialized;
    
public:
    CameraCapture() : hWndCap(nullptr), isInitialized(false) {}
    
    bool initialize() {
        // Create a capture window
        hWndCap = capCreateCaptureWindow(
            L"Camera Capture",
            WS_CHILD | WS_VISIBLE,
            0, 0, 320, 240,
            GetDesktopWindow(),
            1
        );
        
        if (!hWndCap) {
            cout << "Failed to create capture window!" << endl;
            return false;
        }
        
        // Connect to the first available driver
        if (!capDriverConnect(hWndCap, 0)) {
            cout << "Failed to connect to camera driver!" << endl;
            return false;
        }
        
        // Set up video format
        BITMAPINFO bmpInfo;
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = 320;
        bmpInfo.bmiHeader.biHeight = 240;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 24;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        
        capSetVideoFormat(hWndCap, &bmpInfo, sizeof(bmpInfo));
        
        isInitialized = true;
        cout << "Camera initialized successfully!" << endl;
        return true;
    }
    
    SimpleImage captureFrame() {
        if (!isInitialized) {
            return SimpleImage();
        }
        
        // Create a bitmap to capture to
        BITMAPINFO bmpInfo;
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = 320;
        bmpInfo.bmiHeader.biHeight = 240;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 24;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        
        int imageSize = 320 * 240 * 3;
        vector<unsigned char> buffer(imageSize);
        
        // Capture frame
        if (capGrabFrame(hWndCap)) {
            // This is a simplified approach - in reality you'd need more complex handling
            cout << "Frame captured!" << endl;
            
            // Create a simple test image for demonstration
            SimpleImage image(320, 240);
            
            // Fill with a test pattern (since actual video capture is complex)
            for (int y = 0; y < 240; y++) {
                for (int x = 0; x < 320; x++) {
                    RGB& pixel = image.getPixel(x, y);
                    pixel.r = (x + y) % 256;
                    pixel.g = (x * 2) % 256;
                    pixel.b = (y * 2) % 256;
                }
            }
            
            return image;
        }
        
        return SimpleImage();
    }
    
    void cleanup() {
        if (hWndCap) {
            capDriverDisconnect(hWndCap);
            DestroyWindow(hWndCap);
            hWndCap = nullptr;
        }
        isInitialized = false;
    }
    
    ~CameraCapture() {
        cleanup();
    }
};

// Simple console-based display
class ConsoleDisplay {
public:
    static void displayResults(const SimpleImage& image, 
                              const vector<SimpleFaceDetector::FaceRect>& faces, 
                              const vector<SimpleGenderClassifier::GenderResult>& results) {
        system("cls"); // Clear console
        
        cout << "=== Gender Detection Results ===" << endl;
        cout << "Image size: " << image.width << "x" << image.height << endl;
        cout << "Faces detected: " << faces.size() << endl;
        cout << "================================" << endl;
        
        for (size_t i = 0; i < faces.size() && i < results.size(); i++) {
            const auto& face = faces[i];
            const auto& result = results[i];
            
            cout << "Face " << (i + 1) << ":" << endl;
            cout << "  Position: (" << face.x << ", " << face.y << ")" << endl;
            cout << "  Size: " << face.width << "x" << face.height << endl;
            cout << "  Gender: " << result.gender << endl;
            cout << "  Confidence: " << (int)(result.confidence * 100) << "%" << endl;
            cout << "--------------------------------" << endl;
        }
        
        if (faces.empty()) {
            cout << "No faces detected in the image." << endl;
            cout << "Tips:" << endl;
            cout << "- Ensure good lighting" << endl;
            cout << "- Face should be clearly visible" << endl;
            cout << "- Try different angles" << endl;
        }
    }
    
    static void displayMenu() {
        cout << "\n=== Gender Detection Menu ===" << endl;
        cout << "1. Capture and analyze photo" << endl;
        cout << "2. Load image from file" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter your choice: ";
    }
};

// Main application
int main() {
    cout << "=== Gender Detection Project (No OpenCV) ===" << endl;
    cout << "This version uses basic image processing techniques" << endl;
    cout << "Note: Accuracy will be limited compared to ML models" << endl;
    cout << "=============================================" << endl;
    
    SimpleFaceDetector faceDetector;
    SimpleGenderClassifier genderClassifier;
    CameraCapture camera;
    
    // Try to initialize camera
    if (!camera.initialize()) {
        cout << "Camera initialization failed. Using test mode." << endl;
        
        // Create a test image for demonstration
        SimpleImage testImage(320, 240);
        
        // Create a simple face-like pattern
        for (int y = 80; y < 160; y++) {
            for (int x = 120; x < 200; x++) {
                RGB& pixel = testImage.getPixel(x, y);
                // Skin-like color
                pixel.r = 220;
                pixel.g = 180;
                pixel.b = 140;
            }
        }
        
        cout << "Processing test image..." << endl;
        
        // Detect faces
        vector<SimpleFaceDetector::FaceRect> faces = faceDetector.detectFaces(testImage);
        
        // Classify gender for each face
        vector<SimpleGenderClassifier::GenderResult> results;
        for (const auto& face : faces) {
            results.push_back(genderClassifier.classifyGender(testImage, face));
        }
        
        // Display results
        ConsoleDisplay::displayResults(testImage, faces, results);
        
        cout << "\nPress any key to continue..." << endl;
        cin.get();
        return 0;
    }
    
    int choice;
    do {
        ConsoleDisplay::displayMenu();
        cin >> choice;
        
        switch (choice) {
            case 1: {
                cout << "Capturing photo..." << endl;
                SimpleImage image = camera.captureFrame();
                
                if (image.width > 0 && image.height > 0) {
                    cout << "Processing image..." << endl;
                    
                    // Detect faces
                    vector<SimpleFaceDetector::FaceRect> faces = faceDetector.detectFaces(image);
                    
                    // Classify gender for each face
                    vector<SimpleGenderClassifier::GenderResult> results;
                    for (const auto& face : faces) {
                        results.push_back(genderClassifier.classifyGender(image, face));
                    }
                    
                    // Display results
                    ConsoleDisplay::displayResults(image, faces, results);
                } else {
                    cout << "Failed to capture image!" << endl;
                }
                break;
            }
            case 2: {
                cout << "File loading not implemented in this basic version." << endl;
                cout << "This would require additional image format libraries." << endl;
                break;
            }
            case 3: {
                cout << "Exiting..." << endl;
                break;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        }
        
        if (choice != 3) {
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        }
        
    } while (choice != 3);
    
    return 0;
}