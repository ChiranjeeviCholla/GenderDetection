#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <filesystem>

using namespace cv;
using namespace std;
using namespace dnn;

// Paths to the models
const string FACE_CASCADE_PATH = "assets/haarcascade_frontalface_default.xml";
const string GENDER_PROTO = "models/deploy_gender.prototxt";
const string GENDER_MODEL = "models/gender_net.caffemodel";

// Labels
const vector<string> GENDER_LIST = {"Male", "Female"};

// Load DNN Gender Classifier
Net loadGenderNet() {
    Net genderNet = readNetFromCaffe(GENDER_PROTO, GENDER_MODEL);
    if (genderNet.empty()) {
        cerr << "Failed to load gender model!" << endl;
        exit(1);
    }
    return genderNet;
}

// Classify gender
string classifyGender(Net& net, Mat& face)
 {
    Mat blob = blobFromImage(face, 1.0, Size(227, 227), Scalar(78.4263, 87.7689, 114.8958), false);
    net.setInput(blob);
    Mat prob = net.forward();
    int classId;
    double confidence;
    minMaxLoc(prob, 0, &confidence, 0, &classId);
    return GENDER_LIST[classId];
}

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Cannot open webcam!" << endl;
        return -1;
    }

    CascadeClassifier faceCascade;
    if (!faceCascade.load(FACE_CASCADE_PATH)) {
        cerr << "Failed to load Haar cascade!" << endl;
        return -1;
    }

    Net genderNet = loadGenderNet();
    int frameCount = 0;

    cout << "Press 's' to save image, 'q' to quit." << endl;

    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        vector<Rect> faces;
        faceCascade.detectMultiScale(frame, faces);

        for (auto& face : faces) {
            Mat faceROI = frame(face);
            resize(faceROI, faceROI, Size(227, 227));

            string gender = classifyGender(genderNet, faceROI);
            rectangle(frame, face, Scalar(0, 255, 0), 2);
            putText(frame, gender, Point(face.x, face.y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 0, 255), 2);
        }

        imshow("Gender Detection", frame);
        char key = (char)waitKey(1);
        if (key == 'q') break;

        if (key == 's') {
            string filename = "captured_" + to_string(frameCount++) + ".jpg";
            imwrite(filename, frame);
            cout << "Saved " << filename << endl;
        }
    }
#chiru the king of coding
    cap.release();
    destroyAllWindows();
    return 1;
}
